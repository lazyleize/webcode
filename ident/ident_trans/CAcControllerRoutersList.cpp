#include "CAcControllerRoutersList.h"
#include "CSnmpClient.h"
#include "transxmlcfg.h"
#include "tools/CTools.h"
#include "CIdentComm.h"
#include "CIdentRelayApi.h"
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>

// MAC地址转换函数：将十六进制MAC地址转换为OID索引格式
// 输入：Hex-STRING格式，如 "60 53 75 2D B8 80" 或 "60:53:75:2D:B8:80"
// 输出：点分十进制格式，如 "96.83.117.45.184.128"
string CAcControllerRoutersList::ConvertMacToOidIndex(const string& hexMac)
{
    if (hexMac.empty())
        return "";
    
    string mac = hexMac;
    // 移除空格和冒号
    size_t pos = 0;
    while ((pos = mac.find(" ", pos)) != string::npos) {
        mac.erase(pos, 1);
    }
    pos = 0;
    while ((pos = mac.find(":", pos)) != string::npos) {
        mac.erase(pos, 1);
    }
    
    // 如果已经是纯十六进制字符串（如 "6053752DB880"），直接转换
    if (mac.length() == 12)
    {
        ostringstream oss;
        for (size_t i = 0; i < 12; i += 2)
        {
            string byteStr = mac.substr(i, 2);
            int byteValue = 0;
            sscanf(byteStr.c_str(), "%x", &byteValue);
            if (i > 0) oss << ".";
            oss << byteValue;
        }
        return oss.str();
    }
    
    // 如果是空格分隔的格式（如 "60 53 75 2D B8 80"），需要解析
    istringstream iss(hexMac);
    ostringstream oss;
    string byteStr;
    bool first = true;
    while (iss >> byteStr)
    {
        int byteValue = 0;
        if (byteStr.length() == 2)
        {
            sscanf(byteStr.c_str(), "%x", &byteValue);
        }
        else if (byteStr.length() == 1)
        {
            sscanf(byteStr.c_str(), "%x", &byteValue);
        }
        if (!first) oss << ".";
        oss << byteValue;
        first = false;
    }
    return oss.str();
}

// 从AC控制器获取路由器列表
vector<RouterDetailInfo> CAcControllerRoutersList::GetRouterListFromAC(CSnmpClient& acClient)
{
    vector<RouterDetailInfo> routers;
    
    // 优先使用 hwWlanIDIndexedApTable（使用简单AP ID作为索引：0, 1, 2, 3...）
    // 表OID: 1.3.6.1.4.1.2011.6.139.13.3.10.1
    // 1.3.6.1.4.1.2011.6.139.13.3.10.1.1 - AP ID（索引本身）
    // 1.3.6.1.4.1.2011.6.139.13.3.10.1.2 - AP MAC地址
    // 1.3.6.1.4.1.2011.6.139.13.3.10.1.5 - AP名称
    // 1.3.6.1.4.1.2011.6.139.13.3.10.1.14 - AP IP地址
    // 1.3.6.1.4.1.2011.6.139.13.3.10.1.7 - AP状态 (normal(8)=在线)
    
    string apNameOid = "1.3.6.1.4.1.2011.6.139.13.3.10.1.5";  // hwWlanIDIndexedApName
    map<string, string> apNameMap;
    if (acClient.Walk(apNameOid, apNameMap) == 0 && !apNameMap.empty())
    {
        InfoLog("通过Walk获取到 %zu 个AP名称", apNameMap.size());
        
        for (map<string, string>::iterator it = apNameMap.begin(); it != apNameMap.end(); ++it)
        {
            string oid = it->first;
            string apName = it->second;
            
            // 解析OID获取AP ID（最后一个数字）
            // OID格式：1.3.6.1.4.1.2011.6.139.13.3.10.1.5.{apId}
            size_t lastDot = oid.find_last_of(".");
            if (lastDot == string::npos) continue;
            
            string apId = oid.substr(lastDot + 1);
            if (apId.empty() || apName.empty()) continue;
            
            RouterDetailInfo router;
            router.routerId = apId;
            router.name = apName;
            
            // 获取AP IP地址
            string ipOid = "1.3.6.1.4.1.2011.6.139.13.3.10.1.14." + apId;  // hwWlanIDIndexedApIpAddress
            string ipValue;
            if (acClient.Get(ipOid, ipValue) == 0 && ipValue != "0.0.0.0" && ipValue != "255.255.255.255")
            {
                router.ip = ipValue;
            }
            
            // 获取AP状态
            string stateOid = "1.3.6.1.4.1.2011.6.139.13.3.10.1.7." + apId;  // hwWlanIDIndexedApRunState
            int64_t stateValue = 0;
            if (acClient.GetInt(stateOid, stateValue) == 0)
            {
                if (stateValue == 8)  // normal(8)=在线
                {
                    router.status = "online";
                }
                else if (stateValue == 4)  // fault(4)=故障
                {
                    router.status = "fault";
                }
                else
                {
                    router.status = "offline";
                }
            }
            else
            {
                router.status = "offline";
            }
            
            routers.push_back(router);
            InfoLog("发现AP: ID=%s, Name=%s, IP=%s, Status=%s",apId.c_str(), apName.c_str(), router.ip.c_str(), router.status.c_str());
        }
    }
    else
    {
        InfoLog("无法通过Walk获取AP列表，使用默认4个路由器");
        // 降级方案：使用默认的4个路由器
        for (int i = 0; i < 4; ++i)
        {
            RouterDetailInfo router;
            router.routerId = std::to_string(i);
            router.name = "AP-" + std::to_string(i + 1);
            router.status = "offline";
            routers.push_back(router);
        }
    }
    
    return routers;
}

// 从AC控制器获取指定路由器的详细信息
int CAcControllerRoutersList::GetRouterDetailInfoFromAC(CSnmpClient& acClient, RouterDetailInfo& router)
{
    InfoLog("GetRouterDetailInfoFromAC: 开始处理AP ID=%s, Name=%s", router.routerId.c_str(), router.name.c_str());
    
    // ========== 1. 获取基本信息 ==========
    
    // 获取AP IP地址（如果还没有）
    if (router.ip.empty())
    {
        string ipOid = "1.3.6.1.4.1.2011.6.139.13.3.10.1.14." + router.routerId;  // hwWlanIDIndexedApIpAddress
        string ipValue;
        if (acClient.Get(ipOid, ipValue) == 0 && ipValue != "0.0.0.0" && ipValue != "255.255.255.255")
        {
            router.ip = ipValue;
            InfoLog("从AC获取到AP[%s]的IP地址: %s", router.name.c_str(), router.ip.c_str());
        }
    }
    
    // 获取AP状态（如果还没有）
    if (router.status.empty())
    {
        string stateOid = "1.3.6.1.4.1.2011.6.139.13.3.10.1.7." + router.routerId;  // hwWlanIDIndexedApRunState
        int64_t stateValue = 0;
        if (acClient.GetInt(stateOid, stateValue) == 0)
        {
            if (stateValue == 8)  // normal(8)=在线
            {
                router.status = "online";
            }
            else if (stateValue == 4)  // fault(4)=故障
            {
                router.status = "fault";
            }
            else
            {
                router.status = "offline";
            }
        }
    }
    
    // 获取AP MAC地址（用于后续OID查询）
    string macHex;
    string macOidIndex;
    string macOid = "1.3.6.1.4.1.2011.6.139.13.3.10.1.2." + router.routerId;  // hwWlanIDIndexedApMac
    if (acClient.Get(macOid, macHex) == 0 && !macHex.empty())
    {
        macOidIndex = ConvertMacToOidIndex(macHex);
        InfoLog("从AC获取到AP[%s]的MAC地址: %s (OID格式: %s)",router.name.c_str(), macHex.c_str(), macOidIndex.c_str());
    }
    
    // ========== 2. 获取性能数据 ==========
    
    // 获取CPU使用率
    string cpuOid = "1.3.6.1.4.1.2011.6.139.13.3.10.1.42." + router.routerId;  // hwWlanIDIndexedApCpuUseRate
    int64_t cpuValue = 0;
    if (acClient.GetInt(cpuOid, cpuValue) == 0 && cpuValue >= 0 && cpuValue <= 100)
    {
        router.cpu = (int)cpuValue;
        InfoLog("从AC获取到AP[%s]的CPU使用率: %d%%", router.name.c_str(), router.cpu);
    }
    
    // 获取内存使用率
    string memoryOid = "1.3.6.1.4.1.2011.6.139.13.3.10.1.41." + router.routerId;  // hwWlanIDIndexedApMemoryUseRate
    int64_t memoryValue = 0;
    if (acClient.GetInt(memoryOid, memoryValue) == 0 && memoryValue >= 0 && memoryValue <= 100)
    {
        router.memory = (int)memoryValue;
        InfoLog("从AC获取到AP[%s]的内存使用率: %d%%", router.name.c_str(), router.memory);
    }
    
    // 获取在线用户数（connections）
    string usersOid = "1.3.6.1.4.1.2011.6.139.13.3.10.1.45." + router.routerId;  // hwWlanIDIndexedApOnlineUserNum
    int64_t usersValue = 0;
    if (acClient.GetInt(usersOid, usersValue) == 0)
    {
        router.connections = (int)usersValue;
        InfoLog("从AC获取到AP[%s]的在线用户数: %d", router.name.c_str(), router.connections);
    }
    
    // 注意：温度和运行时间应该已经从Memcached中获取（CAcDataCollector已存储）
    // 如果Memcached中没有，需要从数据库获取，失败直接报错
    // TODO: 从数据库查询温度和运行时间（如果Memcached中没有）
    // 注意：GetRouterDetailInfoFromAC函数中没有acIp变量，这些数据应该从Memcached获取
    // 如果需要从数据库获取，应该在调用方（IdentCommit）中处理
    
    // ========== 3. 获取实时速率（当前速率） ==========
    
    // 获取上行实时速率（Mbps -> KB/s）
    if (!macOidIndex.empty())
    {
        string uploadSpeedOid = "1.3.6.1.4.1.2011.6.139.13.3.3.1.54." + macOidIndex;  // hwWlanApUpPortSpeed (Mbps)
        int64_t uploadSpeedMbps = 0;
        if (acClient.GetInt(uploadSpeedOid, uploadSpeedMbps) == 0)
        {
            // Mbps转KB/s: 1 Mbps = 125 KB/s
            router.uploadSpeedCurrent = (int)(uploadSpeedMbps * 125);
            InfoLog("从AC获取到AP[%s]的上行实时速率: %d Mbps (%d KB/s)", 
                   router.name.c_str(), (int)uploadSpeedMbps, router.uploadSpeedCurrent);
        }
    }
    
    // 获取下行实时速率（Mbps -> KB/s）
    if (!macOidIndex.empty())
    {
        string downloadSpeedOid = "1.3.6.1.4.1.2011.6.139.13.3.3.1.57." + macOidIndex;  // hwWlanEthportDownRate (Mbps)
        int64_t downloadSpeedMbps = 0;
        if (acClient.GetInt(downloadSpeedOid, downloadSpeedMbps) == 0)
        {
            // Mbps转KB/s: 1 Mbps = 125 KB/s
            router.downloadSpeedCurrent = (int)(downloadSpeedMbps * 125);
            InfoLog("从AC获取到AP[%s]的下行实时速率: %d Mbps (%d KB/s)",router.name.c_str(), (int)downloadSpeedMbps, router.downloadSpeedCurrent);
        }
        else
        {
            ErrorLog("从AC获取AP[%s]的下行实时速率失败: OID=%s, error=%s",router.name.c_str(), downloadSpeedOid.c_str(), acClient.GetLastError().c_str());
        }
    }
    
    // ========== 4. 获取累计流量（今日流量需要从数据库获取） ==========
    
    // 获取无线上行累计流量（Bytes -> KB）
    if (!macOidIndex.empty())
    {
        string uploadTrafficOid = "1.3.6.1.4.1.2011.6.139.13.3.3.1.58." + macOidIndex;  // hwWlanIDIndexedApAirportUpTraffic
        string uploadTrafficValue;
        if (acClient.Get(uploadTrafficOid, uploadTrafficValue) == 0 && !uploadTrafficValue.empty())
        {
            // 移除引号
            if (uploadTrafficValue[0] == '"' && uploadTrafficValue[uploadTrafficValue.length()-1] == '"')
            {
                uploadTrafficValue = uploadTrafficValue.substr(1, uploadTrafficValue.length()-2);
            }
            uint64_t uploadBytes = 0;
            sscanf(uploadTrafficValue.c_str(), "%lu", (unsigned long*)&uploadBytes);
            // 注意：这是累计流量，不是今日流量
            // 从数据库获取今日上行流量
            // 注意：GetRouterDetailInfoFromAC函数中没有acIp和dateStr变量
            // 这些数据应该在调用方（IdentCommit）中从数据库获取，这里只从SNMP获取累计流量
            // TODO: 在调用方（IdentCommit）中从数据库获取今日流量
            InfoLog("从AC获取到AP[%s]的上行累计流量: %llu KB (%s Bytes)",router.name.c_str(), uploadBytes / 1024, uploadTrafficValue.c_str());
        }
    }
    
    // 获取无线下行累计流量（Bytes -> KB）
    if (!macOidIndex.empty())
    {
        string downloadTrafficOid = "1.3.6.1.4.1.2011.6.139.13.3.3.1.59." + macOidIndex;  // hwWlanIDIndexedApAirportDwTraffic
        string downloadTrafficValue;
        if (acClient.Get(downloadTrafficOid, downloadTrafficValue) == 0 && !downloadTrafficValue.empty())
        {
            // 移除引号
            if (downloadTrafficValue[0] == '"' && downloadTrafficValue[downloadTrafficValue.length()-1] == '"')
            {
                downloadTrafficValue = downloadTrafficValue.substr(1, downloadTrafficValue.length()-2);
            }
            uint64_t downloadBytes = 0;
            sscanf(downloadTrafficValue.c_str(), "%lu", (unsigned long*)&downloadBytes);
            // 注意：这是累计流量，不是今日流量
            // 注意：GetRouterDetailInfoFromAC函数中没有acIp和dateStr变量
            // 这些数据应该在调用方（IdentCommit）中从数据库获取，这里只从SNMP获取累计流量
            // TODO: 在调用方（IdentCommit）中从数据库获取今日流量
            InfoLog("从AC获取到AP[%s]的下行累计流量: %llu KB (%s Bytes)",router.name.c_str(), downloadBytes / 1024, downloadTrafficValue.c_str());
        }
    }
    
    // 设置最后更新时间
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char timeStr[64];
    snprintf(timeStr, sizeof(timeStr), "%04d-%02d-%02d %02d:%02d:%02d",
             tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday,
             tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
    router.lastUpdate = timeStr;
    
    return 0;
}

int CAcControllerRoutersList::IdentCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map inMap, outMap;
    // 获取请求参数
    pReqData->GetStrMap(inMap);
    
    // ========== 1. 参数验证 ==========
    
    // 获取offset参数（必填，最低1）
    int offset = atoi(inMap["offset"].c_str());
    if (offset < 1)
    {
        ErrorLog("offset参数无效: %d，最低为1", offset);
        throw(CTrsExp(ERR_UPFILE_COUNT, "offset参数无效，最低为1"));
        return 0;
    }
    
    // 获取limit参数（必填，最大30）
    int limit = atoi(inMap["limit"].c_str());
    if (limit < 1 || limit > 30)
    {
        ErrorLog("limit参数无效: %d，范围1-30", limit);
        throw(CTrsExp(ERR_UPFILE_COUNT, "limit参数无效，范围1-30"));
        return 0;
    }
    
    InfoLog("请求参数: offset=%d, limit=%d", offset, limit);
    
    // 获取当前日期（用于数据库查询）
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char dateStr[32];
    snprintf(dateStr, sizeof(dateStr), "%04d-%02d-%02d",tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday);
    
    // 从配置获取AC控制器信息
    if(inMap["ac_ip"].empty())
    {
        inMap["ac_ip"] = "192.168.7.252";
    }
    string acIp = inMap["ac_ip"];
    string acCommunity = g_AcCfg.group_name;
    
    // ========== 2. 优先从Memcached读取AP列表数据 ==========
    // 数据采集服务（CAcDataCollector）已将实时数据写入Memcached
    // Key: ac_ap_list_<ac_ip>
    // 格式: AP1数据|AP2数据|AP3数据（用|分隔，每个AP用,分隔字段）
    // 字段顺序：routerId,name,ip,status,cpu,memory,onlineUsers,uploadSpeed,downloadSpeed
    // ====================================================
    string apListKey = "ac_ap_list_" + acIp;
    string apListCache;
    vector<RouterDetailInfo> allRouters;
    bool fromCache = false;
    
    if (GetCache(apListKey, apListCache) == 0 && !apListCache.empty())
    {
        InfoLog("从Memcached读取AP列表数据成功: key=%s", apListKey.c_str());
        fromCache = true;
        
        // 解析AP列表数据（格式：AP1数据|AP2数据|AP3数据）
        string::size_type pos = 0;
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
            
            if (fields.size() >= 11)  // 更新字段数量：增加了uptime和temperature
            {
                RouterDetailInfo router;
                router.routerId = fields[0];      // routerId
                router.name = fields[1];         // name
                router.ip = fields[2];           // ip
                router.status = fields[3];       // status
                router.cpu = atoi(fields[4].c_str());           // cpu
                router.memory = atoi(fields[5].c_str());         // memory
                router.connections = atoi(fields[6].c_str());    // onlineUsers
                router.uploadSpeedCurrent = atoi(fields[7].c_str());  // uploadSpeed (Mbps -> KB/s)
                router.uploadSpeedCurrent = router.uploadSpeedCurrent * 125;  // 转换为KB/s
                router.downloadSpeedCurrent = atoi(fields[8].c_str());  // downloadSpeed (预留，可能为0)
                router.downloadSpeedCurrent = router.downloadSpeedCurrent * 125;  // 转换为KB/s
                router.uptime = atoi(fields[9].c_str());  // uptime (秒)
                router.temperature = atoi(fields[10].c_str());  // temperature (℃)
                
                // 设置最后更新时间
                time_t now = time(NULL);
                struct tm *tm_now = localtime(&now);
                char timeStr[64];
                snprintf(timeStr, sizeof(timeStr), "%04d-%02d-%02d %02d:%02d:%02d",
                         tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday,
                         tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
                router.lastUpdate = timeStr;
                
                // 注意：今日流量统一在后面的循环中查询，避免重复查询
                // 先设置默认值，后续循环中会更新
                router.uploadToday = 0;
                router.downloadToday = 0;
                
                allRouters.push_back(router);
            }
            else if (fields.size() >= 9)
            {
                // 兼容旧格式（9个字段，没有uptime和temperature）
                RouterDetailInfo router;
                router.routerId = fields[0];
                router.name = fields[1];
                router.ip = fields[2];
                router.status = fields[3];
                router.cpu = atoi(fields[4].c_str());
                router.memory = atoi(fields[5].c_str());
                router.connections = atoi(fields[6].c_str());
                router.uploadSpeedCurrent = atoi(fields[7].c_str()) * 125;
                router.downloadSpeedCurrent = atoi(fields[8].c_str()) * 125;
                router.uptime = 0;  // 需要从数据库获取
                router.temperature = 0;  // 需要从数据库获取
                
                time_t now = time(NULL);
                struct tm *tm_now = localtime(&now);
                char timeStr[64];
                snprintf(timeStr, sizeof(timeStr), "%04d-%02d-%02d %02d:%02d:%02d",
                         tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday,
                         tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
                router.lastUpdate = timeStr;
                
                allRouters.push_back(router);
            }
        }
        
        InfoLog("从Memcached解析到 %zu 个AP", allRouters.size());
        
        // ========== 从数据库获取缺失数据（今日流量、运行时间、温度） ==========
        // 这些数据不在Memcached中，必须从数据库获取
        for (size_t i = 0; i < allRouters.size(); ++i)
        {
            // ========== 从Memcached或数据库查询今日流量（方案4：Memcached缓存+数据库） ==========
            // 优化方案：今日流量在采集服务入库时已计算并存储到Memcached和数据库
            // 1. 优先从Memcached读取（< 1ms，最快）
            // 2. 如果Memcached没有，从数据库查询（< 10ms）
            // 3. 查询后写入Memcached（下次走缓存）
            //
            uint64_t todayUpload = 0;
            uint64_t todayDownload = 0;
            bool todayFromCache = false;
            
            // 优先从Memcached读取今日流量
            // Key格式：ap_today_traffic_<ac_ip>_<ap_id>_<date>
            // Value格式：today_upload|today_download|last_upload|last_download|last_time
            // 注意：如果Memcached中有完整值，直接使用；如果没有，从数据库查询
            string todayTrafficKey = "ap_today_traffic_" + acIp + "_" + allRouters[i].routerId + "_" + dateStr;
            string todayTrafficCache;
            
            if (GetCache(todayTrafficKey, todayTrafficCache) == 0 && !todayTrafficCache.empty())
            {
                // 解析Memcached中的今日流量数据
                // 格式1：today_upload|today_download|last_upload|last_download|last_time（完整值）
                // 格式2：last_upload|last_download|last_time（标记值，需要从数据库查询）
                size_t pipeCount = 0;
                for (size_t j = 0; j < todayTrafficCache.length(); ++j)
                {
                    if (todayTrafficCache[j] == '|') pipeCount++;
                }
                
                if (pipeCount >= 4)
                {
                    // 完整值格式（5个字段）
                    size_t pos = 0;
                    size_t pipePos1 = todayTrafficCache.find('|', pos);
                    size_t pipePos2 = todayTrafficCache.find('|', pipePos1 + 1);
                    if (pipePos1 != string::npos && pipePos2 != string::npos)
                    {
                        todayUpload = atoll(todayTrafficCache.substr(0, pipePos1).c_str());
                        todayDownload = atoll(todayTrafficCache.substr(pipePos1 + 1, pipePos2 - pipePos1 - 1).c_str());
                        todayFromCache = true;
                        InfoLog("从Memcached读取到AP[%s]今日流量: 上行=%llu KB, 下行=%llu KB",allRouters[i].routerId.c_str(), todayUpload, todayDownload);
                    }
                }
                else
                {
                    // 标记值格式（3个字段），需要从数据库查询完整值
                    InfoLog("从Memcached检测到AP[%s]今日有流量记录，从数据库查询完整今日流量", allRouters[i].routerId.c_str());
                }
            }
            
            // 声明查询变量（用于今日流量和运行时间查询）
            CStr2Map queryInMap, queryOutMap;
            vector<CStr2Map> queryResult;
            
            // 如果Memcached中没有完整值，从数据库查询
            if (!todayFromCache)
            {
                InfoLog("从数据库查询AP[%s]今日流量", allRouters[i].routerId.c_str());
                
                // 数据库操作：SELECT Ftoday_upload, Ftoday_download, Flast_upload, Flast_download, Flast_time
                //            FROM t_ac_ap_today_traffic 
                //            WHERE Fac_ip = ? AND Fap_id = ? AND Fdate = ?
                // 注意：需要relay服务端实现对应的SQL逻辑（reqid待配置）
                // 性能：O(1)查询，使用唯一索引，毫秒级响应
                queryInMap["ac_ip"] = acIp;
                queryInMap["ap_id"] = allRouters[i].routerId;
                queryInMap["date"] = dateStr;  // 今日日期
                // TODO: 调用新的relay API函数 QueryAcApTodayTraffic（从t_ac_ap_today_traffic表查询）
                // CIdentRelayApi::QueryAcApTodayTraffic(queryInMap, queryOutMap, queryResult, true);
                // 
                // 临时使用旧的查询方式（从t_ac_ap_info表聚合查询，性能较差）
                // 待新接口实现后，替换为上面的QueryAcApTodayTraffic
                CIdentRelayApi::QueryAcApInfoToday(queryInMap, queryOutMap, queryResult, true);
                if (queryResult.size() > 0) {
                    // 使用DelMapF去掉F前缀，或者直接使用带F前缀的字段名
                    CStr2Map resultMap;
                    CIdentComm::DelMapF(queryResult[0], resultMap);
                    
                    todayUpload = atoll(resultMap["today_upload"].c_str());
                    todayDownload = atoll(resultMap["today_download"].c_str());
                    
                    // 查询后写入Memcached（下次走缓存）
                    // 数据库操作：写入Memcached，key=ap_today_traffic_<ac_ip>_<ap_id>_<date>，value=today_upload|today_download|last_upload|last_download|last_time，过期时间=86400秒
                    // 注意：如果字段不存在，使用空字符串或默认值
                    string lastUpload = resultMap.find("last_upload") != resultMap.end() ? resultMap["last_upload"] : "0";
                    string lastDownload = resultMap.find("last_download") != resultMap.end() ? resultMap["last_download"] : "0";
                    string lastTime = resultMap.find("last_time") != resultMap.end() ? resultMap["last_time"] : "";
                    ostringstream todayTrafficOss;
                    todayTrafficOss << todayUpload << "|" << todayDownload << "|" 
                                   << lastUpload << "|" << lastDownload << "|" << lastTime;
                    string todayTrafficValue = todayTrafficOss.str();
                    if (SetOrUpdateCache(todayTrafficKey, todayTrafficValue, 86400) == 0)  // 缓存24小时
                    {
                        InfoLog("AP[%s]今日流量已写入Memcached: key=%s, value=%s", allRouters[i].routerId.c_str(), todayTrafficKey.c_str(), todayTrafficValue.c_str());
                    }
                } else {
                    ErrorLog("从数据库查询今日流量失败: ac_ip=%s, ap_id=%s, date=%s",acIp.c_str(), allRouters[i].routerId.c_str(), dateStr);
                    throw(CTrsExp(ERR_UPFILE_COUNT, "从数据库查询今日流量失败"));
                    return 0;
                }
            }
            
            allRouters[i].uploadToday = todayUpload;
            allRouters[i].downloadToday = todayDownload;
            
            // 从数据库查询运行时间（uptime）- 仅在Memcached中没有时查询
            // 注意：如果Memcached中有uptime数据（fields.size() >= 11），已经设置了uptime，不需要再查询
            // 注意：数据库存储的run_time单位是秒（不是tick，不是centiseconds）
            if (allRouters[i].uptime == 0) {
                queryInMap["ac_ip"] = acIp;
                queryInMap["ap_id"] = allRouters[i].routerId;
                queryInMap["limit"] = "1";
                queryInMap["offset"] = "0";
                CIdentRelayApi::QueryAcApInfoLatestList(queryInMap, queryOutMap, queryResult, true);
                // 从最新记录中获取 Frun_time（单位：秒）
                if (queryResult.size() > 0) {
                    CStr2Map resultMap;
                    CIdentComm::DelMapF(queryResult[0], resultMap);
                    uint64_t runTimeSeconds = atoll(resultMap["run_time"].c_str());
                    allRouters[i].uptime = (uint32_t)runTimeSeconds;  // 直接使用，单位已经是秒
                } else {
                    ErrorLog("从数据库查询运行时间失败: ac_ip=%s, ap_id=%s",acIp.c_str(), allRouters[i].routerId.c_str());
                    throw(CTrsExp(ERR_UPFILE_COUNT, "从数据库查询运行时间失败"));
                    return 0;
                }
            }
            
            // TODO: 从数据库查询温度（如果Memcached中没有）
            // 注意：温度数据应该已经在Memcached中（从CAcDataCollector存储）
            // 如果Memcached中没有，从数据库查询
            if (allRouters[i].temperature == 0) {
                queryInMap["limit"] = "1";
                queryInMap["offset"] = "0";
                queryInMap["ac_ip"] = acIp;
                queryInMap["ap_id"] = allRouters[i].routerId;
                CIdentRelayApi::QueryAcApInfoLatestList(queryInMap, queryOutMap, queryResult, true);
                // 从最新记录中获取温度字段（如果表中有）
                if (queryResult.size() > 0 && queryResult[0].find("Ftemperature") != queryResult[0].end()) {
                    allRouters[i].temperature = atoi(queryResult[0]["Ftemperature"].c_str());
                }
            }
        }
    }
    else
    {
        // Memcached中无AP列表数据，直接报错，不使用SNMP降级
        ErrorLog("从Memcached读取AP列表数据失败: key=%s，数据采集服务可能未运行或数据未写入", apListKey.c_str());
        throw(CTrsExp(ERR_UPFILE_COUNT, "从Memcached读取AP列表数据失败，请检查数据采集服务是否正常运行"));
	return 0;
    }
    
    // ========== 5. 分页处理 ==========
    
    int total = (int)allRouters.size();
    int startIndex = (offset - 1) * limit;  // offset从1开始，转换为数组索引从0开始
    int endIndex = min(startIndex + limit, total);
    int retNum = max(0, endIndex - startIndex);
    
    InfoLog("分页结果: total=%d, startIndex=%d, endIndex=%d, retNum=%d", total, startIndex, endIndex, retNum);
    
    // ========== 6. 设置返回参数 ==========
    
    pResData->SetPara("ret_num", std::to_string(retNum));  // 当前返回的条数（必填）
    pResData->SetPara("total", std::to_string(total));     // 总共有多少记录（必填）
    
    // ========== 7. 设置数组数据 ==========
    
    for (int i = startIndex; i < endIndex && i < total; ++i)
    {
        CStr2Map routerMap;
        RouterDetailInfo& router = allRouters[i];
        
        routerMap["id"] = router.routerId;           // 设备ID（必填）
        routerMap["name"] = router.name;             // 路由器名称（必填）
        routerMap["ip"] = router.ip;                 // IP地址
        routerMap["status"] = router.status;         // 状态：online/offline/fault
        routerMap["connections"] = std::to_string(router.connections);  // 当前连接数
        
        // 上行流量信息（嵌套数组结构）
        // 使用字段名格式：traffic_upload.current 和 traffic_upload.today
        routerMap["traffic_upload.current"] = std::to_string(router.uploadSpeedCurrent);  // 当前上行速率（KB/s）
        routerMap["traffic_upload.today"] = std::to_string(router.uploadToday);            // 今日上行总量（KB）
        
        // 下行流量信息（嵌套数组结构）
        // 使用字段名格式：traffic_download.current 和 traffic_download.today
        routerMap["traffic_download.current"] = std::to_string(router.downloadSpeedCurrent);  // 当前下行速率（KB/s）
        routerMap["traffic_download.today"] = std::to_string(router.downloadToday);          // 今日下行总量（KB）
        
        routerMap["cpu"] = std::to_string(router.cpu);         // CPU使用率（%）
        routerMap["memory"] = std::to_string(router.memory);   // 内存使用率（%）
        
        if (router.temperature > 0 && router.temperature != 255)
        {
            routerMap["temperature"] = std::to_string(router.temperature);  // 温度（℃），可选
        }
        else
            routerMap["temperature"]  = "0";

        routerMap["uptime"] = std::to_string(router.uptime);  // 运行时长（秒）
        routerMap["lastUpdate"] = router.lastUpdate;           // 最后更新时间
        
        
        // 添加到返回数组
        pResData->SetArray(routerMap);
    }
    
    InfoLog("返回数据: total=%d, ret_num=%d", total, retNum);
    
    return 0;
}
