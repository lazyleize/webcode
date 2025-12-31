#ifndef CPRODUCERECORDDATA_H
#define CPRODUCERECORDDATA_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "tools/commdef.h"  // 包含CStr2Map的定义（typedef std::map<std::string,std::string>）

/**
 * @brief 单条生产记录数据结构
 */
struct ProduceRecordItem
{
    // 基础信息
    std::string id;              // 记录ID (Fid)
    std::string app_uid;         // 应用UID (Fapp_uid)
    std::string pos_sn;          // 设备SN号 (Fpos_sn)
    std::string order;           // 订单号 (Forder)
    std::string factory_id;      // 工厂ID (Factory)
    std::string pos_name;        // 机型名称 (Fpos_name)
    
    // 结果信息
    int result;                  // 结果: 0-成功, 1-失败 (Fresult)
    std::string message;         // 消息/错误信息 (Fmessage)
    std::string create_time;     // 创建时间 (Fcreate_time)
    
    // 日志信息
    int log_type;                // 日志类型: 0-下防切, 1-SMT, 2-组装, 3-包装 (Flog_type)
    std::string log_id;          // 日志ID (Flog_id)
    
    // PID信息
    std::string ass_pid;         // 组装PID (Fass_pid)
    std::string pack_pid;        // 打包PID (Fpack_pid)
    
    // 操作信息
    std::string operator_name;    // 操作员 (Foperator)
    
    // 设备详情（可选，从详情查询获取）
    std::string cpu_id;          // CPU ID
    std::string flash_id;        // Flash ID
    std::string wifi_macid;      // WiFi MAC地址
    std::string blue_macid;      // 蓝牙MAC地址
    std::string eth_macid;       // 以太网MAC地址
    std::string imei;            // IMEI
    
    ProduceRecordItem() : result(0), log_type(0) {}
};

/**
 * @brief 机型统计信息
 */
struct PosNameStatistics
{
    std::string pos_name;        // 机型名称
    int total_count;             // 总记录数
    int success_count;           // 成功记录数
    int fail_count;              // 失败记录数
    double success_rate;         // 成功率 (0.0-1.0)
    
    // 按日志类型统计
    std::map<int, int> log_type_count;  // 各日志类型的记录数
    
    // 按工厂统计
    std::map<std::string, int> factory_count;  // 各工厂的记录数
    
    // 时间范围
    std::string time_beg;        // 最早记录时间
    std::string time_end;        // 最晚记录时间
    
    PosNameStatistics() : total_count(0), success_count(0), fail_count(0), success_rate(0.0) {}
};

/**
 * @brief 生产记录数据集合
 * 用于存储和组织生产记录，支持按机型、时间等维度查询和统计
 */
class CProduceRecordData
{
public:
    CProduceRecordData();
    ~CProduceRecordData();
    
    /**
     * @brief 从CStr2Map添加一条生产记录
     * @param recordMap 从数据库查询返回的记录Map
     */
    void AddRecord(const CStr2Map& recordMap);
    
    /**
     * @brief 添加一条生产记录
     * @param record 生产记录对象
     */
    void AddRecord(const ProduceRecordItem& record);
    
    /**
     * @brief 获取指定机型的所有记录
     * @param pos_name 机型名称
     * @return 该机型的所有记录列表
     */
    std::vector<ProduceRecordItem> GetRecordsByPosName(const std::string& pos_name) const;
    
    /**
     * @brief 获取指定时间范围内的记录
     * @param time_beg 开始时间 (格式: YYYY-MM-DD 或 YYYY-MM-DD HH:MM:SS)
     * @param time_end 结束时间
     * @return 符合条件的记录列表
     */
    std::vector<ProduceRecordItem> GetRecordsByTimeRange(const std::string& time_beg, 
                                                          const std::string& time_end) const;
    
    /**
     * @brief 获取指定机型的统计信息
     * @param pos_name 机型名称
     * @return 统计信息
     */
    PosNameStatistics GetStatisticsByPosName(const std::string& pos_name) const;
    
    /**
     * @brief 获取所有机型的统计信息
     * @return map<机型名称, 统计信息>
     */
    std::map<std::string, PosNameStatistics> GetAllStatistics() const;
    
    /**
     * @brief 获取所有机型名称列表
     * @return 机型名称列表
     */
    std::vector<std::string> GetAllPosNames() const;
    
    /**
     * @brief 获取所有记录
     * @return 所有记录列表
     */
    std::vector<ProduceRecordItem> GetAllRecords() const;
    
    /**
     * @brief 获取记录总数
     * @return 记录总数
     */
    size_t GetTotalCount() const;
    
    /**
     * @brief 查找同类型机型（基于机型名称相似度或配置）
     * @param pos_name 目标机型名称
     * @param similarity_threshold 相似度阈值（可选，用于未来扩展）
     * @return 同类型机型名称列表
     */
    std::vector<std::string> FindSimilarPosNames(const std::string& pos_name, 
                                                  double similarity_threshold = 0.0) const;
    
    /**
     * @brief 转换为JSON格式（用于传递给Python脚本）
     * @return JSON字符串
     */
    std::string ToJson() const;
    
    /**
     * @brief 从JSON格式加载数据
     * @param json_str JSON字符串
     * @return 是否成功
     */
    bool FromJson(const std::string& json_str);
    
    /**
     * @brief 清空所有数据
     */
    void Clear();
    
    /**
     * @brief 获取指定机型的SN列表（用于ES查询）
     * @param pos_name 机型名称
     * @return SN列表
     */
    std::vector<std::string> GetSNListByPosName(const std::string& pos_name) const;
    
    /**
     * @brief 获取指定机型的PID列表（用于ES查询）
     * @param pos_name 机型名称
     * @return PID列表（ass_pid和pack_pid）
     */
    std::vector<std::string> GetPIDListByPosName(const std::string& pos_name) const;
    
    /**
     * @brief 按工厂分组获取记录
     * @return map<工厂ID, 记录列表>
     */
    std::map<std::string, std::vector<ProduceRecordItem>> GetRecordsByFactory() const;
    
    /**
     * @brief 按日志类型分组获取记录
     * @return map<日志类型, 记录列表>
     */
    std::map<int, std::vector<ProduceRecordItem>> GetRecordsByLogType() const;

private:
    // 所有记录（按ID索引，便于快速查找）
    std::map<std::string, ProduceRecordItem> m_allRecords;
    
    // 按机型索引的记录
    std::map<std::string, std::vector<std::string>> m_recordsByPosName;
    
    // 按工厂索引的记录
    std::map<std::string, std::vector<std::string>> m_recordsByFactory;
    
    // 按日志类型索引的记录
    std::map<int, std::vector<std::string>> m_recordsByLogType;
    
    // 按时间索引的记录（用于时间范围查询优化）
    std::vector<std::string> m_recordIdsByTime;  // 按时间排序的记录ID列表
    
    /**
     * @brief 更新统计信息（内部方法）
     */
    void UpdateStatistics(const ProduceRecordItem& record);
    
    /**
     * @brief 比较时间字符串
     * @return true if time1 < time2
     */
    bool CompareTime(const std::string& time1, const std::string& time2) const;
    
    /**
     * @brief 从CStr2Map转换为ProduceRecordItem
     */
    ProduceRecordItem ConvertFromMap(const CStr2Map& recordMap) const;
};

#endif // CPRODUCERECORDDATA_H

