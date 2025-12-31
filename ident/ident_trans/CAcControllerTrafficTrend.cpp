#include "CAcControllerTrafficTrend.h"
#include "CIdentRelayApi.h"
#include "transxmlcfg.h"
#include <ctime>
#include <sstream>
#include <algorithm>
#include <map>

int CAcControllerTrafficTrend::IdentCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map inMap;
    // 获取请求参数
    pReqData->GetStrMap(inMap);
    
    // ========== 1. 参数验证 ==========
    
    // 获取timeRange参数（必填）
    string timeRange = inMap["timeRange"];
    if (timeRange.empty() || (timeRange != "1h" && timeRange != "6h" && timeRange != "24h"))
    {
        ErrorLog("timeRange参数无效: %s，有效值：1h/6h/24h", timeRange.c_str());
        throw(CTrsExp(ERR_UPFILE_COUNT, "timeRange参数无效，有效值：1h/6h/24h"));
        return 0;
    }
    
    // 从配置获取AC控制器信息
    string acIp = g_AcCfg.ip;
    
    InfoLog("请求参数: timeRange=%s, ac_ip=%s", 
           timeRange.c_str(), acIp.c_str());
    
    // ========== 2. 计算时间范围 ==========
    
    time_t now = time(NULL);
    time_t startTime = 0;
    int intervalSeconds = 0;  // 时间间隔（秒），用于聚合数据点
    
    if (timeRange == "1h")
    {
        startTime = now - 3600;  // 1小时前
        intervalSeconds = 60;    // 每分钟一个数据点（共60个点）
    }
    else if (timeRange == "6h")
    {
        startTime = now - 21600;  // 6小时前
        intervalSeconds = 300;    // 每5分钟一个数据点（共72个点）
    }
    else if (timeRange == "24h")
    {
        startTime = now - 86400;  // 24小时前（1天前）
        intervalSeconds = 3600;   // 每小时一个数据点（共24个点）
    }
    
    struct tm *tm_start = localtime(&startTime);
    struct tm *tm_end = localtime(&now);
    char startTimeStr[64], endTimeStr[64];
    snprintf(startTimeStr, sizeof(startTimeStr), "%04d-%02d-%02d %02d:%02d:%02d",
             tm_start->tm_year + 1900, tm_start->tm_mon + 1, tm_start->tm_mday,
             tm_start->tm_hour, tm_start->tm_min, tm_start->tm_sec);
    snprintf(endTimeStr, sizeof(endTimeStr), "%04d-%02d-%02d %02d:%02d:%02d",
             tm_end->tm_year + 1900, tm_end->tm_mon + 1, tm_end->tm_mday,
             tm_end->tm_hour, tm_end->tm_min, tm_end->tm_sec);
    
    InfoLog("时间范围: %s 到 %s，间隔: %d秒", startTimeStr, endTimeStr, intervalSeconds);
    
    // ========== 3. 根据timeRange判断数据来源 ==========
    // 只有7d需要查询历史数据，其他（1h/6h/24h）只从当天数据获取
    
    // 获取今天的日期字符串
    struct tm *tm_now = localtime(&now);
    char todayDateStr[32];
    snprintf(todayDateStr, sizeof(todayDateStr), "%04d-%02d-%02d",tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday);
    
    vector<CStr2Map> trafficData;
    
    // ========== 3.1 从Memcached获取当天的数据 ==========
    // 所有时间范围都需要当天数据
    
    InfoLog("从Memcached获取当天流量趋势数据: date=%s", todayDateStr);
        
        // 构建Memcached key（只查询AC级别数据）
        string trendKey = "ac_traffic_trend_" + acIp + "_ac_" + todayDateStr;
        
        string trendCache;
        if (GetCache(trendKey, trendCache) == 0 && !trendCache.empty())
        {
            InfoLog("从Memcached读取到当天流量趋势数据: key=%s, length=%zu", trendKey.c_str(), trendCache.length());
            
            // 解析Memcached数据（每行一个数据点，格式：time|upload|download）
            istringstream iss(trendCache);
            string line;
            while (getline(iss, line))
            {
                if (line.empty()) continue;
                
                // 解析：time|upload|download
                size_t pipe1 = line.find('|');
                size_t pipe2 = line.find('|', pipe1 + 1);
                if (pipe1 != string::npos && pipe2 != string::npos)
                {
                    string timeStr = line.substr(0, pipe1);
                    string uploadStr = line.substr(pipe1 + 1, pipe2 - pipe1 - 1);
                    string downloadStr = line.substr(pipe2 + 1);
                    
                    // 检查时间是否在查询范围内
                    // 将时间字符串转换为time_t进行比较
                    struct tm tm_check = {0};
                    if (sscanf(timeStr.c_str(), "%04d-%02d-%02d %02d:%02d:%02d",
                               &tm_check.tm_year, &tm_check.tm_mon, &tm_check.tm_mday,
                               &tm_check.tm_hour, &tm_check.tm_min, &tm_check.tm_sec) == 6)
                    {
                        tm_check.tm_year -= 1900;
                        tm_check.tm_mon -= 1;
                        time_t checkTime = mktime(&tm_check);
                        
                        if (checkTime >= startTime && checkTime <= now)
                        {
                            CStr2Map dataPoint;
                            dataPoint["time"] = timeStr;
                            dataPoint["upload"] = uploadStr;
                            dataPoint["download"] = downloadStr;
                            trafficData.push_back(dataPoint);
                        }
                    }
                }
            }
            
            InfoLog("从Memcached解析到 %zu 个当天数据点（在时间范围内）", trafficData.size());
        }
        else
        {
            InfoLog("Memcached中无当天流量趋势数据: key=%s", trendKey.c_str());
        }
    
    // ========== 3.3 按时间排序并聚合数据 ==========
    
    // 按时间排序（确保历史数据在前，当天数据在后）
    sort(trafficData.begin(), trafficData.end(), [](const CStr2Map& a, const CStr2Map& b) {
        auto it_a = a.find("time");
        auto it_b = b.find("time");
        string time_a = (it_a != a.end()) ? it_a->second : "";
        string time_b = (it_b != b.end()) ? it_b->second : "";
        return time_a < time_b;
    });
    
    // 如果需要聚合（intervalSeconds > 60），对数据进行聚合
    if (intervalSeconds > 60)
    {
        vector<CStr2Map> aggregatedData;
        map<string, vector<CStr2Map>> timeBucketMap;  // 按时间桶聚合
        
        for (size_t i = 0; i < trafficData.size(); ++i)
        {
            // 将时间转换为time_t，然后按intervalSeconds分组
            struct tm tm_point = {0};
            if (sscanf(trafficData[i]["time"].c_str(), "%04d-%02d-%02d %02d:%02d:%02d",
                       &tm_point.tm_year, &tm_point.tm_mon, &tm_point.tm_mday,
                       &tm_point.tm_hour, &tm_point.tm_min, &tm_point.tm_sec) == 6)
            {
                tm_point.tm_year -= 1900;
                tm_point.tm_mon -= 1;
                time_t pointTime = mktime(&tm_point);
                time_t bucketTime = (pointTime / intervalSeconds) * intervalSeconds;  // 向下取整到intervalSeconds
                
                struct tm *tm_bucket = localtime(&bucketTime);
                char bucketTimeStr[64];
                if (timeRange == "7d")
                {
                    // 7天显示日期格式：2024/1/1
                    snprintf(bucketTimeStr, sizeof(bucketTimeStr), "%04d/%d/%d",
                             tm_bucket->tm_year + 1900, tm_bucket->tm_mon + 1, tm_bucket->tm_mday);
                }
                else
                {
                    // 其他时间范围显示完整时间格式
                    snprintf(bucketTimeStr, sizeof(bucketTimeStr), "%04d-%02d-%02d %02d:%02d:%02d",
                             tm_bucket->tm_year + 1900, tm_bucket->tm_mon + 1, tm_bucket->tm_mday,
                             tm_bucket->tm_hour, tm_bucket->tm_min, tm_bucket->tm_sec);
                }
                
                timeBucketMap[bucketTimeStr].push_back(trafficData[i]);
            }
        }
        
        // 对每个时间桶进行聚合（取平均值）
        for (map<string, vector<CStr2Map>>::iterator it = timeBucketMap.begin(); it != timeBucketMap.end(); ++it)
        {
            if (it->second.empty()) continue;
            
            uint64_t totalUpload = 0;
            uint64_t totalDownload = 0;
            for (size_t i = 0; i < it->second.size(); ++i)
            {
                totalUpload += atoll(it->second[i]["upload"].c_str());
                totalDownload += atoll(it->second[i]["download"].c_str());
            }
            
            CStr2Map dataPoint;
            dataPoint["time"] = it->first;
            ostringstream uploadOss, downloadOss;
            uploadOss << (totalUpload / it->second.size());
            downloadOss << (totalDownload / it->second.size());
            dataPoint["upload"] = uploadOss.str();
            dataPoint["download"] = downloadOss.str();
            aggregatedData.push_back(dataPoint);
        }
        
        trafficData = aggregatedData;
        InfoLog("数据聚合完成，共 %zu 个数据点", trafficData.size());
    }
    
    InfoLog("处理完成，共 %zu 个时间序列数据点（历史数据+当天数据）", trafficData.size());
    
    // ========== 4. 设置返回参数 ==========
    
    pResData->SetPara("ret_num", std::to_string(trafficData.size()));  // 当前返回的条数（必填）
    pResData->SetPara("total", std::to_string(trafficData.size()));    // 总共有多少记录（必填）
    pResData->SetPara("timeRange", timeRange);                         // 时间范围（必填）
    
    // ========== 5. 设置数组数据 ==========
    
    for (size_t i = 0; i < trafficData.size(); ++i)
    {
        CStr2Map dataPoint;
        dataPoint["time"] = trafficData[i]["time"];           // 时间点（必填）
        dataPoint["upload"] = trafficData[i]["upload"];       // 上行流量（KB/s）（必填）
        dataPoint["download"] = trafficData[i]["download"];   // 下行流量（KB/s）（必填）
        
        // 添加到返回数组
        pResData->SetArray(dataPoint);
    }
    
    InfoLog("返回流量趋势数据: timeRange=%s, 数据点数=%zu", 
            timeRange.c_str(), trafficData.size());
    
    return 0;
}
