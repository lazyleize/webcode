#include "CAcControllerRealtime.h"
#include "CIdentRelayApi.h"
#include "CSnmpClient.h"
#include "transxmlcfg.h"
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>

int CAcControllerRealtime::IdentCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map inMap, outMap;
    // 获取请求参数
    pReqData->GetStrMap(inMap);
    
    // 从配置获取AC控制器信息
    string acIp = g_AcCfg.ip;
    string acCommunity = g_AcCfg.group_name;
    
    // ========== 优先从Memcached读取实时数据 ==========
    // 数据采集服务（CAcDataCollector）已将实时数据写入Memcached
    // Key: ac_stats_<ac_ip> - 统计数据
    // Key: ac_ap_list_<ac_ip> - AP列表数据
    // ====================================================
    string statsKey = "ac_stats_" + acIp;
    string apListKey = "ac_ap_list_" + acIp;
    string statsCache;
    string apListCache;
    bool fromCache = false;
    
    int totalConnections = 0;      // 当前总连接数
    int currentUpload = 0;         // 当前总上行速率（KB/s）
    int currentDownload = 0;       // 当前总下行速率（KB/s）
    
    // 尝试从Memcached读取统计数据
    if (GetCache(statsKey, statsCache) == 0 && !statsCache.empty())
    {
        InfoLog("从Memcached读取统计数据成功: key=%s", statsKey.c_str());
        
        // 解析缓存数据（格式：key=value&key=value）
        CStr2Map cacheMap;
        string::size_type pos = 0;
        while (pos < statsCache.length())
        {
            string::size_type eqPos = statsCache.find('=', pos);
            if (eqPos == string::npos) break;
            
            string key = statsCache.substr(pos, eqPos - pos);
            pos = eqPos + 1;
            
            string::size_type ampPos = statsCache.find('&', pos);
            string value;
            if (ampPos == string::npos)
            {
                value = statsCache.substr(pos);
                pos = statsCache.length();
            }
            else
            {
                value = statsCache.substr(pos, ampPos - pos);
                pos = ampPos + 1;
            }
            
            cacheMap[key] = value;
        }
        
        // 从缓存中提取总用户数（作为总连接数）
        totalConnections = atoi(cacheMap["totalUsers"].c_str());
        InfoLog("从Memcached获取到总连接数: %d", totalConnections);
    }
    
    // 尝试从Memcached读取AP列表数据（用于计算总连接数和实时速率）
    if (GetCache(apListKey, apListCache) == 0 && !apListCache.empty())
    {
        InfoLog("从Memcached读取AP列表数据成功: key=%s", apListKey.c_str());
        fromCache = true;
        
        // 解析AP列表数据（格式：AP1数据|AP2数据|AP3数据）
        string::size_type pos = 0;
        int apCount = 0;
        int totalConnectionsFromAP = 0;
        int totalUploadSpeed = 0;  // 所有AP的上行速率总和（Mbps）
        int totalDownloadSpeed = 0; // 所有AP的下行速率总和（Mbps）
        
        while (pos < apListCache.length())
        {
            string::size_type pipePos = apListCache.find('|', pos);
            string apData;
            if (pipePos == string::npos)
            {
                apData = apListCache.substr(pos);
                pos = apListCache.length();
            }
            else
            {
                apData = apListCache.substr(pos, pipePos - pos);
                pos = pipePos + 1;
            }
            
            if (apData.empty()) continue;
            
            // 解析单个AP数据（格式：routerId,name,ip,status,cpu,memory,onlineUsers,uploadSpeed,downloadSpeed）
            vector<string> fields;
            string::size_type fieldPos = 0;
            while (fieldPos < apData.length())
            {
                string::size_type commaPos = apData.find(',', fieldPos);
                string field;
                if (commaPos == string::npos)
                {
                    field = apData.substr(fieldPos);
                    fieldPos = apData.length();
                }
                else
                {
                    field = apData.substr(fieldPos, commaPos - fieldPos);
                    fieldPos = commaPos + 1;
                }
                fields.push_back(field);
            }
            
            if (fields.size() >= 9)
            {
                apCount++;
                int onlineUsers = atoi(fields[6].c_str());      // onlineUsers
                int uploadSpeedMbps = atoi(fields[7].c_str());   // uploadSpeed (Mbps)
                int downloadSpeedMbps = atoi(fields[8].c_str()); // downloadSpeed (Mbps)
                
                totalConnectionsFromAP += onlineUsers;
                totalUploadSpeed += uploadSpeedMbps;
                totalDownloadSpeed += downloadSpeedMbps;
            }
        }
        
        // 如果从AP列表获取到连接数，使用该值；否则使用统计数据的值
        if (totalConnectionsFromAP > 0)
        {
            totalConnections = totalConnectionsFromAP;
        }
        
        // 将Mbps转换为KB/s（1 Mbps = 125 KB/s）
        currentUpload = totalUploadSpeed * 125;
        currentDownload = totalDownloadSpeed * 125;
        
        InfoLog("从Memcached解析到 %d 个AP，总连接数=%d，总上行速率=%d KB/s，总下行速率=%d KB/s",apCount, totalConnections, currentUpload, currentDownload);
    }
    else
    {
        // Memcached中无AP列表数据，直接报错，不使用SNMP降级
        ErrorLog("从Memcached读取AP列表数据失败: key=%s，数据采集服务可能未运行或数据未写入", apListKey.c_str());
        throw(CTrsExp(ERR_UPFILE_COUNT, "从Memcached读取AP列表数据失败，请检查数据采集服务是否正常运行"));
        return 0;
    }
    
    int peakTraffic = 0;
    string peakTime = "";
    
    // 获取今日日期字符串
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char dateStr[32];
    snprintf(dateStr, sizeof(dateStr), "%04d-%02d-%02d",tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday);
    
    // 优先从Memcached读取今日峰值
    // Key格式：ac_peak_traffic_<ac_ip>_<date>
    // Value格式：peak_traffic|peak_time
    // 数据库操作：从Memcached读取，如果不存在则从数据库查询
    string peakKey = "ac_peak_traffic_" + acIp + "_" + dateStr;
    string peakCache;
    bool peakFromCache = false;  // 使用不同的变量名，避免与上面的fromCache冲突
    
    if (GetCache(peakKey, peakCache) == 0 && !peakCache.empty())
    {
        // 解析Memcached中的峰值数据（格式：peak_traffic|peak_time）
        size_t pipePos = peakCache.find('|');
        if (pipePos != string::npos)
        {
            peakTraffic = atoi(peakCache.substr(0, pipePos).c_str());
            peakTime = peakCache.substr(pipePos + 1);
            peakFromCache = true;
            InfoLog("从Memcached读取到峰值流量: %d KB/s, 峰值时间: %s", peakTraffic, peakTime.c_str());
        }
    }
    
    // 如果Memcached中没有，从数据库查询
    if (!peakFromCache)
    {
        InfoLog("Memcached中没有峰值数据，从数据库查询");
        
        // 数据库操作：SELECT Fpeak_traffic, Fpeak_time 
        //            FROM t_ac_controller_peak_traffic 
        //            WHERE Fac_ip = ? AND Fdate = ?
        // 注意：需要relay服务端实现对应的SQL逻辑（reqid=1860或新的reqid）
        CStr2Map queryInMap, queryOutMap;
        queryInMap["ac_ip"] = acIp;
        queryInMap["date"] = dateStr;  // 今日日期，格式：YYYY-MM-DD
        
        // 调用relay服务查询峰值数据
        // 注意：需要修改QueryAcControllerPeakTraffic函数，支持从t_ac_controller_peak_traffic表查询
        bool ret = CIdentRelayApi::QueryAcControllerPeakTraffic(queryInMap, queryOutMap, false);  // 不抛异常
        
        if (ret && queryOutMap.find("peak_traffic") != queryOutMap.end())
        {
            peakTraffic = atoi(queryOutMap["peak_traffic"].c_str());  // KB/s
            peakTime = queryOutMap["peak_time"];  // 格式：YYYY-MM-DD HH:MM:SS
            InfoLog("从数据库查询到峰值流量: %d KB/s, 峰值时间: %s", peakTraffic, peakTime.c_str());
            
            // 查询后写入Memcached（下次走缓存）
            // 数据库操作：写入Memcached，key=ac_peak_traffic_<ac_ip>_<date>，value=peak_traffic|peak_time，过期时间=86400秒
            ostringstream peakOss;
            peakOss << peakTraffic << "|" << peakTime;
            string peakValue = peakOss.str();
            if (SetOrUpdateCache(peakKey, peakValue, 86400) == 0)  // 缓存24小时
            {
                InfoLog("峰值流量已写入Memcached: key=%s, value=%s", peakKey.c_str(), peakValue.c_str());
            }
        }
        else
        {
            ErrorLog("从数据库查询峰值流量失败: ac_ip=%s, date=%s", acIp.c_str(), dateStr);
            // 不抛异常，返回默认值0
            peakTraffic = 0;
            peakTime = "";
        }
    }
    
    // ========== 设置返回参数 ==========
    
    // ret_num 和 total：对于实时监控接口，通常返回1条记录
    pResData->SetPara("ret_num", "1");      // 当前返回的条数（必填）
    pResData->SetPara("total", "1");        // 总共有多少记录（必填）
    pResData->SetPara("totalConnections", std::to_string(totalConnections));  // 当前总连接数（必填）
    pResData->SetPara("currentUpload", std::to_string(currentUpload));        // 当前总上行速率（KB/s）（必填）
    pResData->SetPara("currentDownload", std::to_string(currentDownload));    // 当前总下行速率（KB/s）（必填）
    
    if (peakTraffic > 0 && !peakTime.empty())
    {
        pResData->SetPara("peakTraffic", std::to_string(peakTraffic));  // 峰值流量（KB/s）
        pResData->SetPara("peakTime", peakTime);                        // 峰值时间
    }
    else
    {
        // 如果没有峰值数据，返回空或0
        pResData->SetPara("peakTraffic", "0");
        pResData->SetPara("peakTime", "");
    }
    
    InfoLog("返回实时监控数据: 总连接数=%d, 上行速率=%d KB/s, 下行速率=%d KB/s, 峰值流量=%d KB/s, 峰值时间=%s",totalConnections, currentUpload, currentDownload, peakTraffic, peakTime.c_str());
    
    return 0;
}
