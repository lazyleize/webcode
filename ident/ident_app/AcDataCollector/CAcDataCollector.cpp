/************************************************************
 Desc:     AC控制器数据采集类实现
 Auth:     Auto
 Modify:
 data:     2025-01-29
 ***********************************************************/
#include "CAcDataCollector.h"
#include "CIdentRelayApi.h"
#include "CSnmpClient.h"
#include "transxmlcfg.h"
#include <ctime>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <map>
#include "libmemcached/memcached.h"

extern CIdentAppComm* pIdentAppComm;

// MAC地址转换函数：将十六进制MAC地址转换为OID索引格式
// 输入：Hex-STRING格式，如 "60 53 75 2D B8 80" 或 "60:53:75:2D:B8:80"
// 输出：点分十进制格式，如 "96.83.117.45.184.128"
string CAcDataCollector::ConvertMacToOidIndex(const string& hexMac)
{
    string result;
    stringstream ss;
    
    // 移除空格、冒号、连字符等分隔符
    string cleanMac = hexMac;
    for (size_t i = 0; i < cleanMac.length(); ++i)
    {
        if (cleanMac[i] == ' ' || cleanMac[i] == ':' || cleanMac[i] == '-')
        {
            cleanMac.erase(i, 1);
            --i;
        }
    }
    
    // 转换为点分十进制格式
    if (cleanMac.length() >= 12)  // MAC地址应该是12个十六进制字符
    {
        for (size_t i = 0; i < 12; i += 2)
        {
            if (i > 0) ss << ".";
            string hexByte = cleanMac.substr(i, 2);
            unsigned int byteValue = 0;
            sscanf(hexByte.c_str(), "%x", &byteValue);
            ss << byteValue;
        }
        result = ss.str();
    }
    
    return result;
}

CAcDataCollector::CAcDataCollector() : m_stopped(false), m_acPort(161), m_cachePort(15210), m_lastTrafficTrendTime(0)
{
    m_cacheIp = "127.0.0.1";  // 默认缓存服务器IP
}

CAcDataCollector::~CAcDataCollector()
{
}

int CAcDataCollector::Init()
{
    // 从配置获取AC控制器信息
    // 暂时硬编码，后续可以从配置文件读取
    char ac_ip[128]={0};
    char ac_name[128]={0};

    // 安全地获取配置，避免访问未初始化的map
    string tid = GetTid();  // 先获取tid，避免多次调用
    if (tid.empty())
    {
        ErrorLog("GetTid()返回空字符串，初始化失败");
        return -1;
    }
    
    // 检查g_mTransactions中是否存在该tid
    if (g_mTransactions.find(tid) == g_mTransactions.end())
    {
        ErrorLog("配置中未找到tid=%s，初始化失败", tid.c_str());
        return -1;
    }
    
    // 安全地获取配置值
    string ac_ip_str;
    string ac_name_str;
    
    CStr2Map& mVars = g_mTransactions[tid].m_mVars;
    if (mVars.find("ac_ip") != mVars.end())
    {
        ac_ip_str = mVars["ac_ip"];
    }
    if (mVars.find("ac_community") != mVars.end())
    {
        ac_name_str = mVars["ac_community"];
    }
    if(mVars.find("cache_ip")!=mVars.end())
    {
        m_cacheIp = mVars["cache_ip"];
    }
    if(mVars.find("cache_port")!=mVars.end())
    {
        m_cachePort = atoi(mVars["cache_port"].c_str());
    }
    
    // 如果配置为空，使用默认值或返回错误
    if (ac_ip_str.empty())
    {
        ErrorLog("配置中ac_ip为空，初始化失败");
        return -1;
    }
    
    
    // 安全地复制字符串，确保缓冲区不溢出
    size_t ip_len = (ac_ip_str.length() < sizeof(ac_ip) - 1) ? ac_ip_str.length() : sizeof(ac_ip) - 1;
    size_t name_len = (ac_name_str.length() < sizeof(ac_name) - 1) ? ac_name_str.length() : sizeof(ac_name) - 1;
    
    if (ip_len > 0)
    {
        memcpy(ac_ip, ac_ip_str.c_str(), ip_len);
        ac_ip[ip_len] = '\0';  // 确保字符串终止
    }
    
    if (name_len > 0)
    {
        memcpy(ac_name, ac_name_str.c_str(), name_len);
        ac_name[name_len] = '\0';  // 确保字符串终止
    }

    // 安全地赋值，确保字符串有效
    m_acIp = string(ac_ip);
    m_acCommunity = string(ac_name);
    m_acPort = 161;
    
    // 验证初始化是否成功
    if (m_acIp.empty())
    {
        ErrorLog("AC IP地址为空，初始化失败");
        return -1;
    }
    
    InfoLog("AC数据采集器初始化: IP=%s, Community=%s, Port=%d", m_acIp.c_str(), m_acCommunity.c_str(), m_acPort);
    InfoLog("Memcached缓存配置: IP=%s, Port=%d", m_cacheIp.c_str(), m_cachePort);
    
    return 0;
}

void CAcDataCollector::Cleanup()
{
    // 清理资源
    m_stopped = true;
}

int CAcDataCollector::CollectData(CStr2Map &inMap, CStr2Map &outMap)
{
    InfoLog("开始采集AC控制器数据: IP=%s", m_acIp.c_str());
    
    // 连接AC控制器
    CSnmpClient acClient(m_acIp, m_acPort, m_acCommunity);
    if (acClient.Init() != 0)
    {
        ErrorLog("连接AC控制器失败: %s", acClient.GetLastError().c_str());
        return -1;
    }
    
    // 从AC控制器获取当前连接的AP数量
    // 从AC控制器获取AP总数（使用华为私有OID）
    // 1.3.6.1.4.1.2011.6.139.12.1.5.7.0 - hwWlanApCount (AP总数)
    string apCountOid = "1.3.6.1.4.1.2011.6.139.12.1.5.7.0";
    int64_t apCount = 0;
    if (acClient.GetInt(apCountOid, apCount) == 0)
    {
        InfoLog("从AC控制器获取到AP总数: %lld (OID: %s)", apCount, apCountOid.c_str());
    }
    else
    {
        // 降级方案：尝试旧的OID
        apCountOid = "1.2.156.11235.6001.60.7.2.75.1.1.2.1.0";
        if (acClient.GetInt(apCountOid, apCount) == 0)
        {
            InfoLog("从AC控制器获取到当前连接的AP数量: %lld (降级OID: %s)", apCount, apCountOid.c_str());
        }
        else
        {
            InfoLog("无法从AC控制器获取AP数量，将使用Walk方式获取");
        }
    }
    
    // 从AC控制器获取当前关联总用户数
    string userCountOid = "1.2.156.11235.6001.60.7.2.75.1.1.2.2.0";
    int64_t userCount = 0;
    acClient.GetInt(userCountOid, userCount);
    
    // 获取终端关联统计信息
    int64_t stationAssocSum = 0;
    int64_t stationAssocFailSum = 0;
    int64_t stationReassocSum = 0;
    int64_t stationAssocRejectedSum = 0;
    int64_t stationExDeAuthenSum = 0;
    
    string assocSumOid = "1.2.156.11235.6001.60.7.2.75.1.1.3.1.0";
    acClient.GetInt(assocSumOid, stationAssocSum);
    
    string assocFailOid = "1.2.156.11235.6001.60.7.2.75.1.1.3.2.0";
    acClient.GetInt(assocFailOid, stationAssocFailSum);
    
    string reassocOid = "1.2.156.11235.6001.60.7.2.75.1.1.3.3.0";
    acClient.GetInt(reassocOid, stationReassocSum);
    
    string rejectedOid = "1.2.156.11235.6001.60.7.2.75.1.1.3.4.0";
    acClient.GetInt(rejectedOid, stationAssocRejectedSum);
    
    string exDeAuthOid = "1.2.156.11235.6001.60.7.2.75.1.1.3.5.0";
    acClient.GetInt(exDeAuthOid, stationExDeAuthenSum);
    
    // 从AC控制器获取路由器列表
    vector<RouterInfo> routers = GetRouterListFromAC(acClient);
    InfoLog("获取到 %zu 个AP设备", routers.size());
    
    // 获取每个AP的详细信息
    for (size_t i = 0; i < routers.size(); ++i)
    {
        GetRouterInfoFromAC(acClient, routers[i]);
    }
    
    // 清理AC控制器连接
    acClient.Cleanup();
    
    // 计算统计数据
    AcStatsData stats;
    CalculateStats(routers, apCount, userCount, 
                   stationAssocSum, stationAssocFailSum, stationReassocSum,
                   stationAssocRejectedSum, stationExDeAuthenSum, stats);
    
    // ========== 数据存储策略 ==========
    // 
    // 1. 实时统计数据 -> Memcached（快速查询，CGI程序读取）
    //    - 统计数据（totalRouters, onlineRouters, avgCpu等）
    //    - AP设备列表（实时状态、流量、CPU、内存）
    //    用途：CGI程序快速读取，响应前端请求
    //    过期时间：不过期（由采集服务定期更新）
    //
    // 2. 历史数据 -> MySQL数据库（持久化存储，支持趋势分析）
    //    - 统计数据历史记录（t_ac_controller_stats表）
    //    - AP设备详细信息历史记录（t_ac_ap_info表）
    //    用途：历史查询、趋势分析、报表统计
    //    保留时间：建议至少30天
    //
    // ====================================
    
    // 存储实时数据到Memcached（供CGI快速读取）
    if (SaveToMemcached(stats, routers) == 0)
    {
        InfoLog("实时数据已成功存储到Memcached");
    }
    else
    {
        ErrorLog("存储实时数据到Memcached失败");
    }
    
    // 存储历史数据到MySQL数据库（非实时数据，带备注说明）
    if (SaveToMySQL(stats, routers) == 0)
    {
        InfoLog("历史数据已成功存储到MySQL数据库");
    }
    else
    {
        ErrorLog("存储历史数据到MySQL数据库失败");
    }
    
    return 0;
}

// 计算统计数据（复用CAcControllerStats的逻辑）
void CAcDataCollector::CalculateStats(const vector<RouterInfo>& routers,int64_t apCount, int64_t userCount,int64_t stationAssocSum, int64_t stationAssocFailSum,int64_t stationReassocSum, int64_t stationAssocRejectedSum,int64_t stationExDeAuthenSum,AcStatsData& stats)
{
    // 获取当前日期
    time_t now = time(NULL);
    struct tm tm_now;
    struct tm *tm_now_ptr = localtime(&now);
    if (tm_now_ptr != NULL)
    {
        tm_now = *tm_now_ptr;  // 复制结构体，避免后续localtime调用覆盖
    }
    else
    {
        memset(&tm_now, 0, sizeof(tm_now));
    }
    char dateStr[32];
    snprintf(dateStr, sizeof(dateStr), "%04d-%02d-%02d", 
             tm_now.tm_year + 1900, tm_now.tm_mon + 1, tm_now.tm_mday);
    stats.date = dateStr;
    
    // 统计数据
    int totalRouters = (apCount > 0) ? (int)apCount : (int)routers.size();
    int onlineRouters = 0;
    int offlineRouters = 0;
    int faultRouters = 0;
    
    // 验证：如果AP总数和已查到的AP数量不一致，记录警告
    if (apCount > 0 && (int)routers.size() < apCount)
    {
        ErrorLog("AP数量不一致: AC控制器报告总数=%d, 实际查到=%zu, 缺失=%d台AP",apCount, routers.size(), apCount - (int)routers.size());
    }
    
    int totalCpu = 0;
    int totalMemory = 0;
    int onlineCount = 0;
    uint64_t totalUploadToday = 0;
    uint64_t totalDownloadToday = 0;
    uint64_t totalUploadMonth = 0;
    uint64_t totalDownloadMonth = 0;
    
    // 遍历所有路由器，统计数据
    for (size_t i = 0; i < routers.size(); ++i)
    {
        // 统计状态
        if (routers[i].status == "online")
        {
            onlineRouters++;
            onlineCount++;
            
            // 累计CPU和内存
            if (routers[i].cpu > 0)
            {
                totalCpu += routers[i].cpu;
            }
            if (routers[i].memory > 0)
            {
                totalMemory += routers[i].memory;
            }
            
            // 累计流量
            totalUploadToday += routers[i].upload;
            totalDownloadToday += routers[i].download;
            // 本月流量暂时使用今日流量的30倍作为示例
            totalUploadMonth += routers[i].upload * 30;
            totalDownloadMonth += routers[i].download * 30;
        }
        else if (routers[i].status == "fault")
        {
            faultRouters++;
        }
        else
        {
            offlineRouters++;
        }
    }
    
    // 计算平均值
    int avgCpu = onlineCount > 0 ? (totalCpu / onlineCount) : 0;
    int avgMemory = onlineCount > 0 ? (totalMemory / onlineCount) : 0;
    
    // 填充统计数据
    stats.totalRouters = totalRouters;
    stats.onlineRouters = onlineRouters;
    stats.offlineRouters = offlineRouters;
    stats.faultRouters = faultRouters;
    stats.avgCpu = avgCpu;
    stats.avgMemory = avgMemory;
    stats.totalUploadToday = totalUploadToday;
    stats.totalDownloadToday = totalDownloadToday;
    stats.totalUploadMonth = totalUploadMonth;
    stats.totalDownloadMonth = totalDownloadMonth;
    stats.totalUsers = userCount;
    stats.stationAssocSum = stationAssocSum;
    stats.stationAssocFailSum = stationAssocFailSum;
    stats.stationReassocSum = stationReassocSum;
    stats.stationAssocRejectedSum = stationAssocRejectedSum;
    stats.stationExDeAuthenSum = stationExDeAuthenSum;
    
    InfoLog("统计数据: 总数=%d, 在线=%d, 离线=%d, 故障=%d, 平均CPU=%d%%, 平均内存=%d%%",totalRouters, onlineRouters, offlineRouters, faultRouters, avgCpu, avgMemory);
}

// 存储实时数据到Memcached（供CGI程序快速读取）
// 
// 存储内容：
// 1. 统计数据（ac_stats_<ac_ip>）：实时统计信息，用于前端展示
// 2. AP列表数据（ac_ap_list_<ac_ip>）：AP设备列表，包含实时状态和性能数据
//
// 注意：这些数据是实时数据，不过期，由采集服务定期更新
//      CGI程序读取这些数据时，总是获取到最新的采集结果
int CAcDataCollector::SaveToMemcached(const AcStatsData& stats, const vector<RouterInfo>& routers)
{
    // 直接使用memcached API存储实时数据
    // 创建memcached连接
    memcached_st* pMemc = memcached_create(NULL);
    if (pMemc == NULL)
    {
        ErrorLog("memcached_create failed");
        return -1;
    }
    
    // 连接memcached服务器（使用配置文件中的配置）
    memcached_return_t rc = memcached_server_add(pMemc, m_cacheIp.c_str(), m_cachePort);
    if (rc != MEMCACHED_SUCCESS)
    {
        ErrorLog("memcached_server_add failed: %s", memcached_strerror(pMemc, rc));
        memcached_free(pMemc);
        return -1;
    }
    
    // ========== 存储统计数据到Memcached ==========
    // Key: ac_stats_<ac_ip>
    // 内容：实时统计数据（用于前端展示）
    // 格式：key=value&key=value（简单格式，便于CGI程序解析）
    // 不过期：由采集服务定期更新，CGI总是读取最新数据
    // ============================================
    ostringstream oss;
    oss << "date=" << stats.date << "&"
        << "totalRouters=" << stats.totalRouters << "&"
        << "onlineRouters=" << stats.onlineRouters << "&"
        << "offlineRouters=" << stats.offlineRouters << "&"
        << "faultRouters=" << stats.faultRouters << "&"
        << "avgCpu=" << stats.avgCpu << "&"
        << "avgMemory=" << stats.avgMemory << "&"
        << "totalUploadToday=" << stats.totalUploadToday << "&"
        << "totalDownloadToday=" << stats.totalDownloadToday << "&"
        << "totalUploadMonth=" << stats.totalUploadMonth << "&"
        << "totalDownloadMonth=" << stats.totalDownloadMonth << "&"
        << "totalUsers=" << stats.totalUsers << "&"
        << "stationAssocSum=" << stats.stationAssocSum << "&"
        << "stationAssocFailSum=" << stats.stationAssocFailSum << "&"
        << "stationReassocSum=" << stats.stationReassocSum << "&"
        << "stationAssocRejectedSum=" << stats.stationAssocRejectedSum << "&"
        << "stationExDeAuthenSum=" << stats.stationExDeAuthenSum;
    
    string statsData = oss.str();
    
    // 存储统计数据（key: ac_stats_<ac_ip>）
    string statsKey = "ac_stats_" + m_acIp;
    rc = memcached_set(pMemc, statsKey.c_str(), statsKey.length(),statsData.c_str(), statsData.length(),0, 0);  // 持久
    
    if (rc == MEMCACHED_SUCCESS)
    {
        InfoLog("统计数据存储成功: key=%s", statsKey.c_str());
    }
    else
    {
        ErrorLog("统计数据存储失败: key=%s, error=%s",statsKey.c_str(), memcached_strerror(pMemc, rc));
    }
    
    // ========== 存储AP列表数据到Memcached ==========
    // Key: ac_ap_list_<ac_ip>
    // 内容：AP设备列表（实时状态、性能数据、实时速率、运行时间、温度）
    // 格式：AP1数据|AP2数据|AP3数据（用|分隔，每个AP用,分隔字段）
    // 字段顺序：routerId,name,ip,status,cpu,memory,onlineUsers,uploadSpeed,downloadSpeed,uptime,temperature
    // 注意：实时数据（CPU、内存、在线用户、实时速率、运行时间、温度）写入Memcached
    //      非实时数据（累计流量）写入MySQL
    // 不过期：由采集服务定期更新
    // ==============================================
    ostringstream apListOss;
    for (size_t i = 0; i < routers.size(); ++i)
    {
        if (i > 0) apListOss << "|";
        // 运行时间（tick，单位：秒）
        // 注意：hwWlanIDIndexedApRunTime的单位是秒（1 tick = 1秒），不是centiseconds
        uint32_t uptimeSeconds = routers[i].runTime;  // tick就是秒（1 tick = 1秒）
        // 温度（如果为255表示不支持，存储为0）
        int temperature = (routers[i].temperature == 255) ? 0 : routers[i].temperature;
        
        apListOss << routers[i].routerId << ","
                  << routers[i].name << ","
                  << routers[i].ip << ","
                  << routers[i].status << ","
                  << routers[i].cpu << ","
                  << routers[i].memory << ","
                  << routers[i].onlineUsers << ","
                  << routers[i].uploadSpeed << ","
                  << routers[i].downloadSpeed << ","
                  << uptimeSeconds << ","
                  << temperature;  // 运行时间（秒）、温度（℃）
    }
    
    string apListData = apListOss.str();
    string apListKey = "ac_ap_list_" + m_acIp;
    rc = memcached_set(pMemc, apListKey.c_str(), apListKey.length(),apListData.c_str(), apListData.length(),0, 0);  // 不过期
    
    if (rc == MEMCACHED_SUCCESS)
    {
        InfoLog("AP列表数据存储成功: key=%s, count=%zu", apListKey.c_str(), routers.size());
    }
    else
    {
        ErrorLog("AP列表数据存储失败: key=%s, error=%s",apListKey.c_str(), memcached_strerror(pMemc, rc));
    }
    
    memcached_free(pMemc);
    return (rc == MEMCACHED_SUCCESS) ? 0 : -1;
}

// 存储历史数据到MySQL数据库（非实时数据，带备注说明）
// 存储内容：
// 1. 统计数据历史记录 -> t_ac_controller_stats 表
// 2. AP设备详细信息历史记录 -> t_ac_ap_info 表（累计流量、运行时间等）
// 实现方式：通过CIdentRelayApi调用relay服务执行数据库操作
// 备注：在Fremark字段中说明数据来源、缺失字段等信息
int CAcDataCollector::SaveToMySQL(const AcStatsData& stats, const vector<RouterInfo>& routers)
{
    InfoLog("开始保存历史数据到MySQL数据库");
    
    // 获取当前时间
    time_t now = time(NULL);
    struct tm tm_now;
    struct tm *tm_now_ptr = localtime(&now);
    if (tm_now_ptr != NULL)
    {
        tm_now = *tm_now_ptr;  // 复制结构体，避免后续localtime调用覆盖
    }
    else
    {
        ErrorLog("localtime失败，使用当前时间");
        memset(&tm_now, 0, sizeof(tm_now));
    }
    
    char timeStr[64];
    snprintf(timeStr, sizeof(timeStr), "%04d-%02d-%02d %02d:%02d:%02d",
             tm_now.tm_year + 1900, tm_now.tm_mon + 1, tm_now.tm_mday,
             tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec);
    
    // 获取当前日期字符串（用于Memcached key）
    char dateStr[32] = {0};  // 初始化为0，确保字符串终止
    snprintf(dateStr, sizeof(dateStr), "%04d-%02d-%02d",tm_now.tm_year + 1900, tm_now.tm_mon + 1, tm_now.tm_mday);
    dateStr[sizeof(dateStr) - 1] = '\0';  // 确保字符串终止
    
    // ========== 1. 计算瞬时速率并更新峰值流量 ==========
    // 峰值流量计算逻辑：
    // 1. 从内存缓存获取上一条记录的累计流量
    // 2. 计算瞬时速率 = (当前累计流量 - 上一条累计流量) / 时间差
    // 3. 查询今日峰值（从Memcached或数据库）
    // 4. 如果瞬时速率 > 当前峰值，更新峰值记录
    // 5. 缓存当前记录到内存（供下次使用）
    
    uint64_t currentTotalTraffic = stats.totalUploadToday + stats.totalDownloadToday;  // 当前累计总流量（KB）
    int64_t instantTraffic = 0;  // 瞬时速率（KB/s）
    time_t currentTime = now;
    
    // 保存上一次的采集数据（在更新m_lastRecord之前）
    // 这些值将在后续计算流量趋势时使用
    time_t lastCollectTime = m_lastRecord.collectTime;
    uint64_t lastTotalUpload = m_lastRecord.totalUpload;
    uint64_t lastTotalDownload = m_lastRecord.totalDownload;
    
    // 计算瞬时速率
    if (m_lastRecord.valid && m_lastRecord.collectTime > 0)
    {
        uint64_t lastTotalTraffic = m_lastRecord.totalUpload + m_lastRecord.totalDownload;
        int64_t timeDiff = currentTime - m_lastRecord.collectTime;
        
        if (timeDiff > 0 && currentTotalTraffic >= lastTotalTraffic)
        {
            // 瞬时速率 = (当前累计流量 - 上一条累计流量) / 时间差（秒）
            instantTraffic = (int64_t)((currentTotalTraffic - lastTotalTraffic) / timeDiff);
            InfoLog("计算瞬时速率: 当前累计=%llu KB, 上一条累计=%llu KB, 时间差=%lld秒, 瞬时速率=%lld KB/s",currentTotalTraffic, lastTotalTraffic, timeDiff, instantTraffic);
        }
        else
        {
            InfoLog("无法计算瞬时速率: 时间差=%lld秒, 当前累计=%llu KB, 上一条累计=%llu KB",timeDiff, currentTotalTraffic, lastTotalTraffic);
        }
    }
    else
    {
        InfoLog("第一条记录，无法计算瞬时速率（需要上一条记录）");
    }
    
    // 查询今日峰值并更新（如果瞬时速率更大）
    if (instantTraffic > 0)
    {
        // 优先从Memcached读取今日峰值
        // 注意：这里需要创建memcached连接来读取峰值数据
        // 由于SaveToMemcached函数中已经创建了连接，可以考虑复用
        // 但为了代码清晰，这里先查询，如果需要在SaveToMemcached中统一处理
        string peakKey = "ac_peak_traffic_" + m_acIp + "_" + dateStr;
        int64_t currentPeakTraffic = 0;
        string currentPeakTime = "";
        
        // 创建memcached连接读取峰值数据
        // 数据库操作：从Memcached读取，如果不存在则从数据库查询
        memcached_st* pMemcPeak = memcached_create(NULL);
        if (pMemcPeak != NULL)
        {
            if (memcached_server_add(pMemcPeak, m_cacheIp.c_str(), m_cachePort) == MEMCACHED_SUCCESS)
            {
                size_t valueLength = 0;
                uint32_t flags = 0;
                char* peakValue = memcached_get(pMemcPeak, peakKey.c_str(), peakKey.length(), 
                                                &valueLength, &flags, NULL);
                if (peakValue != NULL)
                {
                    // 解析Memcached中的峰值数据（格式：peak_traffic|peak_time）
                    string peakCache(peakValue, valueLength);
                    free(peakValue);
                    size_t pipePos = peakCache.find('|');
                    if (pipePos != string::npos)
                    {
                        currentPeakTraffic = atoll(peakCache.substr(0, pipePos).c_str());
                        currentPeakTime = peakCache.substr(pipePos + 1);
                        InfoLog("从Memcached读取今日峰值: %lld KB/s, 时间: %s", currentPeakTraffic, currentPeakTime.c_str());
                    }
                }
            }
            memcached_free(pMemcPeak);
        }
        
        // 如果瞬时速率大于当前峰值，更新峰值
        if (instantTraffic > currentPeakTraffic)
        {
            currentPeakTraffic = instantTraffic;
            currentPeakTime = timeStr;
            InfoLog("发现新的峰值流量: %lld KB/s, 时间: %s", currentPeakTraffic, currentPeakTime.c_str());
            
            // 更新Memcached（立即生效，供查询接口快速读取）
            // 格式：peak_traffic|peak_time
            // 数据库操作：写入Memcached，key=ac_peak_traffic_<ac_ip>_<date>，value=peak_traffic|peak_time，过期时间=86400秒
            ostringstream peakOss;
            peakOss << currentPeakTraffic << "|" << currentPeakTime;
            string peakValue = peakOss.str();
            
            memcached_st* pMemcSet = memcached_create(NULL);
            if (pMemcSet != NULL)
            {
                if (memcached_server_add(pMemcSet, m_cacheIp.c_str(), m_cachePort) == MEMCACHED_SUCCESS)
                {
                    memcached_return_t rc = memcached_set(pMemcSet, peakKey.c_str(), peakKey.length(),
                                                         peakValue.c_str(), peakValue.length(),
                                                         86400, 0);  // 缓存24小时
                    if (rc == MEMCACHED_SUCCESS)
                    {
                        InfoLog("峰值流量已更新到Memcached: key=%s, value=%s", peakKey.c_str(), peakValue.c_str());
                    }
                    else
                    {
                        ErrorLog("更新峰值流量到Memcached失败: key=%s, error=%s", 
                                peakKey.c_str(), memcached_strerror(pMemcSet, rc));
                    }
                }
                memcached_free(pMemcSet);
            }
            
            // 更新数据库（持久化存储）
            // 数据库操作：INSERT INTO t_ac_controller_peak_traffic (Fac_ip, Fdate, Fpeak_traffic, Fpeak_time, Fupdate_time)
            //            VALUES (?, ?, ?, ?, ?)
            //            ON DUPLICATE KEY UPDATE Fpeak_traffic=?, Fpeak_time=?, Fupdate_time=?
            CStr2Map peakInMap, peakOutMap;
            peakInMap["ac_ip"] = m_acIp;
            peakInMap["date"] = dateStr;
            ostringstream peakTrafficOss;
            peakTrafficOss << currentPeakTraffic;
            peakInMap["peak_traffic"] = peakTrafficOss.str();
            peakInMap["peak_time"] = currentPeakTime;
            peakInMap["update_time"] = timeStr;
            
            // 调用relay服务保存峰值数据
            // 注意：需要创建新的relay API函数 InsertAcControllerPeakTraffic
            if (CIdentRelayApi::InsertAcControllerPeakTraffic(peakInMap, peakOutMap, false))
            {
                InfoLog("峰值流量已成功保存到数据库: %lld KB/s, 时间: %s", currentPeakTraffic, currentPeakTime.c_str());
            }
            else
            {
                ErrorLog("保存峰值流量到数据库失败: %s", peakOutMap["errormessage"].c_str());
            }
        }
    }
    
    // 更新内存缓存（供下次使用）
    m_lastRecord.totalUpload = stats.totalUploadToday;
    m_lastRecord.totalDownload = stats.totalDownloadToday;
    m_lastRecord.collectTime = currentTime;
    m_lastRecord.valid = true;
    
    // ========== 2. 保存统计数据到 t_ac_controller_stats 表 ==========
    CStr2Map statsInMap, statsOutMap;
    statsInMap["collect_time"] = timeStr;
    statsInMap["ac_ip"] = m_acIp;
    // 使用stringstream转换整数为字符串
    ostringstream oss;
    oss << stats.totalRouters; statsInMap["total_ap_count"] = oss.str(); oss.str("");
    oss << stats.onlineRouters; statsInMap["online_ap_count"] = oss.str(); oss.str("");
    oss << stats.offlineRouters; statsInMap["offline_ap_count"] = oss.str(); oss.str("");
    oss << stats.faultRouters; statsInMap["fault_ap_count"] = oss.str(); oss.str("");
    oss << stats.totalUsers; statsInMap["total_users"] = oss.str(); oss.str("");
    oss << stats.avgCpu; statsInMap["avg_cpu"] = oss.str(); oss.str("");
    oss << stats.avgMemory; statsInMap["avg_memory"] = oss.str(); oss.str("");
    oss << stats.totalUploadToday; statsInMap["total_upload_today"] = oss.str(); oss.str("");
    oss << stats.totalDownloadToday; statsInMap["total_download_today"] = oss.str(); oss.str("");
    oss << stats.totalUploadMonth; statsInMap["total_upload_month"] = oss.str(); oss.str("");
    oss << stats.totalDownloadMonth; statsInMap["total_download_month"] = oss.str(); oss.str("");
    oss << stats.stationAssocSum; statsInMap["station_assoc_sum"] = oss.str(); oss.str("");
    oss << stats.stationAssocFailSum; statsInMap["station_assoc_fail_sum"] = oss.str(); oss.str("");
    oss << stats.stationReassocSum; statsInMap["station_reassoc_sum"] = oss.str(); oss.str("");
    oss << stats.stationAssocRejectedSum; statsInMap["station_assoc_rejected_sum"] = oss.str(); oss.str("");
    oss << stats.stationExDeAuthenSum; statsInMap["station_ex_deauthen_sum"] = oss.str();
    
    // 备注：说明数据来源和缺失字段
    ostringstream statsRemark;
    statsRemark << "数据来源: SNMP采集(AC控制器IP=" << m_acIp << "); ";
    statsRemark << "采集时间: " << timeStr << "; ";
    statsRemark << "数据说明: 实时数据(CPU/内存/用户数)已写入Memcached, 历史数据写入MySQL; ";
    statsRemark << "缺失字段: 无";
    statsInMap["remark"] = statsRemark.str();
    
    // 调用relay服务保存统计数据
    if (CIdentRelayApi::InsertAcControllerStats(statsInMap, statsOutMap, false))
    {
        InfoLog("统计数据已成功保存到t_ac_controller_stats表");
    }
    else
    {
        ErrorLog("保存统计数据到t_ac_controller_stats表失败: %s", statsOutMap["errormessage"].c_str());
    }
    
    // ========== 2. 保存AP详细信息到 t_ac_ap_info 表 ==========
    for (size_t i = 0; i < routers.size(); ++i)
    {
        CStr2Map apInMap, apOutMap;
        apInMap["collect_time"] = timeStr;
        apInMap["ac_ip"] = m_acIp;
        apInMap["ap_id"] = routers[i].routerId;
        apInMap["ap_name"] = routers[i].name;
        apInMap["ap_ip"] = routers[i].ip;
        apInMap["status"] = routers[i].status;
        ostringstream apOss;
        apOss << routers[i].cpu; apInMap["cpu"] = apOss.str(); apOss.str("");
        apOss << routers[i].memory; apInMap["memory"] = apOss.str(); apOss.str("");
        apOss << routers[i].upload; apInMap["upload"] = apOss.str(); apOss.str("");  // 累计流量（KB）
        apOss << routers[i].download; apInMap["download"] = apOss.str(); apOss.str("");  // 累计流量（KB）
        apOss << routers[i].runTime; apInMap["run_time"] = apOss.str(); apOss.str("");  // 运行时间（秒）
        apOss << routers[i].onlineTime; apInMap["online_time"] = apOss.str();  // 上线时长（秒）
        
        // 备注：说明数据来源和缺失字段
        ostringstream apRemark;
        apRemark << "数据来源: SNMP采集(AC控制器IP=" << m_acIp << ", AP ID=" << routers[i].routerId << "); ";
        apRemark << "采集时间: " << timeStr << "; ";
        apRemark << "数据说明: 实时数据(CPU/内存/在线用户/实时速率)已写入Memcached, 历史数据(累计流量/运行时间)写入MySQL; ";
        apRemark << "运行时间: " << routers[i].runTime << " 秒; ";
        apRemark << "上线时长: " << routers[i].onlineTime << " 秒";
        apInMap["remark"] = apRemark.str();
        
        // 调用relay服务保存AP信息
        if (CIdentRelayApi::InsertAcApInfo(apInMap, apOutMap, false))
        {
            InfoLog("AP[%s]信息已成功保存到t_ac_ap_info表", routers[i].name.c_str());
        }
        else
        {
            ErrorLog("保存AP[%s]信息到t_ac_ap_info表失败: %s", routers[i].name.c_str(), apOutMap["errormessage"].c_str());
        }
        
        // ========== 3. 计算并存储AP今日流量 ==========
        // 今日流量计算逻辑
        // 1. 从内存缓存获取该AP上一条记录的累计流量
        // 2. 计算本次增量 = 当前累计流量 - 上一条累计流量
        // 3. 如果上一条记录是今天的，累加到今日流量；如果是新的一天，重置今日流量
        // 4. 更新今日流量到 t_ac_ap_today_traffic 表（增量累加方式）
        // 5. 缓存当前记录到内存（供下次使用）
        
        // 使用已声明的dateStr（第472行已声明）
        
        // 获取该AP的上一条记录
        map<string, ApLastRecord>::iterator it = m_apLastRecords.find(routers[i].routerId);
        ApLastRecord* apLastRecord = NULL;
        if (it != m_apLastRecords.end())
        {
            apLastRecord = &(it->second);
        }
        
        // 计算本次增量
        uint64_t incrementUpload = 0;
        uint64_t incrementDownload = 0;
        uint64_t firstUpload = routers[i].upload;
        uint64_t firstDownload = routers[i].download;
        string firstTime = timeStr;
        string lastTime = timeStr;
        
        if (apLastRecord != NULL && apLastRecord->valid)
        {
            // 检查上一条记录是否是今天的
            struct tm lastTm;
            struct tm* lastTm_ptr = localtime(&(apLastRecord->collectTime));
            if (lastTm_ptr != NULL)
            {
                lastTm = *lastTm_ptr;  // 复制结构体，避免后续localtime调用覆盖
            }
            else
            {
                ErrorLog("localtime失败，使用当前时间");
                memset(&lastTm, 0, sizeof(lastTm));
            }
            char lastDateStr[32];
            snprintf(lastDateStr, sizeof(lastDateStr), "%04d-%02d-%02d", 
                     lastTm.tm_year + 1900, lastTm.tm_mon + 1, lastTm.tm_mday);
            
            if (string(lastDateStr) == string(dateStr))
            {
                // 同一天，计算增量
                if (routers[i].upload >= apLastRecord->upload)
                {
                    incrementUpload = routers[i].upload - apLastRecord->upload;
                }
                if (routers[i].download >= apLastRecord->download)
                {
                    incrementDownload = routers[i].download - apLastRecord->download;
                }
                // 首次记录从数据库查询（如果不存在则使用当前值）
                // 这里简化处理：首次记录在第一次插入时设置，后续更新时不改变
                firstUpload = 0;  // 0表示不更新首次记录
                firstDownload = 0;
                firstTime = "";  // 空字符串表示不更新首次时间
            }
            else
            {
                // 新的一天，本次增量就是今日流量（首次记录）
                incrementUpload = 0;  // 新的一天，首次记录从当前值开始
                incrementDownload = 0;
                firstUpload = routers[i].upload;
                firstDownload = routers[i].download;
                firstTime = timeStr;
            }
        }
        else
        {
            // 第一条记录（程序启动后第一次采集该AP）
            // 需要从数据库读取上一次的累计流量作为基准值
            // 这样可以正确计算增量，避免程序重启导致今日流量丢失
            InfoLog("AP[%s]第一次采集，尝试从数据库恢复上一次的累计流量基准值", routers[i].name.c_str());
            
            // 从数据库查询今日流量记录（获取last_upload/last_download作为基准）
            CStr2Map todayQueryInMap, todayQueryOutMap;
            vector<CStr2Map> todayQueryResult;
            todayQueryInMap["ac_ip"] = m_acIp;
            todayQueryInMap["ap_id"] = routers[i].routerId;
            todayQueryInMap["date"] = dateStr;
            
            if (CIdentRelayApi::QueryAcApInfoToday(todayQueryInMap, todayQueryOutMap, todayQueryResult, false) 
                && todayQueryResult.size() > 0)
            {
                // 今日已有记录，从数据库恢复基准值
                // 直接使用带F前缀的字段名（数据库字段命名规范）
                uint64_t lastUploadFromDb = atoll(todayQueryResult[0]["Flast_upload"].c_str());
                uint64_t lastDownloadFromDb = atoll(todayQueryResult[0]["Flast_download"].c_str());
                
                // 计算增量（当前累计 - 数据库中的上次累计）
                if (routers[i].upload >= lastUploadFromDb)
                {
                    incrementUpload = routers[i].upload - lastUploadFromDb;
                }
                if (routers[i].download >= lastDownloadFromDb)
                {
                    incrementDownload = routers[i].download - lastDownloadFromDb;
                }
                
                // 首次记录不更新（数据库已有）
                firstUpload = 0;
                firstDownload = 0;
                firstTime = "";
                
                InfoLog("AP[%s]从数据库恢复基准值: lastUpload=%llu KB, lastDownload=%llu KB, 计算增量: 上行=%llu KB, 下行=%llu KB",
                       routers[i].name.c_str(), lastUploadFromDb, lastDownloadFromDb, incrementUpload, incrementDownload);
            }
            else
            {
                // 今日无记录，第一次插入，增量为0
                incrementUpload = 0;
                incrementDownload = 0;
                firstUpload = routers[i].upload;
                firstDownload = routers[i].download;
                firstTime = timeStr;
                InfoLog("AP[%s]今日无记录，首次插入: firstUpload=%llu KB, firstDownload=%llu KB",
                       routers[i].name.c_str(), firstUpload, firstDownload);
            }
        }
        
        CStr2Map todayTrafficInMap, todayTrafficOutMap;
        todayTrafficInMap["ac_ip"] = m_acIp;
        todayTrafficInMap["ap_id"] = routers[i].routerId;
        todayTrafficInMap["date"] = dateStr;
        ostringstream todayOss;
        todayOss << incrementUpload; todayTrafficInMap["today_upload"] = todayOss.str(); todayOss.str("");  // 增量
        todayOss << incrementDownload; todayTrafficInMap["today_download"] = todayOss.str(); todayOss.str("");  // 增量
        todayOss << firstUpload; todayTrafficInMap["first_upload"] = todayOss.str(); todayOss.str("");
        todayOss << firstDownload; todayTrafficInMap["first_download"] = todayOss.str(); todayOss.str("");
        todayOss << routers[i].upload; todayTrafficInMap["last_upload"] = todayOss.str(); todayOss.str("");
        todayOss << routers[i].download; todayTrafficInMap["last_download"] = todayOss.str(); todayOss.str("");
        todayTrafficInMap["first_time"] = firstTime;
        todayTrafficInMap["last_time"] = lastTime;
        todayTrafficInMap["update_time"] = timeStr;
        
        // 调用relay服务保存今日流量到数据库
        // 注意：需要创建新的relay API函数 InsertAcApTodayTraffic
        if (CIdentRelayApi::InsertAcApTodayTraffic(todayTrafficInMap, todayTrafficOutMap, false))
        {
            InfoLog("AP[%s]今日流量已成功保存到数据库: 上行增量=%llu KB, 下行增量=%llu KB", 
                   routers[i].name.c_str(), incrementUpload, incrementDownload);
            
            // ========== 同时写入Memcached（供查询接口快速读取） ==========
            // Key格式：ap_today_traffic_<ac_ip>_<ap_id>_<date>
            // Value格式：today_upload|today_download|last_upload|last_download|last_time
            // 注意：需要从数据库查询当前累计的今日流量（因为增量方式，需要累加）
            // 简化方案：Memcached中存储最后流量值作为标记，查询接口从数据库获取完整今日流量
            // 或者：在数据库更新成功后，从数据库查询完整值再写入Memcached
            // 这里先存储标记值，查询接口检测到标记后从数据库获取完整值
            string todayTrafficKey = "ap_today_traffic_" + m_acIp + "_" + routers[i].routerId + "_" + dateStr;
            ostringstream todayTrafficOss;
            // 存储最后流量值作为标记（表示今日有数据）
            todayTrafficOss << routers[i].upload << "|" << routers[i].download << "|" << lastTime;
            string todayTrafficValue = todayTrafficOss.str();
            
            memcached_st* pMemcToday = memcached_create(NULL);
            if (pMemcToday != NULL)
            {
                if (memcached_server_add(pMemcToday, m_cacheIp.c_str(), m_cachePort) == MEMCACHED_SUCCESS)
                {
                    memcached_return_t rc = memcached_set(pMemcToday, todayTrafficKey.c_str(), todayTrafficKey.length(),
                                                         todayTrafficValue.c_str(), todayTrafficValue.length(),
                                                         86400, 0);  // 缓存24小时
                    if (rc == MEMCACHED_SUCCESS)
                    {
                        InfoLog("AP[%s]今日流量已更新到Memcached: key=%s", routers[i].name.c_str(), todayTrafficKey.c_str());
                    }
                    else
                    {
                        ErrorLog("更新AP[%s]今日流量到Memcached失败: key=%s, error=%s",routers[i].name.c_str(), todayTrafficKey.c_str(), memcached_strerror(pMemcToday, rc));
                    }
                }
                memcached_free(pMemcToday);
            }
        }
        else
        {
            ErrorLog("保存AP[%s]今日流量到数据库失败: %s", routers[i].name.c_str(), todayTrafficOutMap["errormessage"].c_str());
        }
        
        // 更新内存缓存（供下次使用）
        if (apLastRecord == NULL)
        {
            ApLastRecord newRecord;
            newRecord.upload = routers[i].upload;
            newRecord.download = routers[i].download;
            newRecord.collectTime = currentTime;
            newRecord.valid = true;
            m_apLastRecords[routers[i].routerId] = newRecord;
        }
        else
        {
            apLastRecord->upload = routers[i].upload;
            apLastRecord->download = routers[i].download;
            apLastRecord->collectTime = currentTime;
            apLastRecord->valid = true;
        }
    }
    
    // ========== 4. 保存流量趋势数据（每分钟一次） ==========
    // 流量趋势数据入库逻辑：
    // 1. 检查是否到了每分钟入库的时间（精确到分钟）
    // 2. 计算AC级别瞬时速率并入库（只保存AC级别数据，数据库表已移除Fap_id字段）
    // 3. 更新入库时间记录
    
    // 使用已声明的currentTime变量（第460行已声明）
    // 将时间精确到分钟（秒数归零）
    time_t currentMinute = currentTime - (currentTime % 60);
    
    // 检查是否需要入库流量趋势数据（每分钟一次）
    if (m_lastTrafficTrendTime == 0 || currentMinute > m_lastTrafficTrendTime)
    {
        InfoLog("开始保存流量趋势数据（每分钟一次）");
        
        // 生成精确到分钟的时间字符串（格式：YYYY-MM-DD HH:MM:00）
        // 注意：localtime返回静态内存，需要立即使用或复制
        struct tm tm_current;
        struct tm *tm_current_ptr = localtime(&currentTime);
        if (tm_current_ptr != NULL)
        {
            tm_current = *tm_current_ptr;  // 复制结构体，避免后续localtime调用覆盖
        }
        else
        {
            ErrorLog("localtime失败，使用当前时间");
            memset(&tm_current, 0, sizeof(tm_current));
        }
        
        char minuteTimeStr[64];
        snprintf(minuteTimeStr, sizeof(minuteTimeStr), "%04d-%02d-%02d %02d:%02d:%02d",
                 tm_current.tm_year + 1900, tm_current.tm_mon + 1, tm_current.tm_mday,
                 tm_current.tm_hour, tm_current.tm_min, 0);  // 秒数固定为0
        
        // 4.1 保存AC级别流量趋势数据
        // 注意：这里需要使用更新前的m_lastRecord.collectTime（即lastCollectTime）
        // 因为第679行已经将m_lastRecord.collectTime更新为currentTime了
        InfoLog("流量趋势数据写入条件检查: m_lastRecord.valid=%d, lastCollectTime=%lld, m_lastRecord.collectTime=%lld",m_lastRecord.valid ? 1 : 0, (long long)lastCollectTime, (long long)m_lastRecord.collectTime);
        
        // 注意：这里需要使用更新前的m_lastRecord值（即lastCollectTime、lastTotalUpload、lastTotalDownload）
        // 因为第682-684行已经将m_lastRecord更新为本次采集的值了
        if (m_lastRecord.valid && lastCollectTime > 0)
        {
            int64_t timeDiff = currentTime - lastCollectTime;
            
            InfoLog("流量趋势数据计算: currentTime=%lld, lastCollectTime=%lld, timeDiff=%lld秒, lastUpload=%llu KB, lastDownload=%llu KB, currentUpload=%llu KB, currentDownload=%llu KB", 
                   (long long)currentTime, (long long)lastCollectTime, (long long)timeDiff,
                   lastTotalUpload, lastTotalDownload, stats.totalUploadToday, stats.totalDownloadToday);
            
            if (timeDiff > 0)
            {
                // 计算AC级别瞬时速率（KB/s）
                int64_t acUploadSpeed = 0;
                int64_t acDownloadSpeed = 0;
                
                if (stats.totalUploadToday >= lastTotalUpload)
                {
                    acUploadSpeed = (int64_t)((stats.totalUploadToday - lastTotalUpload) / timeDiff);
                }
                if (stats.totalDownloadToday >= lastTotalDownload)
                {
                    acDownloadSpeed = (int64_t)((stats.totalDownloadToday - lastTotalDownload) / timeDiff);
                }
                
                // 保存AC级别流量趋势数据
                CStr2Map acTrendInMap, acTrendOutMap;
                acTrendInMap["collect_time"] = minuteTimeStr;
                acTrendInMap["ac_ip"] = m_acIp;
                ostringstream acUploadOss;
                acUploadOss << acUploadSpeed;
                acTrendInMap["upload_speed"] = acUploadOss.str();
                ostringstream acDownloadOss;
                acDownloadOss << acDownloadSpeed;
                acTrendInMap["download_speed"] = acDownloadOss.str();
                
                if (CIdentRelayApi::InsertAcTrafficTrend(acTrendInMap, acTrendOutMap, false))
                {
                    InfoLog("AC级别流量趋势数据已保存: 上行=%lld KB/s, 下行=%lld KB/s", acUploadSpeed, acDownloadSpeed);
                    
                    // 同时写入Memcached（供查询接口快速读取当天数据）
                    // Key格式：ac_traffic_trend_<ac_ip>_ac_<date>
                    // Value格式：每行一个数据点，格式：time|upload|download，多个数据点用换行符分隔
                    // 确保dateStr是有效的C字符串
                    string dateStr_safe = string(dateStr);  // 安全转换为string
                    string trendKey = "ac_traffic_trend_" + m_acIp + "_ac_" + dateStr_safe;
                    ostringstream trendValueOss;
                    trendValueOss << minuteTimeStr << "|" << acUploadSpeed << "|" << acDownloadSpeed << "\n";
                    string trendValue = trendValueOss.str();
                    
                    memcached_st* pMemcTrend = memcached_create(NULL);
                    if (pMemcTrend != NULL)
                    {
                        if (memcached_server_add(pMemcTrend, m_cacheIp.c_str(), m_cachePort) == MEMCACHED_SUCCESS)
                        {
                            // 尝试追加模式（如果key已存在则追加，不存在则返回NOT_STORED）
                            memcached_return_t rc = memcached_append(pMemcTrend, trendKey.c_str(), trendKey.length(),trendValue.c_str(), trendValue.length(), 86400, 0);
                            if (rc == MEMCACHED_NOTSTORED)
                            {
                                // 如果key不存在（NOT_STORED），使用set创建
                                InfoLog("流量趋势数据key不存在，使用set创建: key=%s", trendKey.c_str());
                                rc = memcached_set(pMemcTrend, trendKey.c_str(), trendKey.length(),trendValue.c_str(), trendValue.length(), 86400, 0);
                            }
                            else if (rc != MEMCACHED_SUCCESS)
                            {
                                // 如果追加失败（其他错误），也尝试使用set
                                ErrorLog("memcached_append失败，尝试使用set: key=%s, error=%s",trendKey.c_str(), memcached_strerror(pMemcTrend, rc));
                                rc = memcached_set(pMemcTrend, trendKey.c_str(), trendKey.length(),trendValue.c_str(), trendValue.length(), 86400, 0);
                            }
                            
                            if (rc == MEMCACHED_SUCCESS || rc == MEMCACHED_STORED)
                            {
                                InfoLog("AC级别流量趋势数据已写入Memcached: key=%s, value_length=%zu",trendKey.c_str(), trendValue.length());
                            }
                            else
                            {
                                ErrorLog("写入AC级别流量趋势数据到Memcached失败: key=%s, error=%s",trendKey.c_str(), memcached_strerror(pMemcTrend, rc));
                            }
                        }
                        else
                        {
                            ErrorLog("连接Memcached失败，无法写入流量趋势数据: server=%s, port=%d", m_cacheIp.c_str(), m_cachePort);
                        }
                        memcached_free(pMemcTrend);
                    }
                }
                else
                {
                    ErrorLog("保存AC级别流量趋势数据失败: %s", acTrendOutMap["errormessage"].c_str());
                }
            }
            else
            {
                InfoLog("时间差<=0，跳过流量趋势数据写入: timeDiff=%lld秒", (long long)timeDiff);
            }
        }
        else
        {
            InfoLog("上一条记录无效，跳过流量趋势数据写入: valid=%d, collectTime=%lld",m_lastRecord.valid ? 1 : 0, (long long)m_lastRecord.collectTime);
        }
        
        // 更新入库时间记录
        m_lastTrafficTrendTime = currentMinute;
        InfoLog("流量趋势数据保存完成（共 %zu 个AP）", routers.size());
    }
    else
    {
        InfoLog("未到流量趋势数据入库时间（上次入库: %lld, 当前分钟: %lld）",m_lastTrafficTrendTime, currentMinute);
    }
    
    InfoLog("历史数据保存完成（共 %zu 个AP）", routers.size());
    
    return 0;
}

// 从AC控制器获取路由器列表（使用已验证的hwWlanIDIndexedApTable）
// 优先使用：1.3.6.1.4.1.2011.6.139.13.3.10.1 (hwWlanIDIndexedApTable)
// 索引：简单整数（0, 1, 2, 3）
vector<RouterInfo> CAcDataCollector::GetRouterListFromAC(CSnmpClient& acClient)
{
    vector<RouterInfo> routers;
    map<string, string> apList;
    
    // 优先使用已验证的hwWlanIDIndexedApTable
    // OID: 1.3.6.1.4.1.2011.6.139.13.3.10.1
    // 索引：简单整数（0, 1, 2, 3）
    vector<string> tryOids;
    tryOids.push_back("1.3.6.1.4.1.2011.6.139.13.3.10.1.5");   // hwWlanIDIndexedApName (AP名称)
    tryOids.push_back("1.3.6.1.4.1.2011.6.139.13.3.10.1.14");  // hwWlanIDIndexedApIpAddress (AP IP)
    tryOids.push_back("1.3.6.1.4.1.2011.6.139.13.3.10.1.7");   // hwWlanIDIndexedApRunState (AP状态)
    tryOids.push_back("1.3.6.1.4.1.2011.6.139.13.3.10.1.2");   // hwWlanIDIndexedApMac (AP MAC地址)
    
    // 备选：尝试其他OID（如果hwWlanIDIndexedApTable不可用）
    tryOids.push_back("1.2.156.11235.6001.60.7.2.75.2.1.1.1.5");  // AP名称表
    tryOids.push_back("1.2.156.11235.6001.60.7.2.75.2.1.1.1.2");  // AP IP地址表
    tryOids.push_back("1.2.156.11235.6001.60.7.2.75.2.1.1.1.4");  // AP状态表
    tryOids.push_back("1.3.6.1.4.1.2011.6.139.14.3.3.1.4");  // AP状态表
    
    bool found = false;
    for (size_t i = 0; i < tryOids.size() && !found; ++i)
    {
        apList.clear();
        InfoLog("尝试Walk OID: %s", tryOids[i].c_str());
        
        if (acClient.Walk(tryOids[i], apList) == 0 && !apList.empty())
        {
            InfoLog("成功Walk OID %s，获取到 %zu 条记录", tryOids[i].c_str(), apList.size());
            
            // 跳过VLAN配置OID
            if (tryOids[i].find("1.3.6.1.4.1.2011.6.8.1.1.1") != string::npos)
            {
                InfoLog("OID %s 返回的是VLAN配置，跳过", tryOids[i].c_str());
                continue;
            }
            
            // 判断当前OID是哪个字段
            // hwWlanIDIndexedApTable字段：
            // .1.5 = hwWlanIDIndexedApName (AP名称)
            // .1.14 = hwWlanIDIndexedApIpAddress (AP IP)
            // .1.7 = hwWlanIDIndexedApRunState (AP状态，normal(8)=在线)
            // .1.2 = hwWlanIDIndexedApMac (AP MAC地址)
            bool isHwWlanIDIndexedTable = (tryOids[i].find("1.3.6.1.4.1.2011.6.139.13.3.10.1") != string::npos);
            bool isNameField = (tryOids[i].find(".1.5") != string::npos || 
                               tryOids[i] == "1.2.156.11235.6001.60.7.2.75.2.1.1.1.5" ||
                               tryOids[i].find("1.3.6.1.4.1.2011.6.139.1.1.1.1.3") != string::npos ||
                               tryOids[i].find("1.3.6.1.4.1.2011.6.139.14.3.3.1.8") != string::npos ||
                               (isHwWlanIDIndexedTable && tryOids[i].find(".1.5") != string::npos));
            bool isIpField = (tryOids[i].find(".1.14") != string::npos ||
                             tryOids[i].find(".1.2") != string::npos || 
                             tryOids[i] == "1.2.156.11235.6001.60.7.2.75.2.1.1.1.2" ||
                             tryOids[i].find("1.3.6.1.4.1.2011.6.139.1.1.1.1.4") != string::npos ||
                             (isHwWlanIDIndexedTable && tryOids[i].find(".1.14") != string::npos));
            bool isStateField = (tryOids[i].find(".1.7") != string::npos ||
                                tryOids[i].find(".1.4") != string::npos || 
                                tryOids[i] == "1.2.156.11235.6001.60.7.2.75.2.1.1.1.4" ||
                                tryOids[i].find("1.3.6.1.4.1.2011.6.139.1.1.1.1.5") != string::npos ||
                                tryOids[i].find("1.3.6.1.4.1.2011.6.139.14.3.3.1.4") != string::npos ||
                                (isHwWlanIDIndexedTable && tryOids[i].find(".1.7") != string::npos));
            bool isMacField = (tryOids[i].find(".1.2") != string::npos && 
                               isHwWlanIDIndexedTable && tryOids[i].find(".1.14") == string::npos);
            bool isApIdField = (tryOids[i].find(".1.1") != string::npos || 
                               tryOids[i] == "1.2.156.11235.6001.60.7.2.75.2.1.1.1.1");
            
            for (map<string, string>::iterator it = apList.begin(); it != apList.end(); ++it)
            {
                string oid = it->first;
                string value = it->second;
                
                // 从OID中提取AP索引
                string apIndex;
                string baseOid = tryOids[i];
                size_t basePos = oid.find(baseOid);
                if (basePos != string::npos)
                {
                    size_t indexStart = basePos + baseOid.length();
                    if (indexStart < oid.length() && oid[indexStart] == '.')
                    {
                        indexStart++;
                    }
                    apIndex = oid.substr(indexStart);
                }
                else
                {
                    size_t lastDot = oid.find_last_of('.');
                    if (lastDot == string::npos) continue;
                    apIndex = oid.substr(lastDot + 1);
                }
                
                // 对于hwWlanIDIndexedApTable，索引0是有效的（第一个AP）
                // 对于其他表，索引0可能是特殊值，跳过
                if (apIndex.empty()) continue;
                if (!isHwWlanIDIndexedTable && apIndex == "0") continue;
                
                // 检查是否已经存在这个索引的AP
                bool exists = false;
                for (size_t j = 0; j < routers.size(); ++j)
                {
                    if (routers[j].routerId == apIndex)
                    {
                        exists = true;
                        if (isNameField) routers[j].name = value;
                        else if (isIpField) routers[j].ip = value;
                        else if (isStateField)
                        {
                            int state = atoi(value.c_str());
                            if (isHwWlanIDIndexedTable && tryOids[i].find(".1.7") != string::npos)
                            {
                                // hwWlanIDIndexedApRunState: normal(8)=在线
                                if (state == 8)  // normal状态表示在线
                                {
                                    routers[j].status = "online";
                                }
                                else
                                {
                                    routers[j].status = "offline";
                                }
                            }
                            else if (tryOids[i].find("1.3.6.1.4.1.2011.6.139.14.3.3.1.4") != string::npos)
                            {
                                routers[j].status = (state == 1) ? "online" : "offline";
                            }
                            else
                            {
                                if (state == 5) routers[j].status = "online";
                                else if (state >= 1 && state <= 4) routers[j].status = "offline";
                                else routers[j].status = "fault";
                            }
                        }
                        else if (isMacField)
                        {
                            // MAC地址字段（Hex格式）
                            routers[j].mac = value;
                            routers[j].macOid = ConvertMacToOidIndex(value);
                        }
                        else if (isApIdField && routers[j].routerId.empty())
                        {
                            routers[j].routerId = value;
                        }
                        break;
                    }
                }
                
                if (!exists)
                {
                    RouterInfo router;
                    router.routerId = apIndex;
                    
                    if (isNameField) router.name = value;
                    else if (isIpField) { router.ip = value; router.name = "AP-" + apIndex; }
                    else if (isStateField)
                    {
                        int state = atoi(value.c_str());
                        if (isHwWlanIDIndexedTable && tryOids[i].find(".1.7") != string::npos)
                        {
                            // hwWlanIDIndexedApRunState: normal(8)=在线
                            if (state == 8)  // normal状态表示在线
                            {
                                router.status = "online";
                            }
                            else
                            {
                                router.status = "offline";
                            }
                        }
                        else if (tryOids[i].find("1.3.6.1.4.1.2011.6.139.14.3.3.1.4") != string::npos)
                        {
                            router.status = (state == 1) ? "online" : "offline";
                        }
                        else
                        {
                            if (state == 5) router.status = "online";
                            else if (state >= 1 && state <= 4) router.status = "offline";
                            else router.status = "fault";
                        }
                        router.name = "AP-" + apIndex;
                    }
                    else if (isMacField)
                    {
                        // MAC地址字段（Hex格式）
                        router.mac = value;
                        router.macOid = ConvertMacToOidIndex(value);
                        router.name = "AP-" + apIndex;
                    }
                    else if (isApIdField) { router.routerId = value; router.name = "AP-" + value; }
                    else router.name = "AP-" + apIndex;
                    
                    routers.push_back(router);
                }
            }
            
            // 如果找到了一些AP，检查是否应该停止
            // 优先使用hwWlanIDIndexedApTable，因为它能查询到所有AP
            // 如果使用标准OID只找到部分AP（少于4台），继续尝试其他OID
            if (!routers.empty())
            {
                bool isHwWlanIDIndexedTable = (tryOids[i].find("1.3.6.1.4.1.2011.6.139.13.3.10.1") != string::npos);
                
                if (isHwWlanIDIndexedTable)
                {
                    // hwWlanIDIndexedApTable可以查询到所有AP，使用这个结果
                    found = true;
                    InfoLog("通过OID %s (hwWlanIDIndexedApTable) 找到 %zu 个AP", tryOids[i].c_str(), routers.size());
                }
                else if (routers.size() >= 4)
                {
                    // 标准OID或其他OID找到了4台或更多AP，使用这个结果
                    found = true;
                    InfoLog("通过OID %s 找到 %zu 个AP", tryOids[i].c_str(), routers.size());
                }
                else
                {
                    // 标准OID只找到部分AP（少于4台），继续尝试其他OID
                    InfoLog("通过OID %s 只找到 %zu 个AP（少于4台），继续尝试其他OID", tryOids[i].c_str(), routers.size());
                }
            }
        }
    }
    
    return routers;
}

// 从AC控制器获取指定路由器的详细信息（使用已验证的OID）
// 实时数据：CPU、内存、在线用户数、实时速率 -> 写入Memcached
// 非实时数据：累计流量、运行时间、上线时长 -> 写入MySQL
int CAcDataCollector::GetRouterInfoFromAC(CSnmpClient& acClient, RouterInfo& router)
{
    InfoLog("GetRouterInfoFromAC: 开始处理AP ID=%s, Name=%s", router.routerId.c_str(), router.name.c_str());
    
    // ========== 1. 获取AP基本信息（使用hwWlanIDIndexedApTable） ==========
    
    // 获取AP IP地址
    if (router.ip.empty() || router.ip == "255.255.255.255")
    {
        string ipOid = "1.3.6.1.4.1.2011.6.139.13.3.10.1.14." + router.routerId;  // hwWlanIDIndexedApIpAddress
        string ipValue;
        if (acClient.Get(ipOid, ipValue) == 0 && ipValue != "0.0.0.0" && ipValue != "255.255.255.255")
        {
            router.ip = ipValue;
            InfoLog("从AC获取到AP[%s]的IP地址: %s", router.name.c_str(), router.ip.c_str());
        }
    }
    
    // 获取AP状态
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
                router.status = "offline";  // 其他状态：idle(1), autofind(2), typeNotMatch(3), config(5), download(7)等
            }
            InfoLog("从AC获取到AP[%s]的状态: %s (state=%lld)", router.name.c_str(), router.status.c_str(), stateValue);
        }
    }
    
    // 获取AP MAC地址（如果还没有）
    if (router.mac.empty())
    {
        string macOid = "1.3.6.1.4.1.2011.6.139.13.3.10.1.2." + router.routerId;  // hwWlanIDIndexedApMac
        string macValue;
        if (acClient.Get(macOid, macValue) == 0 && !macValue.empty())
        {
            router.mac = macValue;
            router.macOid = ConvertMacToOidIndex(macValue);
            InfoLog("从AC获取到AP[%s]的MAC地址: %s (OID格式: %s)",router.name.c_str(), router.mac.c_str(), router.macOid.c_str());
        }
    }
    
    // ========== 2. 获取实时数据（写入Memcached） ==========
    
    // 获取CPU使用率
    if (router.cpu == 0)
    {
        string cpuOid = "1.3.6.1.4.1.2011.6.139.13.3.10.1.42." + router.routerId;  // hwWlanIDIndexedApCpuUseRate
        int64_t cpuValue = 0;
        if (acClient.GetInt(cpuOid, cpuValue) == 0 && cpuValue >= 0 && cpuValue <= 100)
        {
            router.cpu = (int)cpuValue;
            InfoLog("从AC获取到AP[%s]的CPU使用率: %d%%", router.name.c_str(), router.cpu);
        }
    }
    
    // 获取内存使用率
    if (router.memory == 0)
    {
        string memoryOid = "1.3.6.1.4.1.2011.6.139.13.3.10.1.41." + router.routerId;  // hwWlanIDIndexedApMemoryUseRate
        int64_t memoryValue = 0;
        if (acClient.GetInt(memoryOid, memoryValue) == 0 && memoryValue >= 0 && memoryValue <= 100)
        {
            router.memory = (int)memoryValue;
            InfoLog("从AC获取到AP[%s]的内存使用率: %d%%", router.name.c_str(), router.memory);
        }
    }
    
    // 获取在线用户数
    string usersOid = "1.3.6.1.4.1.2011.6.139.13.3.10.1.45." + router.routerId;  // hwWlanIDIndexedApOnlineUserNum
    int64_t usersValue = 0;
    if (acClient.GetInt(usersOid, usersValue) == 0)
    {
        router.onlineUsers = (int)usersValue;
        InfoLog("从AC获取到AP[%s]的在线用户数: %d", router.name.c_str(), router.onlineUsers);
    }
    
    // 获取上行实时速率（Mbps）
    if (!router.macOid.empty())
    {
        string uploadSpeedOid = "1.3.6.1.4.1.2011.6.139.13.3.3.1.54." + router.macOid;  // hwWlanApUpPortSpeed
        int64_t uploadSpeedValue = 0;
        if (acClient.GetInt(uploadSpeedOid, uploadSpeedValue) == 0)
        {
            router.uploadSpeed = (int)uploadSpeedValue;
            InfoLog("从AC获取到AP[%s]的上行实时速率: %d Mbps", router.name.c_str(), router.uploadSpeed);
        }
    }
    
    // 获取运行时间（单位：秒）
    // 注意：hwWlanIDIndexedApRunTime返回Gauge32类型，单位是秒（1 tick = 1秒），不是centiseconds
    // 与sysUpTime不同，sysUpTime使用Timeticks（centiseconds），但hwWlanIDIndexedApRunTime使用秒
    string runTimeOid = "1.3.6.1.4.1.2011.6.139.13.3.10.1.19." + router.routerId;  // hwWlanIDIndexedApRunTime
    int64_t runTimeInt = 0;
    if (acClient.GetInt(runTimeOid, runTimeInt) == 0 && runTimeInt >= 0)
    {
        router.runTime = (uint32_t)runTimeInt;
        uint32_t hours = router.runTime / 3600;
        uint32_t minutes = (router.runTime % 3600) / 60;
        uint32_t seconds = router.runTime % 60;
        InfoLog("从AC获取到AP[%s]的运行时间: %u 秒 (%u小时 %u分钟 %u秒)",router.name.c_str(), router.runTime, hours, minutes, seconds);
    }
    else
    {
        ErrorLog("从AC获取AP[%s]的运行时间失败: OID=%s, error=%s",router.name.c_str(), runTimeOid.c_str(), acClient.GetLastError().c_str());
    }
    
    // 获取温度（可选）
    string tempOid = "1.3.6.1.4.1.2011.6.139.13.3.10.1.44." + router.routerId;  // hwWlanIDIndexedApTemperature
    int64_t tempValue = 0;
    if (acClient.GetInt(tempOid, tempValue) == 0)
    {
        router.temperature = (int)tempValue;  // 255表示不支持，其他值表示实际温度
        if (tempValue != 255)
        {
            InfoLog("从AC获取到AP[%s]的温度: %d°C", router.name.c_str(), router.temperature);
        }
    }
    
    // 获取下行实时速率（Mbps）
    if (!router.macOid.empty())
    {
        string downloadSpeedOid = "1.3.6.1.4.1.2011.6.139.13.3.3.1.57." + router.macOid;  // hwWlanEthportDownRate
        int64_t downloadSpeedValue = 0;
        if (acClient.GetInt(downloadSpeedOid, downloadSpeedValue) == 0)
        {
            router.downloadSpeed = (int)downloadSpeedValue;
            InfoLog("从AC获取到AP[%s]的下行实时速率: %d Mbps", router.name.c_str(), router.downloadSpeed);
        }
        else
        {
            ErrorLog("从AC获取AP[%s]的下行实时速率失败: OID=%s, error=%s", 
                     router.name.c_str(), downloadSpeedOid.c_str(), acClient.GetLastError().c_str());
        }
    }
    
    // ========== 3. 获取非实时数据（写入MySQL） ==========
    
    // 获取无线上行累计流量（Bytes）
    if (!router.macOid.empty())
    {
        string uploadTrafficOid = "1.3.6.1.4.1.2011.6.139.13.3.3.1.58." + router.macOid;  // hwWlanIDIndexedApAirportUpTraffic
        string uploadTrafficValue;
        if (acClient.Get(uploadTrafficOid, uploadTrafficValue) == 0 && !uploadTrafficValue.empty())
        {
            // 移除引号
            if (uploadTrafficValue[0] == '"' && uploadTrafficValue[uploadTrafficValue.length()-1] == '"')
            {
                uploadTrafficValue = uploadTrafficValue.substr(1, uploadTrafficValue.length()-2);
            }
            uint64_t uploadBytes = 0;
            sscanf(uploadTrafficValue.c_str(), "%llu", &uploadBytes);
            router.upload = uploadBytes / 1024;  // 转换为KB
            InfoLog("从AC获取到AP[%s]的上行累计流量: %llu KB (%s Bytes)",router.name.c_str(), router.upload, uploadTrafficValue.c_str());
        }
    }
    
    // 获取无线下行累计流量（Bytes）
    if (!router.macOid.empty())
    {
        string downloadTrafficOid = "1.3.6.1.4.1.2011.6.139.13.3.3.1.59." + router.macOid;  // hwWlanIDIndexedApAirportDwTraffic
        string downloadTrafficValue;
        if (acClient.Get(downloadTrafficOid, downloadTrafficValue) == 0 && !downloadTrafficValue.empty())
        {
            // 移除引号
            if (downloadTrafficValue[0] == '"' && downloadTrafficValue[downloadTrafficValue.length()-1] == '"')
            {
                downloadTrafficValue = downloadTrafficValue.substr(1, downloadTrafficValue.length()-2);
            }
            uint64_t downloadBytes = 0;
            sscanf(downloadTrafficValue.c_str(), "%llu", &downloadBytes);
            router.download = downloadBytes / 1024;  // 转换为KB
            InfoLog("从AC获取到AP[%s]的下行累计流量: %llu KB (%s Bytes)",router.name.c_str(), router.download, downloadTrafficValue.c_str());
        }
    }
    
    // 获取上线时长（单位：秒）
    // 注意：hwWlanIDIndexedApOnlineTime返回Gauge32类型，单位是秒（1 tick = 1秒），不是centiseconds
    string onlineTimeOid = "1.3.6.1.4.1.2011.6.139.13.3.10.1.22." + router.routerId;  // hwWlanIDIndexedApOnlineTime
    int64_t onlineTimeInt = 0;
    if (acClient.GetInt(onlineTimeOid, onlineTimeInt) == 0 && onlineTimeInt >= 0)
    {
        router.onlineTime = (uint32_t)onlineTimeInt;
        uint32_t hours = router.onlineTime / 3600;
        uint32_t minutes = (router.onlineTime % 3600) / 60;
        uint32_t seconds = router.onlineTime % 60;
        InfoLog("从AC获取到AP[%s]的上线时长: %u 秒 (%u小时 %u分钟 %u秒)",router.name.c_str(), router.onlineTime, hours, minutes, seconds);
    }
    else
    {
        ErrorLog("从AC获取AP[%s]的上线时长失败: OID=%s, error=%s", 
                 router.name.c_str(), onlineTimeOid.c_str(), acClient.GetLastError().c_str());
    }
    
    InfoLog("GetRouterInfoFromAC: 完成处理AP ID=%s, 状态=%s, CPU=%d%%, 内存=%d%%, 用户=%d, 上行=%lluKB, 下行=%lluKB, 上行速率=%dMbps, 下行速率=%dMbps", 
             router.routerId.c_str(), router.status.c_str(), router.cpu, router.memory, 
             router.onlineUsers, router.upload, router.download, router.uploadSpeed, router.downloadSpeed);
    return 0;
}

