/************************************************************
 Desc:     AC控制器数据采集类 - 采集AC控制器和AP设备数据并存储
 Auth:     Auto
 Modify:
 data:     2025-01-29
 ***********************************************************/
#ifndef CACDATACOLLECTOR_H_
#define CACDATACOLLECTOR_H_

#include "CIdentAppComm.h"
#include "CIdentComm.h"
#include "CSnmpClient.h"
#include "transxmlcfg.h"
#include <vector>
#include <string>

using namespace std;

// 路由器信息结构（从AC控制器获取）
struct RouterInfo
{
    string routerId;      // 路由器ID/索引（AP ID，如0,1,2,3）
    string name;          // 路由器名称
    string ip;            // 路由器IP地址
    string mac;           // MAC地址（Hex格式，如"60 53 75 2D B8 80"）
    string macOid;        // MAC地址（OID格式，如"96.83.117.45.184.128"）
    string status;        // 状态：online/offline/fault
    int cpu;              // CPU使用率（%）
    int memory;           // 内存使用率（%）
    int onlineUsers;      // 在线用户数
    uint64_t upload;      // 上行累计流量（KB）
    uint64_t download;    // 下行累计流量（KB）
    int uploadSpeed;      // 上行实时速率（Mbps）
    int downloadSpeed;    // 下行实时速率（Mbps）
    uint32_t runTime;     // 运行时间（tick）
    uint32_t onlineTime;  // 上线时长（tick）
    int temperature;      // 温度（℃），255表示不支持
    
    RouterInfo() : cpu(0), memory(0), onlineUsers(0), upload(0), download(0), 
                   uploadSpeed(0), downloadSpeed(0), runTime(0), onlineTime(0), temperature(0) {}
};

// AC统计数据
struct AcStatsData
{
    string date;                    // 日期
    int totalRouters;               // AP总数
    int onlineRouters;              // 在线路由器数量
    int offlineRouters;              // 离线路由器数量
    int faultRouters;                // 故障路由器数量
    int avgCpu;                     // 平均CPU使用率（%）
    int avgMemory;                  // 平均内存使用率（%）
    uint64_t totalUploadToday;      // 今日上行流量（KB）
    uint64_t totalDownloadToday;    // 今日下行流量（KB）
    uint64_t totalUploadMonth;      // 本月上行流量（KB）
    uint64_t totalDownloadMonth;    // 本月下行流量（KB）
    int64_t totalUsers;             // 总用户数
    int64_t stationAssocSum;         // 关联成功次数
    int64_t stationAssocFailSum;    // 关联失败次数
    int64_t stationReassocSum;      // 重关联请求数
    int64_t stationAssocRejectedSum; // 拒绝关联次数
    int64_t stationExDeAuthenSum;   // 异常去认证次数
    
    AcStatsData() : totalRouters(0), onlineRouters(0), offlineRouters(0), 
                    faultRouters(0), avgCpu(0), avgMemory(0),
                    totalUploadToday(0), totalDownloadToday(0),
                    totalUploadMonth(0), totalDownloadMonth(0),
                    totalUsers(0), stationAssocSum(0), stationAssocFailSum(0),
                    stationReassocSum(0), stationAssocRejectedSum(0),
                    stationExDeAuthenSum(0) {}
};

class CAcDataCollector
{
public:
    CAcDataCollector();
    virtual ~CAcDataCollector();
    
    // 初始化
    int Init();
    
    // 清理资源
    void Cleanup();
    
    // 停止采集（设置标志）
    void Stop() { m_stopped = true; }
    
    // 执行数据采集
    int CollectData(CStr2Map &inMap, CStr2Map &outMap);
    
    string GetTid() {
        return string("app_AcDataCollector");
    };

private:
    // 从AC控制器获取路由器列表
    vector<RouterInfo> GetRouterListFromAC(CSnmpClient& acClient);
    
    // 从AC控制器获取指定路由器的详细信息
    int GetRouterInfoFromAC(CSnmpClient& acClient, RouterInfo& router);
    
    // 存储实时数据到Memcached（供CGI程序快速读取）
    // 存储内容：
    // 1. 统计数据（ac_stats_<ac_ip>）：实时统计信息
    // 2. AP列表数据（ac_ap_list_<ac_ip>）：AP设备列表（包含实时速率）
    int SaveToMemcached(const AcStatsData& stats, const vector<RouterInfo>& routers);
    
    // 存储历史数据到MySQL数据库（非实时数据，带备注说明）
    // 存储内容：
    // 1. 统计数据历史记录 -> t_ac_controller_stats 表
    // 2. AP设备详细信息历史记录 -> t_ac_ap_info 表（累计流量、运行时间等）
    // 实现方式：通过CIdentRelayApi调用relay服务执行数据库操作
    // 备注：在Fremark字段中说明数据来源、缺失字段等信息
    int SaveToMySQL(const AcStatsData& stats, const vector<RouterInfo>& routers);
    
    // MAC地址转换：Hex格式 -> OID格式
    // 输入：Hex格式 "60 53 75 2D B8 80"
    // 输出：OID格式 "96.83.117.45.184.128"
    static string ConvertMacToOidIndex(const string& macHex);
    
    // 计算统计数据
    void CalculateStats(const vector<RouterInfo>& routers, 
                       int64_t apCount, int64_t userCount,
                       int64_t stationAssocSum, int64_t stationAssocFailSum,
                       int64_t stationReassocSum, int64_t stationAssocRejectedSum,
                       int64_t stationExDeAuthenSum,
                       AcStatsData& stats);
    
    bool m_stopped;              // 停止标志
    string m_acIp;               // AC控制器IP
    string m_acCommunity;        // AC控制器community
    int m_acPort;                // AC控制器端口
    
    // Memcached缓存配置
    string m_cacheIp;            // 缓存服务器IP
    int m_cachePort;             // 缓存服务器端口
    
    // 上一条记录缓存（用于计算瞬时速率）
    struct LastRecord {
        uint64_t totalUpload;      // 上一条累计上行流量（KB）
        uint64_t totalDownload;    // 上一条累计下行流量（KB）
        time_t collectTime;        // 上一条采集时间
        bool valid;                // 是否有效
        LastRecord() : totalUpload(0), totalDownload(0), collectTime(0), valid(false) {}
    };
    LastRecord m_lastRecord;     // 缓存上一条记录（AC控制器级别）
    
    // AP上一条记录缓存（用于计算AP今日流量）
    struct ApLastRecord {
        uint64_t upload;          // 上一条累计上行流量（KB）
        uint64_t download;         // 上一条累计下行流量（KB）
        time_t collectTime;        // 上一条采集时间
        bool valid;                 // 是否有效
        ApLastRecord() : upload(0), download(0), collectTime(0), valid(false) {}
    };
    map<string, ApLastRecord> m_apLastRecords;  // 缓存每个AP的上一条记录，key=ap_id
    
    // 流量趋势数据入库时间记录（用于每分钟入库一次）
    time_t m_lastTrafficTrendTime;  // 上次入库流量趋势数据的时间（精确到分钟）
};

#endif

