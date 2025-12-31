#include "CGetTermProduceReport.h"
#include "CIdentRelayApi.h"
#include "CProduceRecordData.h"
#include "curl/curl.h"
#include "xml/unicode.h"

#include <base/datetime.hpp>
#include <base/directory.hpp>
#include <base/file.hpp>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <iomanip>

// 取消宏定义，避免冲突
#undef max
#undef min
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;

using namespace aps;

// ============================================================================
// CProduceRecordData 实现（合并到本文件中）
// ============================================================================

CProduceRecordData::CProduceRecordData()
{
}

CProduceRecordData::~CProduceRecordData()
{
    Clear();
}

void CProduceRecordData::AddRecord(const CStr2Map& recordMap)
{
    ProduceRecordItem record = ConvertFromMap(recordMap);
    AddRecord(record);
}

void CProduceRecordData::AddRecord(const ProduceRecordItem& record)
{
    // 如果记录ID为空，生成一个唯一ID
    std::string recordId = record.id;
    if (recordId.empty())
    {
        std::stringstream ss;
        ss << record.pos_sn << "_" << record.create_time << "_" << record.log_type;
        recordId = ss.str();
    }
    
    // 检查是否已存在
    if (m_allRecords.find(recordId) != m_allRecords.end())
    {
        return; // 已存在，不重复添加
    }
    
    // 添加到主记录表
    ProduceRecordItem newRecord = record;
    newRecord.id = recordId;
    m_allRecords[recordId] = newRecord;
    
    // 更新索引
    m_recordsByPosName[newRecord.pos_name].push_back(recordId);
    m_recordsByFactory[newRecord.factory_id].push_back(recordId);
    m_recordsByLogType[newRecord.log_type].push_back(recordId);
    
    // 更新时间索引（插入排序，保持时间顺序）
    auto it = std::lower_bound(m_recordIdsByTime.begin(), m_recordIdsByTime.end(), 
                                newRecord.create_time,
                                [this](const std::string& id, const std::string& time) {
                                    return CompareTime(m_allRecords[id].create_time, time);
                                });
    m_recordIdsByTime.insert(it, recordId);
    
    // 更新统计信息
    UpdateStatistics(newRecord);
}

std::vector<ProduceRecordItem> CProduceRecordData::GetRecordsByPosName(const std::string& pos_name) const
{
    std::vector<ProduceRecordItem> result;
    
    auto it = m_recordsByPosName.find(pos_name);
    if (it != m_recordsByPosName.end())
    {
        for (const auto& recordId : it->second)
        {
            auto recordIt = m_allRecords.find(recordId);
            if (recordIt != m_allRecords.end())
            {
                result.push_back(recordIt->second);
            }
        }
    }
    
    return result;
}

std::vector<ProduceRecordItem> CProduceRecordData::GetRecordsByTimeRange(const std::string& time_beg, 
                                                                          const std::string& time_end) const
{
    std::vector<ProduceRecordItem> result;
    
    for (const auto& recordId : m_recordIdsByTime)
    {
        auto it = m_allRecords.find(recordId);
        if (it != m_allRecords.end())
        {
            const ProduceRecordItem& record = it->second;
            if (!CompareTime(record.create_time, time_beg) && 
                CompareTime(record.create_time, time_end))
            {
                result.push_back(record);
            }
        }
    }
    
    return result;
}

PosNameStatistics CProduceRecordData::GetStatisticsByPosName(const std::string& pos_name) const
{
    PosNameStatistics stats;
    stats.pos_name = pos_name;
    
    auto records = GetRecordsByPosName(pos_name);
    
    for (const auto& record : records)
    {
        stats.total_count++;
        
        if (record.result == 0)
        {
            stats.success_count++;
        }
        else
        {
            stats.fail_count++;
        }
        
        // 统计日志类型
        stats.log_type_count[record.log_type]++;
        
        // 统计工厂
        stats.factory_count[record.factory_id]++;
        
        // 更新时间范围
        if (stats.time_beg.empty() || CompareTime(record.create_time, stats.time_beg))
        {
            stats.time_beg = record.create_time;
        }
        if (stats.time_end.empty() || CompareTime(stats.time_end, record.create_time))
        {
            stats.time_end = record.create_time;
        }
    }
    
    // 计算成功率
    if (stats.total_count > 0)
    {
        stats.success_rate = static_cast<double>(stats.success_count) / stats.total_count;
    }
    
    return stats;
}

std::map<std::string, PosNameStatistics> CProduceRecordData::GetAllStatistics() const
{
    std::map<std::string, PosNameStatistics> result;
    
    for (const auto& pair : m_recordsByPosName)
    {
        result[pair.first] = GetStatisticsByPosName(pair.first);
    }
    
    return result;
}

std::vector<std::string> CProduceRecordData::GetAllPosNames() const
{
    std::vector<std::string> result;
    result.reserve(m_recordsByPosName.size());
    
    for (const auto& pair : m_recordsByPosName)
    {
        result.push_back(pair.first);
    }
    
    return result;
}

std::vector<ProduceRecordItem> CProduceRecordData::GetAllRecords() const
{
    std::vector<ProduceRecordItem> result;
    result.reserve(m_allRecords.size());
    
    for (const auto& pair : m_allRecords)
    {
        result.push_back(pair.second);
    }
    
    return result;
}

size_t CProduceRecordData::GetTotalCount() const
{
    return m_allRecords.size();
}

std::vector<std::string> CProduceRecordData::FindSimilarPosNames(const std::string& pos_name, 
                                                                  double similarity_threshold) const
{
    std::vector<std::string> result;
    
    // 简单实现：查找包含相同前缀或后缀的机型
    // 未来可以扩展为更复杂的相似度算法
    
    for (const auto& pair : m_recordsByPosName)
    {
        const std::string& otherPosName = pair.first;
        
        if (otherPosName == pos_name)
        {
            continue; // 跳过自己
        }
        
        // 简单匹配：如果机型名称有公共前缀（至少3个字符）
        size_t minLen = std::min(pos_name.length(), otherPosName.length());
        if (minLen >= 3)
        {
            size_t commonPrefix = 0;
            for (size_t i = 0; i < minLen && pos_name[i] == otherPosName[i]; ++i)
            {
                commonPrefix++;
            }
            
            // 如果公共前缀长度超过阈值，认为是同类型
            if (commonPrefix >= 3)
            {
                result.push_back(otherPosName);
            }
        }
    }
    
    return result;
}

std::string CProduceRecordData::ToJson() const
{
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    
    writer.StartObject();
    
    // 元数据
    writer.Key("meta");
    writer.StartObject();
    writer.Key("total_count");
    writer.Uint64(m_allRecords.size());
    writer.Key("pos_name_count");
    writer.Uint64(m_recordsByPosName.size());
    writer.EndObject();
    
    // 统计信息
    writer.Key("statistics");
    writer.StartObject();
    auto allStats = GetAllStatistics();
    for (const auto& pair : allStats)
    {
        writer.Key(pair.first.c_str());
        writer.StartObject();
        writer.Key("total_count");
        writer.Int(pair.second.total_count);
        writer.Key("success_count");
        writer.Int(pair.second.success_count);
        writer.Key("fail_count");
        writer.Int(pair.second.fail_count);
        writer.Key("success_rate");
        writer.Double(pair.second.success_rate);
        writer.Key("time_beg");
        writer.String(pair.second.time_beg.c_str());
        writer.Key("time_end");
        writer.String(pair.second.time_end.c_str());
        writer.EndObject();
    }
    writer.EndObject();
    
    // 记录列表（按机型分组）
    writer.Key("records");
    writer.StartObject();
    for (const auto& posPair : m_recordsByPosName)
    {
        writer.Key(posPair.first.c_str());
        writer.StartArray();
        
        for (const auto& recordId : posPair.second)
        {
            auto it = m_allRecords.find(recordId);
            if (it != m_allRecords.end())
            {
                const ProduceRecordItem& record = it->second;
                writer.StartObject();
                writer.Key("id");
                writer.String(record.id.c_str());
                writer.Key("pos_sn");
                writer.String(record.pos_sn.c_str());
                writer.Key("pos_name");
                writer.String(record.pos_name.c_str());
                writer.Key("order");
                writer.String(record.order.c_str());
                writer.Key("factory_id");
                writer.String(record.factory_id.c_str());
                writer.Key("result");
                writer.Int(record.result);
                writer.Key("log_type");
                writer.Int(record.log_type);
                writer.Key("create_time");
                writer.String(record.create_time.c_str());
                writer.Key("ass_pid");
                writer.String(record.ass_pid.c_str());
                writer.Key("pack_pid");
                writer.String(record.pack_pid.c_str());
                writer.EndObject();
            }
        }
        
        writer.EndArray();
    }
    writer.EndObject();
    
    writer.EndObject();
    
    return buffer.GetString();
}

bool CProduceRecordData::FromJson(const std::string& json_str)
{
    Document doc;
    doc.Parse(json_str.c_str());
    
    if (doc.HasParseError())
    {
        return false;
    }
    
    Clear();
    
    // 解析records
    if (doc.HasMember("records") && doc["records"].IsObject())
    {
        const Value& recordsObj = doc["records"];
        for (Value::ConstMemberIterator it = recordsObj.MemberBegin(); 
             it != recordsObj.MemberEnd(); ++it)
        {
            if (it->value.IsArray())
            {
                for (const auto& recordValue : it->value.GetArray())
                {
                    if (recordValue.IsObject())
                    {
                        ProduceRecordItem record;
                        if (recordValue.HasMember("id") && recordValue["id"].IsString())
                            record.id = recordValue["id"].GetString();
                        if (recordValue.HasMember("pos_sn") && recordValue["pos_sn"].IsString())
                            record.pos_sn = recordValue["pos_sn"].GetString();
                        if (recordValue.HasMember("pos_name") && recordValue["pos_name"].IsString())
                            record.pos_name = recordValue["pos_name"].GetString();
                        if (recordValue.HasMember("order") && recordValue["order"].IsString())
                            record.order = recordValue["order"].GetString();
                        if (recordValue.HasMember("factory_id") && recordValue["factory_id"].IsString())
                            record.factory_id = recordValue["factory_id"].GetString();
                        if (recordValue.HasMember("result") && recordValue["result"].IsInt())
                            record.result = recordValue["result"].GetInt();
                        if (recordValue.HasMember("log_type") && recordValue["log_type"].IsInt())
                            record.log_type = recordValue["log_type"].GetInt();
                        if (recordValue.HasMember("create_time") && recordValue["create_time"].IsString())
                            record.create_time = recordValue["create_time"].GetString();
                        if (recordValue.HasMember("ass_pid") && recordValue["ass_pid"].IsString())
                            record.ass_pid = recordValue["ass_pid"].GetString();
                        if (recordValue.HasMember("pack_pid") && recordValue["pack_pid"].IsString())
                            record.pack_pid = recordValue["pack_pid"].GetString();
                        
                        AddRecord(record);
                    }
                }
            }
        }
    }
    
    return true;
}

void CProduceRecordData::Clear()
{
    m_allRecords.clear();
    m_recordsByPosName.clear();
    m_recordsByFactory.clear();
    m_recordsByLogType.clear();
    m_recordIdsByTime.clear();
}

std::vector<std::string> CProduceRecordData::GetSNListByPosName(const std::string& pos_name) const
{
    std::vector<std::string> result;
    auto records = GetRecordsByPosName(pos_name);
    
    for (const auto& record : records)
    {
        if (!record.pos_sn.empty())
        {
            result.push_back(record.pos_sn);
        }
    }
    
    // 去重
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    
    return result;
}

std::vector<std::string> CProduceRecordData::GetPIDListByPosName(const std::string& pos_name) const
{
    std::vector<std::string> result;
    auto records = GetRecordsByPosName(pos_name);
    
    for (const auto& record : records)
    {
        if (!record.ass_pid.empty())
        {
            result.push_back(record.ass_pid);
        }
        if (!record.pack_pid.empty())
        {
            result.push_back(record.pack_pid);
        }
    }
    
    // 去重
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    
    return result;
}

std::map<std::string, std::vector<ProduceRecordItem>> CProduceRecordData::GetRecordsByFactory() const
{
    std::map<std::string, std::vector<ProduceRecordItem>> result;
    
    for (const auto& pair : m_recordsByFactory)
    {
        for (const auto& recordId : pair.second)
        {
            auto it = m_allRecords.find(recordId);
            if (it != m_allRecords.end())
            {
                result[pair.first].push_back(it->second);
            }
        }
    }
    
    return result;
}

std::map<int, std::vector<ProduceRecordItem>> CProduceRecordData::GetRecordsByLogType() const
{
    std::map<int, std::vector<ProduceRecordItem>> result;
    
    for (const auto& pair : m_recordsByLogType)
    {
        for (const auto& recordId : pair.second)
        {
            auto it = m_allRecords.find(recordId);
            if (it != m_allRecords.end())
            {
                result[pair.first].push_back(it->second);
            }
        }
    }
    
    return result;
}

void CProduceRecordData::UpdateStatistics(const ProduceRecordItem& record)
{
    // 统计信息在GetStatisticsByPosName中实时计算，这里可以预留扩展
}

bool CProduceRecordData::CompareTime(const std::string& time1, const std::string& time2) const
{
    // 简单字符串比较（假设时间格式统一）
    return time1 < time2;
}

ProduceRecordItem CProduceRecordData::ConvertFromMap(const CStr2Map& recordMap) const
{
    ProduceRecordItem record;
    
    // 辅助函数：从const map中安全获取值
    auto getValue = [&recordMap](const std::string& key) -> std::string {
        auto it = recordMap.find(key);
        return (it != recordMap.end()) ? it->second : std::string();
    };
    
    // 使用find方法访问，如果key不存在会返回空字符串
    record.id = getValue("Fid");
    record.app_uid = getValue("Fapp_uid");
    record.pos_sn = getValue("Fpos_sn");
    record.order = getValue("Forder");
    record.factory_id = getValue("Factory");
    record.pos_name = getValue("Fpos_name");
    
    // 转换result
    std::string resultStr = getValue("Fresult");
    if (resultStr.empty()) resultStr = "0";
    record.result = (resultStr == "0" || resultStr == "成功") ? 0 : 1;
    
    record.message = getValue("Fmessage");
    record.create_time = getValue("Fcreate_time");
    
    // 转换log_type
    std::string logTypeStr = getValue("Flog_type");
    if (logTypeStr.empty()) logTypeStr = "0";
    record.log_type = atoi(logTypeStr.c_str());
    
    record.log_id = getValue("Flog_id");
    record.ass_pid = getValue("Fass_pid");
    record.pack_pid = getValue("Fpack_pid");
    record.operator_name = getValue("Foperator");
    
    // 可选字段
    record.cpu_id = getValue("Fcpu_id");
    record.flash_id = getValue("Falsh_id");
    record.wifi_macid = getValue("Fwifi_macid");
    record.blue_macid = getValue("Fblue_macid");
    record.eth_macid = getValue("Feth_macid");
    record.imei = getValue("Fimei");
    
    return record;
}

// ============================================================================
// CGetTermProduceReport 实现
// ============================================================================
int CGetTermProduceReport::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,tmpMap,outMap,sessionMap;
	string uid;

	// 取得请求参数
    pReqData->GetStrMap(inMap);
	
	//检查输入参数
	CheckParameter(inMap);

    //读取配置获取JSON存放路径
    char jsonPath[256] = {0};
    memcpy(jsonPath, g_mTransactions[GetTid()].m_mVars["json_path"].c_str(), g_mTransactions[GetTid()].m_mVars["json_path"].length());
    string jsonPathStr = string(jsonPath);

    //读取配置获取脚本路径
    char scriptPath[256] = {0};
    memcpy(scriptPath, g_mTransactions[GetTid()].m_mVars["python_path"].c_str(), g_mTransactions[GetTid()].m_mVars["python_path"].length());
    string scriptPathStr = string(scriptPath);

    //读取配置获取数据脚本路径
    char DataScriptPath[256] = {0};
    memcpy(DataScriptPath, g_mTransactions[GetTid()].m_mVars["get_data_python"].c_str(), g_mTransactions[GetTid()].m_mVars["get_data_python"].length());
    string getDataScriptPath = string(DataScriptPath);

    string targetPosName = inMap["pos_name"];
    string timeBeg = inMap["create_time_beg"];
    string timeEnd = inMap["create_time_end"];
    
    // ============================================================================
    // 1.创建目录 2.调用脚本获取数据 3.调用ES获取数据 4.找同类机型重复2-3 5.生成报告
    // ============================================================================
    
    // 第一步：创建目录
    string outputDir = jsonPathStr;
    
    // 构建目录名：机型_时间（格式：机型名_YYYYMMDDHHMMSS）
    string dirName = targetPosName + "_" + Datetime::now().format("YMDHIS");
    string fullDirPath = outputDir;
    if (fullDirPath[fullDirPath.length() - 1] != '/')
    {
        fullDirPath += "/";
    }
    fullDirPath += dirName;
    
    // 创建目录
    try
    {
        Directory::mkdir(fullDirPath.c_str());
        InfoLog("创建目录成功: %s", fullDirPath.c_str());
    }
    catch (const std::exception& e)
    {
        ErrorLog("创建目录失败: %s, 错误: %s", fullDirPath.c_str(), e.what());
        throw CTrsExp(ERR_SYSCODE_INCORRECT, "创建目录失败");
    }
    
    // 构建最终报告JSON文件路径（所有数据都写入这个文件）
    string reportJsonFile = fullDirPath + "/report.json";
    
    // 创建最终报告JSON文档（用于累积所有机型的数据）
    Document finalReportDoc;
    finalReportDoc.SetObject();
    Document::AllocatorType& allocator = finalReportDoc.GetAllocator();
    
    // 添加报告元数据（Python脚本期望的结构）
    finalReportDoc.AddMember("errorcode", Value("0000", allocator), allocator);
    finalReportDoc.AddMember("errormessage", Value("OK", allocator), allocator);
    
    // 创建report_meta对象
    Value reportMeta(kObjectType);
    reportMeta.AddMember("title", Value("机型生产分析报告", allocator), allocator);
    reportMeta.AddMember("target_pos_name", Value(targetPosName.c_str(), allocator), allocator);
    reportMeta.AddMember("time_range", Value().SetObject(), allocator);
    reportMeta["time_range"].AddMember("beg", Value(timeBeg.c_str(), allocator), allocator);
    reportMeta["time_range"].AddMember("end", Value(timeEnd.c_str(), allocator), allocator);
    reportMeta.AddMember("generate_time", Value(Datetime::now().format("Y-M-D H:I:S").c_str(), allocator), allocator);
    finalReportDoc.AddMember("report_meta", reportMeta, allocator);
    
    // 创建机型数据数组（用于内部处理）
    Value posDataArray(kArrayType);
    
    // 辅助函数：调用Python脚本获取生产记录数据
    auto getProductionData = [&](const string& posName) -> Value {
        InfoLog("调用Python脚本获取生产记录 - 机型: %s", posName.c_str());
        
        // 构建Python命令
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "python3 %s %s %s %s %s 1", 
                 getDataScriptPath.c_str(),
                 posName.c_str(),
                 timeBeg.c_str(),
                 timeEnd.c_str(),
                 fullDirPath.c_str());
        
        InfoLog("执行Python脚本命令: %s", cmd);
        
        // 执行脚本并读取JSON输出
        FILE* pipe = popen(cmd, "r");
        if (!pipe)
        {
            ErrorLog("执行Python脚本失败");
            return Value();  // 返回空值
        }
        
        // 读取脚本输出
        std::ostringstream scriptOutput;
        char buffer[4096];
        while (fgets(buffer, sizeof(buffer), pipe) != NULL)
        {
            scriptOutput << buffer;
        }
        int scriptRet = pclose(pipe);
        
        if (scriptRet != 0)
        {
            ErrorLog("Python脚本执行失败，返回码: %d，输出: %s", scriptRet, scriptOutput.str().c_str());
            return Value();  // 返回空值
        }
        
        string jsonOutput = scriptOutput.str();
        InfoLog("Python脚本返回JSON长度: %zu", jsonOutput.length());
        
        // 解析JSON结果
        Document doc;
        if (doc.Parse(jsonOutput.c_str()).HasParseError())
        {
            ErrorLog("解析Python脚本返回的JSON失败");
            return Value();  // 返回空值
        }
        
        if (!doc.HasMember("errorcode") || string(doc["errorcode"].GetString()) != "0000")
        {
            string errorMsg = doc.HasMember("errormessage") ? doc["errormessage"].GetString() : "未知错误";
            ErrorLog("Python脚本返回错误: %s", errorMsg.c_str());
            return Value();  // 返回空值
        }
        
        // 创建机型数据对象
        Value posData(kObjectType);
        posData.AddMember("pos_name", Value(posName.c_str(), allocator), allocator);
        
        // 复制统计数据（需要显式复制）
        if (doc.HasMember("total_count") && doc["total_count"].IsInt())
        {
            posData.AddMember("total_count", Value(doc["total_count"].GetInt()), allocator);
        }
        if (doc.HasMember("success_count") && doc["success_count"].IsInt())
        {
            posData.AddMember("success_count", Value(doc["success_count"].GetInt()), allocator);
        }
        if (doc.HasMember("fail_count") && doc["fail_count"].IsInt())
        {
            posData.AddMember("fail_count", Value(doc["fail_count"].GetInt()), allocator);
        }
        
        // 计算成功率
        double successRate = 0.0;
        if (doc.HasMember("total_count") && doc["total_count"].IsInt() && doc["total_count"].GetInt() > 0)
        {
            int total = doc["total_count"].GetInt();
            int success = doc.HasMember("success_count") && doc["success_count"].IsInt() ? doc["success_count"].GetInt() : 0;
            successRate = static_cast<double>(success) / total;
        }
        posData.AddMember("success_rate", Value(successRate), allocator);
        
        // 复制生产记录（需要显式复制）
        if (doc.HasMember("success_records") && doc["success_records"].IsArray())
        {
            Value successRecords(kArrayType);
            const Value& srcRecords = doc["success_records"];
            for (SizeType i = 0; i < srcRecords.Size(); ++i)
            {
                Value record(kObjectType);
                if (srcRecords[i].HasMember("pos_sn") && srcRecords[i]["pos_sn"].IsString())
                    record.AddMember("pos_sn", Value(srcRecords[i]["pos_sn"].GetString(), allocator), allocator);
                if (srcRecords[i].HasMember("create_time") && srcRecords[i]["create_time"].IsString())
                    record.AddMember("create_time", Value(srcRecords[i]["create_time"].GetString(), allocator), allocator);
                if (srcRecords[i].HasMember("log_path") && srcRecords[i]["log_path"].IsString())
                    record.AddMember("log_path", Value(srcRecords[i]["log_path"].GetString(), allocator), allocator);
                successRecords.PushBack(record, allocator);
            }
            posData.AddMember("success_records", successRecords, allocator);
        }
        if (doc.HasMember("fail_records") && doc["fail_records"].IsArray())
        {
            Value failRecords(kArrayType);
            const Value& srcRecords = doc["fail_records"];
            for (SizeType i = 0; i < srcRecords.Size(); ++i)
            {
                Value record(kObjectType);
                if (srcRecords[i].HasMember("pos_sn") && srcRecords[i]["pos_sn"].IsString())
                    record.AddMember("pos_sn", Value(srcRecords[i]["pos_sn"].GetString(), allocator), allocator);
                if (srcRecords[i].HasMember("create_time") && srcRecords[i]["create_time"].IsString())
                    record.AddMember("create_time", Value(srcRecords[i]["create_time"].GetString(), allocator), allocator);
                if (srcRecords[i].HasMember("log_path") && srcRecords[i]["log_path"].IsString())
                    record.AddMember("log_path", Value(srcRecords[i]["log_path"].GetString(), allocator), allocator);
                failRecords.PushBack(record, allocator);
            }
            posData.AddMember("fail_records", failRecords, allocator);
        }
        
        // 检查是否有日志压缩包
        if (doc.HasMember("archive_path") && doc["archive_path"].IsString())
        {
            string archivePath = doc["archive_path"].GetString();
            if (!archivePath.empty())
            {
                InfoLog("日志文件已打包: %s", archivePath.c_str());
                posData.AddMember("archive_path", Value(archivePath.c_str(), allocator), allocator);
            }
        }
        
        return posData;
    };
    
    // 辅助函数：调用ES获取数据
    auto getESData = [&](const string& posName) -> Value {
        InfoLog("调用ES获取数据 - 机型: %s", posName.c_str());
        
        // 确定ES索引名称
        string esIndexName = "fac";
        if (!inMap["es_index"].empty())
        {
            esIndexName = inMap["es_index"];
        }
        
        // 构建ES查询JSON（单个机型）
        vector<string> posNames;
        posNames.push_back(posName);
        string esQueryJson = CreateESQueryJson(posNames, timeBeg, timeEnd);
        
        // 执行ES查询
        string esResultJson;
        if (QueryElasticsearch(esIndexName, esQueryJson, esResultJson))
        {
            InfoLog("ES查询成功，返回数据长度: %zu", esResultJson.length());
            
            // 解析ES结果
            Document esDoc;
            if (ParseESAggregationResult(esResultJson, esDoc))
            {
                // 将ES数据转换为Value并返回（只返回aggregations部分）
                if (esDoc.HasMember("aggregations") && esDoc["aggregations"].IsObject())
                {
                    Value esData(kObjectType);
                    esData.AddMember("aggregations", Value(esDoc["aggregations"], allocator), allocator);
                    return esData;
                }
            }
        }
        
        return Value();  // 返回空值
    };
    
    // 第二步：获取目标机型的生产记录数据
    Value targetPosData = getProductionData(targetPosName);
    if (targetPosData.IsNull() || !targetPosData.IsObject())
    {
        ErrorLog("获取目标机型生产记录数据失败");
        throw CTrsExp(ERR_SYSCODE_INCORRECT, "获取目标机型生产记录数据失败");
    }
    
    // 第三步：获取目标机型的ES数据
    Value targetESData = getESData(targetPosName);
    if (targetESData.IsObject() && !targetESData.IsNull())
    {
        InfoLog("ES数据获取成功，准备保存到单独文件");
        
        // 将ES数据保存到单独文件，避免主JSON文件过大
        string esDataFile = fullDirPath + "/es_data.json";
        StringBuffer esBuffer;
        Writer<StringBuffer> esWriter(esBuffer);
        targetESData.Accept(esWriter);
        string esDataJson = esBuffer.GetString();
        
        // 检查ES数据大小，如果超过10MB，保存到单独文件
        size_t esDataSize = esDataJson.length();
        InfoLog("ES数据大小: %zu 字节 (%.2f MB)", esDataSize, esDataSize / 1024.0 / 1024.0);
        
        if (esDataSize > 10 * 1024 * 1024)  // 超过10MB
        {
            InfoLog("ES数据超过10MB，保存到单独文件: %s", esDataFile.c_str());
            std::ofstream esFile(esDataFile);
            if (esFile.is_open())
            {
                esFile << esDataJson;
                esFile.close();
                InfoLog("ES数据已保存到单独文件");
                
                // 在主数据中只保存文件路径引用
                Value esDataRef(kObjectType);
                esDataRef.AddMember("file_path", Value("es_data.json", allocator), allocator);
                esDataRef.AddMember("size", Value(static_cast<int64_t>(esDataSize)), allocator);
                targetPosData.AddMember("es_data", esDataRef, allocator);
            }
            else
            {
                ErrorLog("保存ES数据文件失败: %s", esDataFile.c_str());
                // 如果保存失败，仍然尝试添加到主JSON（可能导致文件过大）
                targetPosData.AddMember("es_data", targetESData, allocator);
            }
        }
        else
        {
            // ES数据较小，直接添加到主JSON
            InfoLog("ES数据较小，直接添加到主JSON");
            targetPosData.AddMember("es_data", targetESData, allocator);
        }
    }
    else
    {
        InfoLog("ES数据为空或无效，跳过添加");
    }
    
    // 验证targetPosData是否有效
    InfoLog("验证targetPosData: IsObject=%d, IsNull=%d", targetPosData.IsObject(), targetPosData.IsNull());
    if (!targetPosData.IsObject() || targetPosData.IsNull())
    {
        ErrorLog("targetPosData无效，无法添加到数组");
        throw CTrsExp(ERR_SYSCODE_INCORRECT, "targetPosData无效");
    }
    
    // 添加到机型数据数组
    InfoLog("准备将targetPosData添加到posDataArray");
    posDataArray.PushBack(targetPosData, allocator);
    
    InfoLog("目标机型数据已添加到数组，当前数组大小: %zu", posDataArray.Size());
    
    // 验证添加后的数据
    if (posDataArray.Size() > 0)
    {
        const Value& verifyData = posDataArray[0];
        InfoLog("验证添加后的数据: IsObject=%d, IsNull=%d, HasMember(pos_name)=%d", 
                verifyData.IsObject(), verifyData.IsNull(), 
                verifyData.HasMember("pos_name"));
    }
    
    // 第四步：查询同类机型（已屏蔽）
    /*
    InfoLog("开始查询同类机型 - 目标机型: %s", targetPosName.c_str());
    vector<string> similarPosNames;
    if (QuerySimilarPosNames(targetPosName, similarPosNames))
    {
        InfoLog("找到 %zu 个同类机型", similarPosNames.size());
        
        // 对每个同类机型重复步骤2和3
        for (const auto& posName : similarPosNames)
        {
            InfoLog("处理同类机型: %s", posName.c_str());
            
            // 获取生产记录数据
            Value similarPosData = getProductionData(posName);
            if (similarPosData.IsNull() || !similarPosData.IsObject())
            {
                ErrorLog("获取同类机型 %s 的生产记录数据失败，跳过", posName.c_str());
                continue;
            }
            
            // 获取ES数据
            Value similarESData = getESData(posName);
            if (similarESData.IsObject())
            {
                similarPosData.AddMember("es_data", similarESData, allocator);
            }
            
            // 添加到机型数据数组
            posDataArray.PushBack(similarPosData, allocator);
        }
    }
    else
    {
        ErrorLog("查询同类机型失败");
        // 不抛出异常，继续执行，只使用目标机型
    }
    */
    // 屏蔽同类机型查询，使用空数组
    vector<string> similarPosNames;
    InfoLog("同类机型查询已屏蔽，跳过查询");
    
    // 注意：不再将posDataArray添加到最终报告，因为ES数据可能很大
    // 如果ES数据超过10MB，已经保存到单独文件，主JSON中只包含文件路径引用
    InfoLog("准备构建最终报告，posDataArray大小: %zu", posDataArray.Size());
    // 不再添加pos_data到finalReportDoc，避免JSON文件过大
    // finalReportDoc.AddMember("pos_data", posDataArray, allocator);
    
    // 构建Python脚本期望的数据结构
    // 1. production_statistics
    Value productionStats(kObjectType);
    InfoLog("开始构建production_statistics，检查posDataArray...");
    if (posDataArray.Size() > 0)
    {
        try
        {
            InfoLog("尝试访问posDataArray[0]...");
            const Value& firstElement = posDataArray[0];
            InfoLog("posDataArray[0]访问成功，类型检查: IsObject=%d, IsNull=%d, IsArray=%d", 
                    firstElement.IsObject(), firstElement.IsNull(), firstElement.IsArray());
            
            if (firstElement.IsObject() && !firstElement.IsNull())
            {
                const Value& targetData = firstElement;
                InfoLog("开始构建targetPos对象");
                Value targetPos(kObjectType);
                targetPos.AddMember("pos_name", Value(targetPosName.c_str(), allocator), allocator);
                
                        InfoLog("检查targetData的成员...");
                if (targetData.HasMember("total_count") && targetData["total_count"].IsInt())
                {
                    int totalCount = targetData["total_count"].GetInt();
                    InfoLog("total_count: %d", totalCount);
                    targetPos.AddMember("total_count", Value(totalCount), allocator);
                }
                if (targetData.HasMember("success_count") && targetData["success_count"].IsInt())
                {
                    int successCount = targetData["success_count"].GetInt();
                    InfoLog("success_count: %d", successCount);
                    targetPos.AddMember("success_count", Value(successCount), allocator);
                }
                if (targetData.HasMember("fail_count") && targetData["fail_count"].IsInt())
                {
                    int failCount = targetData["fail_count"].GetInt();
                    InfoLog("fail_count: %d", failCount);
                    targetPos.AddMember("fail_count", Value(failCount), allocator);
                }
                if (targetData.HasMember("success_rate") && targetData["success_rate"].IsNumber())
                {
                    double successRate = targetData["success_rate"].GetDouble();
                    InfoLog("success_rate: %f", successRate);
                    targetPos.AddMember("success_rate", Value(successRate), allocator);
                }
                InfoLog("添加time_beg和time_end");
                targetPos.AddMember("time_beg", Value(timeBeg.c_str(), allocator), allocator);
                targetPos.AddMember("time_end", Value(timeEnd.c_str(), allocator), allocator);
                
                InfoLog("创建空对象by_log_type和by_factory");
                Value byLogType(kObjectType);
                Value byFactory(kObjectType);
                targetPos.AddMember("by_log_type", byLogType, allocator);  // 空对象，Python脚本会处理
                targetPos.AddMember("by_factory", byFactory, allocator);     // 空对象，Python脚本会处理
                
                InfoLog("添加target_pos到productionStats");
                productionStats.AddMember("target_pos", targetPos, allocator);
                
                // 同类机型统计
                InfoLog("构建同类机型统计，posDataArray大小: %zu", posDataArray.Size());
                Value similarStats(kArrayType);
                for (SizeType i = 1; i < posDataArray.Size(); ++i)
                {
                    if (!posDataArray[i].IsObject())
                    {
                        ErrorLog("跳过无效的同类机型数据 [索引: %zu]", i);
                        continue;
                    }
                    const Value& similarData = posDataArray[i];
                    Value similarPos(kObjectType);
                    if (similarData.HasMember("pos_name") && similarData["pos_name"].IsString())
                        similarPos.AddMember("pos_name", Value(similarData["pos_name"].GetString(), allocator), allocator);
                    if (similarData.HasMember("total_count") && similarData["total_count"].IsInt())
                        similarPos.AddMember("total_count", Value(similarData["total_count"].GetInt()), allocator);
                    if (similarData.HasMember("success_count") && similarData["success_count"].IsInt())
                        similarPos.AddMember("success_count", Value(similarData["success_count"].GetInt()), allocator);
                    if (similarData.HasMember("fail_count") && similarData["fail_count"].IsInt())
                        similarPos.AddMember("fail_count", Value(similarData["fail_count"].GetInt()), allocator);
                    if (similarData.HasMember("success_rate") && similarData["success_rate"].IsNumber())
                        similarPos.AddMember("success_rate", Value(similarData["success_rate"].GetDouble()), allocator);
                    similarStats.PushBack(similarPos, allocator);
                }
                InfoLog("添加similar_pos_stats，包含 %zu 个同类机型", similarStats.Size());
                productionStats.AddMember("similar_pos_stats", similarStats, allocator);
                InfoLog("production_statistics构建完成");
            }
            else
            {
                ErrorLog("posDataArray[0]不是有效的对象类型");
                throw std::runtime_error("posDataArray[0]不是有效的对象");
            }
        }
        catch (const std::exception& e)
        {
            ErrorLog("访问posDataArray[0]时发生异常: %s", e.what());
            // 使用默认值
            InfoLog("使用默认值构建production_statistics");
            Value targetPos(kObjectType);
            targetPos.AddMember("pos_name", Value(targetPosName.c_str(), allocator), allocator);
            targetPos.AddMember("total_count", Value(0), allocator);
            targetPos.AddMember("success_count", Value(0), allocator);
            targetPos.AddMember("fail_count", Value(0), allocator);
            targetPos.AddMember("success_rate", Value(0.0), allocator);
            targetPos.AddMember("time_beg", Value(timeBeg.c_str(), allocator), allocator);
            targetPos.AddMember("time_end", Value(timeEnd.c_str(), allocator), allocator);
            Value byLogType(kObjectType);
            Value byFactory(kObjectType);
            targetPos.AddMember("by_log_type", byLogType, allocator);
            targetPos.AddMember("by_factory", byFactory, allocator);
            productionStats.AddMember("target_pos", targetPos, allocator);
            Value emptySimilarStats(kArrayType);
            productionStats.AddMember("similar_pos_stats", emptySimilarStats, allocator);
        }
    }
    else
    {
        ErrorLog("警告: posDataArray为空或第一个元素无效，无法构建production_statistics");
        InfoLog("使用默认值构建production_statistics");
        // 创建空的target_pos对象
        Value targetPos(kObjectType);
        targetPos.AddMember("pos_name", Value(targetPosName.c_str(), allocator), allocator);
        targetPos.AddMember("total_count", Value(0), allocator);
        targetPos.AddMember("success_count", Value(0), allocator);
        targetPos.AddMember("fail_count", Value(0), allocator);
        targetPos.AddMember("success_rate", Value(0.0), allocator);
        targetPos.AddMember("time_beg", Value(timeBeg.c_str(), allocator), allocator);
        targetPos.AddMember("time_end", Value(timeEnd.c_str(), allocator), allocator);
        targetPos.AddMember("by_log_type", Value().SetObject(), allocator);
        targetPos.AddMember("by_factory", Value().SetObject(), allocator);
        productionStats.AddMember("target_pos", targetPos, allocator);
        Value emptySimilarStats(kArrayType);
        productionStats.AddMember("similar_pos_stats", emptySimilarStats, allocator);
    }
    
    InfoLog("添加production_statistics到finalReportDoc");
    finalReportDoc.AddMember("production_statistics", productionStats, allocator);
    InfoLog("production_statistics添加完成");
    
    // 2. similar_pos_names
    InfoLog("构建similar_pos_names，数量: %zu", similarPosNames.size());
    Value similarPosNamesArray(kArrayType);
    for (const auto& posName : similarPosNames)
    {
        similarPosNamesArray.PushBack(Value(posName.c_str(), allocator), allocator);
    }
    InfoLog("添加similar_pos_names到finalReportDoc");
    finalReportDoc.AddMember("similar_pos_names", similarPosNamesArray, allocator);
    InfoLog("similar_pos_names添加完成");
    // 3. es_module_statistics（从目标机型的ES数据中提取）
    // 注意：如果ES数据保存在单独文件中，这里只添加文件路径引用，Python脚本会读取文件
    InfoLog("构建es_module_statistics");
    Value esModuleStats(kObjectType);
    if (posDataArray.Size() > 0 && posDataArray[0].IsObject())
    {
        const Value& targetData = posDataArray[0];
        InfoLog("检查es_data...");
        if (targetData.HasMember("es_data") && targetData["es_data"].IsObject() && !targetData["es_data"].IsNull())
        {
            const Value& esData = targetData["es_data"];
            
            // 检查是否是文件引用（包含file_path字段）
            if (esData.HasMember("file_path") && esData["file_path"].IsString())
            {
                // ES数据保存在单独文件中，只添加文件路径引用
                InfoLog("ES数据保存在单独文件中: %s", esData["file_path"].GetString());
                Value esDataRef(kObjectType);
                esDataRef.AddMember("file_path", Value(esData["file_path"].GetString(), allocator), allocator);
                if (esData.HasMember("size") && esData["size"].IsInt64())
                {
                    esDataRef.AddMember("size", Value(esData["size"].GetInt64()), allocator);
                }
                esModuleStats.AddMember(Value(targetPosName.c_str(), allocator), esDataRef, allocator);
                InfoLog("esModuleStats添加文件引用完成");
            }
            else if (esData.HasMember("aggregations") && esData["aggregations"].IsObject() && !esData["aggregations"].IsNull())
            {
                // ES数据直接包含在主JSON中（数据较小的情况）
                InfoLog("ES数据直接包含在主JSON中");
                Value posModuleData(kObjectType);
                Value modulesArray(kArrayType);
                
                // 解析ES聚合结果（这里简化处理，实际需要根据ES返回结构解析）
                // 暂时添加空结构，Python脚本会处理
                posModuleData.AddMember("modules", modulesArray, allocator);
                esModuleStats.AddMember(Value(targetPosName.c_str(), allocator), posModuleData, allocator);
                InfoLog("esModuleStats添加完成");
            }
        }
    }
    InfoLog("添加es_module_statistics到finalReportDoc");
    finalReportDoc.AddMember("es_module_statistics", esModuleStats, allocator);
    InfoLog("es_module_statistics添加完成");
    // 4. production_records_detail（从目标机型数据中提取，包含所有记录）
    InfoLog("构建production_records_detail");
    Value productionRecordsDetail(kArrayType);
    if (posDataArray.Size() > 0 && posDataArray[0].IsObject())
    {
        const Value& targetData = posDataArray[0];
        InfoLog("开始处理生产记录详情");
        // 合并成功和失败记录（包含所有记录，不限制数量）
        if (targetData.HasMember("success_records") && targetData["success_records"].IsArray())
        {
            const Value& successRecords = targetData["success_records"];
            for (SizeType i = 0; i < successRecords.Size(); ++i)
            {
                // 检查数组元素是否有效
                if (!successRecords[i].IsObject())
                {
                    ErrorLog("跳过无效的成功记录 [索引: %zu]", i);
                    continue;
                }
                
                // 需要显式复制记录
                Value record(kObjectType);
                if (successRecords[i].HasMember("pos_sn") && successRecords[i]["pos_sn"].IsString())
                    record.AddMember("pos_sn", Value(successRecords[i]["pos_sn"].GetString(), allocator), allocator);
                if (successRecords[i].HasMember("create_time") && successRecords[i]["create_time"].IsString())
                    record.AddMember("create_time", Value(successRecords[i]["create_time"].GetString(), allocator), allocator);
                if (successRecords[i].HasMember("log_path") && successRecords[i]["log_path"].IsString())
                    record.AddMember("log_path", Value(successRecords[i]["log_path"].GetString(), allocator), allocator);
                record.AddMember("result", Value(0), allocator);  // 成功记录，result=0
                productionRecordsDetail.PushBack(record, allocator);
            }
        }
        if (targetData.HasMember("fail_records") && targetData["fail_records"].IsArray())
        {
            const Value& failRecords = targetData["fail_records"];
            for (SizeType i = 0; i < failRecords.Size(); ++i)
            {
                // 检查数组元素是否有效
                if (!failRecords[i].IsObject())
                {
                    ErrorLog("跳过无效的失败记录 [索引: %zu]", i);
                    continue;
                }
                
                // 需要显式复制记录
                Value record(kObjectType);
                if (failRecords[i].HasMember("pos_sn") && failRecords[i]["pos_sn"].IsString())
                    record.AddMember("pos_sn", Value(failRecords[i]["pos_sn"].GetString(), allocator), allocator);
                if (failRecords[i].HasMember("create_time") && failRecords[i]["create_time"].IsString())
                    record.AddMember("create_time", Value(failRecords[i]["create_time"].GetString(), allocator), allocator);
                if (failRecords[i].HasMember("log_path") && failRecords[i]["log_path"].IsString())
                    record.AddMember("log_path", Value(failRecords[i]["log_path"].GetString(), allocator), allocator);
                record.AddMember("result", Value(1), allocator);  // 失败记录，result=1
                productionRecordsDetail.PushBack(record, allocator);
            }
        }
        InfoLog("production_records_detail处理完成，记录数: %zu", productionRecordsDetail.Size());
    }
    else
    {
        InfoLog("posDataArray为空或无效，跳过production_records_detail");
    }
    
    InfoLog("添加production_records_detail到finalReportDoc");
    finalReportDoc.AddMember("production_records_detail", productionRecordsDetail, allocator);
    InfoLog("production_records_detail添加完成");
    
    // 5. error_analysis（空对象，Python脚本会处理）
    finalReportDoc.AddMember("error_analysis", Value().SetObject(), allocator);
    
    // 6. last_year_comparison（空对象，Python脚本会处理）
    finalReportDoc.AddMember("last_year_comparison", Value().SetObject(), allocator);
    
    // 将最终报告JSON写入文件
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    finalReportDoc.Accept(writer);
    string finalReportJson = buffer.GetString();
    
    // 保存JSON文件
    std::ofstream jsonFile(reportJsonFile);
    if (jsonFile.is_open())
    {
        jsonFile << finalReportJson;
        jsonFile.close();
        InfoLog("JSON文件已保存: %s", reportJsonFile.c_str());
    }
    else
    {
        ErrorLog("保存JSON文件失败: %s", reportJsonFile.c_str());
        throw CTrsExp(ERR_SYSCODE_INCORRECT, "保存JSON文件失败");
    }
    
    // 第五步：调用Python脚本生成HTML报告
    if (!scriptPathStr.empty())
    {
        InfoLog("开始调用Python脚本生成HTML报告");
        
        // 构建HTML输出路径（与JSON在同一目录）
        string htmlFileName = fullDirPath + "/report.html";
        
        // 构建Python命令（将输出重定向到日志文件以便调试）
        string logFile = fullDirPath + "/python_script.log";
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "python3 %s %s %s > %s 2>&1", 
                 scriptPathStr.c_str(), reportJsonFile.c_str(), htmlFileName.c_str(), logFile.c_str());
        
        InfoLog("执行Python命令: %s", cmd);
        
        int ret = std::system(cmd);
        
        // 无论成功失败，都读取日志文件以便调试
        if (File::exists(logFile))
        {
            std::ifstream logFileStream(logFile.c_str(), std::ios::in | std::ios::binary);
            if (logFileStream.is_open())
            {
                std::ostringstream oss;
                oss << logFileStream.rdbuf();
                string logContent = oss.str();
                logFileStream.close();
                
                if (!logContent.empty())
                {
                    if (ret == 0)
                    {
                        InfoLog("Python脚本输出: %s", logContent.c_str());
                    }
                    else
                    {
                        ErrorLog("Python脚本执行失败，返回码: %d，错误信息: %s", ret, logContent.c_str());
                    }
                }
            }
        }
        else
        {
            ErrorLog("Python脚本日志文件不存在: %s", logFile.c_str());
        }
        
        if (ret == 0)
        {
            InfoLog("HTML报告生成成功: %s", htmlFileName.c_str());
            
            // 检查HTML文件是否生成成功
            if (File::exists(htmlFileName))
            {
                // 读取配置获取下载路径基础目录
                char downPathBase[256] = {0};
                string downPathBaseStr;
                if (!g_mTransactions[GetTid()].m_mVars["report_down_path"].empty())
                {
                    memcpy(downPathBase, g_mTransactions[GetTid()].m_mVars["report_down_path"].c_str(),
                           g_mTransactions[GetTid()].m_mVars["report_down_path"].length());
                    downPathBaseStr = string(downPathBase);
                }
                else if (!g_mTransactions[GetTid()].m_mVars["down_path_base"].empty())
                {
                    memcpy(downPathBase, g_mTransactions[GetTid()].m_mVars["down_path_base"].c_str(),
                           g_mTransactions[GetTid()].m_mVars["down_path_base"].length());
                    downPathBaseStr = string(downPathBase);
                }
                
                // 如果配置了下载路径，将HTML文件复制到下载目录
                string reportUrl = "";
                if (!downPathBaseStr.empty())
                {
                    // 构建下载目录中的文件名
                    string downloadFileName = dirName + ".html";
                    string downloadFilePath = downPathBaseStr;
                    if (downloadFilePath[downloadFilePath.length() - 1] != '/')
                    {
                        downloadFilePath += "/";
                    }
                    downloadFilePath += downloadFileName;
                    
                    // 复制文件到下载目录
                    string copyCmd = "cp -f " + htmlFileName + " " + downloadFilePath;
                    InfoLog("复制文件命令: %s", copyCmd.c_str());
                    
                    if (std::system(copyCmd.c_str()) == 0)
                    {
                        InfoLog("文件已复制到下载目录: %s", downloadFilePath.c_str());
                        
                        // 提取相对路径（从download开始）
                        size_t nPos = downloadFilePath.find("download");
                        if (nPos != string::npos)
                        {
                            reportUrl = downloadFilePath.substr(nPos);
                        }
                        else
                        {
                            // 如果没有download，使用完整路径
                            reportUrl = downloadFilePath;
                        }
                    }
                    else
                    {
                        ErrorLog("复制文件到下载目录失败");
                        // 如果复制失败，使用原始路径
                        size_t nPos = htmlFileName.find("download");
                        if (nPos != string::npos)
                        {
                            reportUrl = htmlFileName.substr(nPos);
                        }
                    }
                }
                else
                {
                    // 没有配置下载路径，直接使用HTML文件路径
                    size_t nPos = htmlFileName.find("download");
                    if (nPos != string::npos)
                    {
                        reportUrl = htmlFileName.substr(nPos);
                    }
                    else
                    {
                        reportUrl = htmlFileName;
                    }
                }
                
                // 返回报告URL
                if (!reportUrl.empty())
                {
                    pResData->SetPara("report_url", reportUrl);
                    InfoLog("报告URL: %s", reportUrl.c_str());
                }
            }
            else
            {
                ErrorLog("HTML文件生成失败，文件不存在: %s", htmlFileName.c_str());
            }
        }
        else
        {
            ErrorLog("Python脚本执行失败，返回码: %d", ret);
        }
    }
    else
    {
        ErrorLog("Python脚本路径配置为空，无法生成HTML报告");
    }
    
    // 添加响应数据（从production_statistics中提取统计信息）
    if (finalReportDoc.HasMember("production_statistics") && finalReportDoc["production_statistics"].IsObject())
    {
        const Value& prodStats = finalReportDoc["production_statistics"];
        if (prodStats.HasMember("target_pos") && prodStats["target_pos"].IsObject())
        {
            const Value& targetPos = prodStats["target_pos"];
            if (targetPos.HasMember("total_count") && targetPos["total_count"].IsInt())
            {
                pResData->SetPara("total_count", std::to_string(targetPos["total_count"].GetInt()));
            }
            if (targetPos.HasMember("success_count") && targetPos["success_count"].IsInt())
            {
                pResData->SetPara("success_count", std::to_string(targetPos["success_count"].GetInt()));
            }
            if (targetPos.HasMember("fail_count") && targetPos["fail_count"].IsInt())
            {
                pResData->SetPara("fail_count", std::to_string(targetPos["fail_count"].GetInt()));
            }
            if (targetPos.HasMember("success_rate") && targetPos["success_rate"].IsNumber())
            {
                char successRateStr[32];
                snprintf(successRateStr, sizeof(successRateStr), "%.2f", targetPos["success_rate"].GetDouble() * 100);
                pResData->SetPara("success_rate", successRateStr);
            }
        }
    }
    
    // 将文件路径添加到响应中（用于调试）
    pResData->SetPara("json_file_path", reportJsonFile);
    pResData->SetPara("json_dir_path", fullDirPath);
    
    // 返回JSON响应
    pResData->SetJsonText(CIdentPub::TransformJson(finalReportJson));
    
	return 0;
}

// 输入判断
void CGetTermProduceReport::CheckParameter( CStr2Map& inMap)
{
	if(inMap["pos_name"].empty())
    {
        ErrorLog("机型不能为空");
		CIdentPub::SendAlarm2("机型不能为空[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"机型不能为空");
    }
    if(inMap["create_time_beg"].empty() || inMap["create_time_end"].empty())
    {
        inMap["create_time_end"]= aps::Date::now().format("Y-M-D");
        inMap["create_time_beg"]= aps::Date::now().addMonths(-3).format("Y-M-D");
    }
}

// 查询同类机型（根据Fsmt_node、Fac_node、Fpack_node相同）
bool CGetTermProduceReport::QuerySimilarPosNames(const string& targetPosName, vector<string>& similarPosNames)
{
    similarPosNames.clear();
    
    // 1. 查询目标机型的节点信息
    CStr2Map targetInMap, targetOutMap;
    targetInMap["term_name"] = targetPosName;
    
    if (!CIdentRelayApi::QueryTermWorkNode(targetInMap, targetOutMap, true))
    {
        ErrorLog("查询目标机型节点信息失败: %s", targetPosName.c_str());
        return false;
    }
    
    string targetSmtNode = targetOutMap["Fsmt_node"];
    string targetFacNode = targetOutMap["Fac_node"];
    string targetPackNode = targetOutMap["Fpack_node"];
    
    InfoLog("目标机型 [%s] 节点信息: SMT=%s, FAC=%s, PACK=%s", 
            targetPosName.c_str(), 
            targetSmtNode.c_str(), 
            targetFacNode.c_str(), 
            targetPackNode.c_str());
    
    // 如果目标机型没有节点信息，无法查找同类机型
    if (targetSmtNode.empty() && targetFacNode.empty() && targetPackNode.empty())
    {
        InfoLog("目标机型没有节点信息，无法查找同类机型");
        return true; // 返回成功，但没有同类机型
    }
    
    // 2. 查询所有机型列表
    CStr2Map termListInMap, termListOutMap;
    termListInMap["limit"] = "0";  // 获取所有机型
    termListInMap["offset"] = "0";
    
    vector<CStr2Map> termListArray;
    if (!CIdentRelayApi::QueryTermList(termListInMap, termListOutMap, termListArray, false))
    {
        ErrorLog("查询机型列表失败");
        return false;
    }
    
    InfoLog("查询到 %zu 个机型，开始匹配同类机型", termListArray.size());
    
    // 3. 遍历所有机型，查找节点信息相同的机型
    for (size_t i = 0; i < termListArray.size(); ++i)
    {
        string posName = termListArray[i]["Fterm_name"];
        
        // 跳过目标机型本身
        if (posName == targetPosName)
        {
            continue;
        }
        
        // 查询该机型的节点信息
        CStr2Map posInMap, posOutMap;
        posInMap["term_name"] = posName;
        
        if (!CIdentRelayApi::QueryTermWorkNode(posInMap, posOutMap, false))
        {
            // 查询失败，跳过
            continue;
        }
        
        string smtNode = posOutMap["Fsmt_node"];
        string facNode = posOutMap["Fac_node"];
        string packNode = posOutMap["Fpack_node"];
        
        // 比较节点信息：Fsmt_node、Fac_node、Fpack_node都相同
        if (smtNode == targetSmtNode && 
            facNode == targetFacNode && 
            packNode == targetPackNode)
        {
            similarPosNames.push_back(posName);
            InfoLog("找到同类机型: %s (SMT=%s, FAC=%s, PACK=%s)", 
                    posName.c_str(), smtNode.c_str(), facNode.c_str(), packNode.c_str());
        }
    }
    
    return true;
}

// 构建ES查询JSON（根据时间+机型查询）
string CGetTermProduceReport::CreateESQueryJson(const vector<string>& posNames, 
                                                     const string& timeBeg, 
                                                     const string& timeEnd)
{
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    
    writer.StartObject();
    
    // 设置size为0，只返回聚合结果
    writer.Key("size");
    writer.Int(0);
    
    // 构建query
    writer.Key("query");
    writer.StartObject();
    writer.Key("bool");
    writer.StartObject();
    writer.Key("must");
    writer.StartArray();
    
    // 1. 时间范围条件
    writer.StartObject();
    writer.Key("range");
    writer.StartObject();
    writer.Key("timestamp");
    writer.StartObject();
    writer.Key("gte");
    writer.String(timeBeg.c_str());
    writer.Key("lte");
    writer.String(timeEnd.c_str());
    writer.EndObject(); // 结束 timestamp
    writer.EndObject(); // 结束 range
    writer.EndObject(); // 结束 must 条件
    
    // 2. 机型条件（使用terms查询，匹配多个机型）
    if (!posNames.empty())
    {
        writer.StartObject();
        writer.Key("terms");
        writer.StartObject();
        writer.Key("posname");
        writer.StartArray();
        for (const auto& posName : posNames)
        {
            writer.String(posName.c_str());
        }
        writer.EndArray(); // 结束 posname 数组
        writer.EndObject(); // 结束 terms
        writer.EndObject(); // 结束 must 条件
    }
    
    writer.EndArray(); // 结束 must
    writer.EndObject(); // 结束 bool
    writer.EndObject(); // 结束 query
    
    // 添加聚合（按机型统计）
    writer.Key("aggs");
    writer.StartObject();
    
    // 按机型聚合
    writer.Key("by_posname");
    writer.StartObject();
    writer.Key("terms");
    writer.StartObject();
    writer.Key("field");
    writer.String("posname");
    writer.Key("size");
    writer.Int(100);  // 最多返回100个机型
    writer.EndObject(); // 结束 terms
    
    // 嵌套聚合：按功能模块统计
    writer.Key("aggs");
    writer.StartObject();
    
    // 嵌套查询funclist
    writer.Key("nested_funclist");
    writer.StartObject();
    writer.Key("nested");
    writer.StartObject();
    writer.Key("path");
    writer.String("funclist");
    writer.EndObject(); // 结束 nested
    
    writer.Key("aggs");
    writer.StartObject();
    
    // 按功能模块聚合
    writer.Key("by_func");
    writer.StartObject();
    writer.Key("terms");
    writer.StartObject();
    writer.Key("field");
    writer.String("funclist.func");
    writer.Key("size");
    writer.Int(1000);
    writer.EndObject(); // 结束 terms
    
    // 功能模块的统计
    writer.Key("aggs");
    writer.StartObject();
    
    // 总计数
    writer.Key("total_count");
    writer.StartObject();
    writer.Key("value_count");
    writer.StartObject();
    writer.Key("field");
    writer.String("funclist.func");
    writer.EndObject(); // 结束 value_count
    writer.EndObject(); // 结束 total_count
    
    // 成功计数
    writer.Key("success_count");
    writer.StartObject();
    writer.Key("filter");
    writer.StartObject();
    writer.Key("term");
    writer.StartObject();
    writer.Key("funclist.OnceSuccess");
    writer.String("Yes");
    writer.EndObject(); // 结束 term
    writer.EndObject(); // 结束 filter
    writer.EndObject(); // 结束 success_count
    
    // 成功率计算
    writer.Key("success_rate");
    writer.StartObject();
    writer.Key("bucket_script");
    writer.StartObject();
    writer.Key("buckets_path");
    writer.StartObject();
    writer.Key("success");
    writer.String("success_count._count");
    writer.Key("total");
    writer.String("total_count");
    writer.EndObject(); // 结束 buckets_path
    writer.Key("script");
    writer.String("params.total > 0 ? params.success / params.total : 0");
    writer.EndObject(); // 结束 bucket_script
    writer.EndObject(); // 结束 success_rate
    
    // Duration统计（性能指标）
    writer.Key("avg_duration");
    writer.StartObject();
    writer.Key("avg");
    writer.StartObject();
    writer.Key("field");
    writer.String("funclist.duration");
    writer.EndObject(); // 结束 avg
    writer.EndObject(); // 结束 avg_duration
    
    writer.Key("min_duration");
    writer.StartObject();
    writer.Key("min");
    writer.StartObject();
    writer.Key("field");
    writer.String("funclist.duration");
    writer.EndObject(); // 结束 min
    writer.EndObject(); // 结束 min_duration
    
    writer.Key("max_duration");
    writer.StartObject();
    writer.Key("max");
    writer.StartObject();
    writer.Key("field");
    writer.String("funclist.duration");
    writer.EndObject(); // 结束 max
    writer.EndObject(); // 结束 max_duration
    
    writer.Key("p95_duration");
    writer.StartObject();
    writer.Key("percentiles");
    writer.StartObject();
    writer.Key("field");
    writer.String("funclist.duration");
    writer.Key("percents");
    writer.StartArray();
    writer.Double(95.0);
    writer.EndArray(); // 结束 percents
    writer.EndObject(); // 结束 percentiles
    writer.EndObject(); // 结束 p95_duration
    
    writer.EndObject(); // 结束 aggs
    writer.EndObject(); // 结束 by_func
    writer.EndObject(); // 结束 aggs
    writer.EndObject(); // 结束 nested_funclist
    
    writer.EndObject(); // 结束 aggs
    writer.EndObject(); // 结束 by_posname
    
    writer.EndObject(); // 结束 aggs
    
    writer.EndObject(); // 结束整个JSON
    
    return buffer.GetString();
}

// 发送ES查询请求
bool CGetTermProduceReport::QueryElasticsearch(const string& indexName, const string& queryJson, string& resultJson)
{
    // 构建ES查询URL
    string esUrl = g_esCfg.strESBaseUrl + indexName + "/_search";
    
    InfoLog("ES查询URL: %s", esUrl.c_str());
    InfoLog("ES查询JSON: %s", queryJson.c_str());
    
    // 发送HTTP POST请求
    if (CIdentPub::HttpESPost(esUrl, queryJson, resultJson))
    {
        ErrorLog("ES查询HTTP请求失败: %s", esUrl.c_str());
        return false;
    }
    
    InfoLog("ES查询成功，返回数据长度: %zu", resultJson.length());
    
    // 验证返回的JSON是否有效
    rapidjson::Document doc;
    if (doc.Parse(resultJson.c_str()).HasParseError())
    {
        ErrorLog("ES返回JSON解析失败");
        return false;
    }
    
    return true;
}

// 解析ES聚合结果
bool CGetTermProduceReport::ParseESAggregationResult(const string& esResultJson, rapidjson::Document& esData)
{
    if (esResultJson.empty())
    {
        return false;
    }
    
    // 解析JSON
    if (esData.Parse(esResultJson.c_str()).HasParseError())
    {
        ErrorLog("ES结果JSON解析失败");
        return false;
    }
    
    // 验证是否有aggregations字段
    if (!esData.HasMember("aggregations") || !esData["aggregations"].IsObject())
    {
        ErrorLog("ES结果中没有aggregations字段");
        return false;
    }
    
    return true;
}

// 生成最终报告JSON（整合生产记录数据 + ES数据）
// 注意：produceData可能只包含前50条记录，统计信息需要从外部传入
string CGetTermProduceReport::GenerateFinalReportJson(CProduceRecordData& produceData, 
                                                       const rapidjson::Document& esData,
                                                       const string& targetPosName,
                                                       const vector<string>& similarPosNames,
                                                       const string& timeBeg,
                                                       const string& timeEnd,
                                                       int totalCount,
                                                       int successCount,
                                                       int failCount,
                                                       double successRate)
{
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    
    writer.StartObject();
    
    // 报告元数据
    writer.Key("report_meta");
    writer.StartObject();
    writer.Key("title");
    writer.String("机型生产分析报告");
    writer.Key("target_pos_name");
    writer.String(targetPosName.c_str());
    writer.Key("time_range");
    writer.StartObject();
    writer.Key("beg");
    writer.String(timeBeg.c_str());
    writer.Key("end");
    writer.String(timeEnd.c_str());
    writer.EndObject();
    writer.Key("generate_time");
    writer.String(Datetime::now().format("Y-M-D H:I:S").c_str());
    writer.Key("similar_pos_count");
    writer.Int(similarPosNames.size());
    writer.EndObject();
    
    // 查询去年同期数据（用于对比）
    writer.Key("last_year_comparison");
    writer.StartObject();
    
    // 计算去年同期时间范围
    try
    {
        // 解析日期字符串 (格式: YYYY-MM-DD)
        // 提取年月日
        int yearBeg = 0, monthBeg = 0, dayBeg = 0;
        int yearEnd = 0, monthEnd = 0, dayEnd = 0;
        
        if (sscanf(timeBeg.c_str(), "%d-%d-%d", &yearBeg, &monthBeg, &dayBeg) == 3 &&
            sscanf(timeEnd.c_str(), "%d-%d-%d", &yearEnd, &monthEnd, &dayEnd) == 3)
        {
            Datetime timeBegDt, timeEndDt;
            timeBegDt.setDatetime(yearBeg, monthBeg, dayBeg, 0, 0, 0, 0);
            timeEndDt.setDatetime(yearEnd, monthEnd, dayEnd, 0, 0, 0, 0);
            
            // 减去一年（使用addMonths，12个月）
            Datetime lastYearBeg = timeBegDt.addMonths(-12);
            Datetime lastYearEnd = timeEndDt.addMonths(-12);
            
            string lastYearBegStr = lastYearBeg.format("Y-M-D");
            string lastYearEndStr = lastYearEnd.format("Y-M-D");
        
        // 查询去年同期的生产记录数量
        CStr2Map lastYearQueryInMap, lastYearQueryOutMap;
        lastYearQueryInMap["pos_name"] = targetPosName;
        lastYearQueryInMap["create_time_beg"] = lastYearBegStr;
        lastYearQueryInMap["create_time_end"] = lastYearEndStr;
        lastYearQueryInMap["limit"] = "0";
        lastYearQueryInMap["offset"] = "0";
        
        vector<CStr2Map> lastYearRecords;
        if (CIdentRelayApi::QueryProduceRecordListNoToal(lastYearQueryInMap, lastYearQueryOutMap, lastYearRecords, false))
        {
            // 统计去年同期的数据
            int lastYearTotal = 0;
            int lastYearSuccess = 0;
            int lastYearFail = 0;
            
            for (const auto& record : lastYearRecords)
            {
                lastYearTotal++;
                auto it = record.find("Fresult");
                string resultStr = (it != record.end()) ? it->second : "0";
                if (resultStr.empty()) resultStr = "0";
                if (resultStr == "0" || resultStr == "成功")
                {
                    lastYearSuccess++;
                }
                else
                {
                    lastYearFail++;
                }
            }
            
            writer.Key("time_range");
            writer.StartObject();
            writer.Key("beg");
            writer.String(lastYearBegStr.c_str());
            writer.Key("end");
            writer.String(lastYearEndStr.c_str());
            writer.EndObject();
            
            writer.Key("total_count");
            writer.Int(lastYearTotal);
            writer.Key("success_count");
            writer.Int(lastYearSuccess);
            writer.Key("fail_count");
            writer.Int(lastYearFail);
            
            double lastYearSuccessRate = 0.0;
            if (lastYearTotal > 0)
            {
                lastYearSuccessRate = static_cast<double>(lastYearSuccess) / lastYearTotal;
            }
            writer.Key("success_rate");
            writer.Double(lastYearSuccessRate);
        }
        else
        {
            writer.Key("total_count");
            writer.Int(0);
        }
        }
        else
        {
            // 日期解析失败
            writer.Key("total_count");
            writer.Int(0);
        }
    }
    catch (...)
    {
        // 解析时间失败，不添加去年同期数据
        writer.Key("total_count");
        writer.Int(0);
    }
    
    writer.EndObject(); // 结束last_year_comparison
    
    // 同类机型列表
    writer.Key("similar_pos_names");
    writer.StartArray();
    for (const auto& posName : similarPosNames)
    {
        writer.String(posName.c_str());
    }
    writer.EndArray();
    
    // 生产记录统计信息
    writer.Key("production_statistics");
    writer.StartObject();
    
    // 目标机型统计（使用传入的统计信息，因为produceData只包含前50条记录）
    writer.Key("target_pos");
    writer.StartObject();
    writer.Key("pos_name");
    writer.String(targetPosName.c_str());
    writer.Key("total_count");
    writer.Int(totalCount);
    writer.Key("success_count");
    writer.Int(successCount);
    writer.Key("fail_count");
    writer.Int(failCount);
    writer.Key("success_rate");
    writer.Double(successRate);
    writer.Key("time_beg");
    writer.String(timeBeg.c_str());
    writer.Key("time_end");
    writer.String(timeEnd.c_str());
    
    // 按日志类型统计（从produceData的前50条记录中统计，用于展示）
    writer.Key("by_log_type");
    writer.StartObject();
    PosNameStatistics tempStats = produceData.GetStatisticsByPosName(targetPosName);
    for (const auto& pair : tempStats.log_type_count)
    {
        char key[32];
        snprintf(key, sizeof(key), "%d", pair.first);
        writer.Key(key);
        writer.Int(pair.second);
    }
    writer.EndObject();
    
    // 按工厂统计（从produceData的前50条记录中统计，用于展示）
    writer.Key("by_factory");
    writer.StartObject();
    for (const auto& pair : tempStats.factory_count)
    {
        writer.Key(pair.first.c_str());
        writer.Int(pair.second);
    }
    writer.EndObject();
    
    writer.EndObject(); // 结束target_pos
    
    // 同类机型统计
    writer.Key("similar_pos_stats");
    writer.StartArray();
    for (const auto& posName : similarPosNames)
    {
        PosNameStatistics stats = produceData.GetStatisticsByPosName(posName);
        writer.StartObject();
        writer.Key("pos_name");
        writer.String(posName.c_str());
        writer.Key("total_count");
        writer.Int(stats.total_count);
        writer.Key("success_count");
        writer.Int(stats.success_count);
        writer.Key("fail_count");
        writer.Int(stats.fail_count);
        writer.Key("success_rate");
        writer.Double(stats.success_rate);
        writer.EndObject();
    }
    writer.EndArray();
    
    writer.EndObject(); // 结束production_statistics
    
    // ES聚合数据（模块通过率统计）
    writer.Key("es_module_statistics");
    if (esData.HasMember("aggregations") && esData["aggregations"].HasMember("by_posname"))
    {
        const rapidjson::Value& byPosname = esData["aggregations"]["by_posname"];
        if (byPosname.HasMember("buckets") && byPosname["buckets"].IsArray())
        {
            writer.StartObject();
            const rapidjson::Value& buckets = byPosname["buckets"];
            
            for (rapidjson::SizeType i = 0; i < buckets.Size(); ++i)
            {
                const rapidjson::Value& bucket = buckets[i];
                if (!bucket.HasMember("key") || !bucket["key"].IsString())
                {
                    continue;
                }
                
                string posName = bucket["key"].GetString();
                writer.Key(posName.c_str());
                writer.StartObject();
                
                // 机型总记录数
                if (bucket.HasMember("doc_count") && bucket["doc_count"].IsInt())
                {
                    writer.Key("total_docs");
                    writer.Int(bucket["doc_count"].GetInt());
                }
                
                // 功能模块统计
                if (bucket.HasMember("nested_funclist") && 
                    bucket["nested_funclist"].HasMember("by_func") &&
                    bucket["nested_funclist"]["by_func"].HasMember("buckets"))
                {
                    writer.Key("modules");
                    writer.StartArray();
                    
                    const rapidjson::Value& moduleBuckets = bucket["nested_funclist"]["by_func"]["buckets"];
                    for (rapidjson::SizeType j = 0; j < moduleBuckets.Size(); ++j)
                    {
                        const rapidjson::Value& moduleBucket = moduleBuckets[j];
                        writer.StartObject();
                        
                        // 模块名称
                        if (moduleBucket.HasMember("key") && moduleBucket["key"].IsString())
                        {
                            writer.Key("module_name");
                            writer.String(moduleBucket["key"].GetString());
                        }
                        
                        // 总计数
                        if (moduleBucket.HasMember("total_count") && 
                            moduleBucket["total_count"].HasMember("value"))
                        {
                            writer.Key("total_count");
                            writer.Int(moduleBucket["total_count"]["value"].GetInt());
                        }
                        
                        // 成功计数
                        if (moduleBucket.HasMember("success_count") && 
                            moduleBucket["success_count"].HasMember("doc_count"))
                        {
                            writer.Key("success_count");
                            writer.Int(moduleBucket["success_count"]["doc_count"].GetInt());
                        }
                        
                        // 成功率
                        if (moduleBucket.HasMember("success_rate") && 
                            moduleBucket["success_rate"].HasMember("value"))
                        {
                            writer.Key("success_rate");
                            writer.Double(moduleBucket["success_rate"]["value"].GetDouble());
                        }
                        
                        // Duration性能指标
                        if (moduleBucket.HasMember("avg_duration") && 
                            moduleBucket["avg_duration"].HasMember("value"))
                        {
                            writer.Key("avg_duration_ms");
                            writer.Double(moduleBucket["avg_duration"]["value"].GetDouble());
                        }
                        
                        if (moduleBucket.HasMember("min_duration") && 
                            moduleBucket["min_duration"].HasMember("value"))
                        {
                            writer.Key("min_duration_ms");
                            writer.Double(moduleBucket["min_duration"]["value"].GetDouble());
                        }
                        
                        if (moduleBucket.HasMember("max_duration") && 
                            moduleBucket["max_duration"].HasMember("value"))
                        {
                            writer.Key("max_duration_ms");
                            writer.Double(moduleBucket["max_duration"]["value"].GetDouble());
                        }
                        
                        if (moduleBucket.HasMember("p95_duration") && 
                            moduleBucket["p95_duration"].HasMember("percentiles") &&
                            moduleBucket["p95_duration"]["percentiles"].HasMember("95.0"))
                        {
                            writer.Key("p95_duration_ms");
                            writer.Double(moduleBucket["p95_duration"]["percentiles"]["95.0"].GetDouble());
                        }
                        
                        writer.EndObject();
                    }
                    
                    writer.EndArray();
                }
                
                writer.EndObject();
            }
            
            writer.EndObject();
        }
        else
        {
            writer.Null();
        }
    }
    else
    {
        writer.Null();
    }
    
    // 生产记录详情（最多100条，支持分页）
    writer.Key("production_records_detail");
    writer.StartArray();
    vector<ProduceRecordItem> allRecords = produceData.GetRecordsByPosName(targetPosName);
    // 限制记录数量，最多100条
    size_t maxRecords = std::min(allRecords.size(), size_t(100));
    for (size_t i = 0; i < maxRecords; ++i)
    {
        const ProduceRecordItem& record = allRecords[i];
        writer.StartObject();
        writer.Key("id");
        writer.String(record.id.c_str());
        writer.Key("pos_sn");
        writer.String(record.pos_sn.c_str());
        writer.Key("order");
        writer.String(record.order.c_str());
        writer.Key("factory_id");
        writer.String(record.factory_id.c_str());
        writer.Key("result");
        writer.Int(record.result);
        writer.Key("message");
        writer.String(record.message.c_str());
        writer.Key("create_time");
        writer.String(record.create_time.c_str());
        // 工位段映射：0、1表示包装段，2表示组装段，3表示贴片段
        int station_type = record.log_type;
        string station_name = "";
        if (station_type == 0 || station_type == 1)
        {
            station_name = "包装段";
        }
        else if (station_type == 2)
        {
            station_name = "组装段";
        }
        else if (station_type == 3)
        {
            station_name = "贴片段";
        }
        else
        {
            station_name = "未知";
        }
        
        writer.Key("log_type");
        writer.Int(record.log_type);
        writer.Key("station_type");
        writer.String(station_name.c_str());
        writer.Key("log_id");
        writer.String(record.log_id.c_str());
        writer.Key("ass_pid");
        writer.String(record.ass_pid.c_str());
        writer.Key("pack_pid");
        writer.String(record.pack_pid.c_str());
        writer.EndObject();
    }
    writer.EndArray();
    
    // 错误分析（按错误类型、时间、工厂等分组）
    writer.Key("error_analysis");
    writer.StartObject();
    
    // 按错误消息分组统计
    writer.Key("by_error_message");
    writer.StartObject();
    map<string, int> errorMsgCount;
    for (const auto& record : allRecords)
    {
        if (record.result == 1 && !record.message.empty())
        {
            errorMsgCount[record.message]++;
        }
    }
    for (const auto& pair : errorMsgCount)
    {
        writer.Key(pair.first.c_str());
        writer.Int(pair.second);
    }
    writer.EndObject();
    
    // 按时间分组统计错误
    writer.Key("by_time");
    writer.StartObject();
    map<string, int> errorByTime;
    for (const auto& record : allRecords)
    {
        if (record.result == 1 && !record.create_time.empty())
        {
            // 提取日期部分（假设格式为 YYYY-MM-DD HH:MM:SS）
            string datePart = record.create_time.substr(0, 10);
            errorByTime[datePart]++;
        }
    }
    for (const auto& pair : errorByTime)
    {
        writer.Key(pair.first.c_str());
        writer.Int(pair.second);
    }
    writer.EndObject();
    
    // 按工厂分组统计错误
    writer.Key("by_factory");
    writer.StartObject();
    map<string, int> errorByFactory;
    for (const auto& record : allRecords)
    {
        if (record.result == 1 && !record.factory_id.empty())
        {
            errorByFactory[record.factory_id]++;
        }
    }
    for (const auto& pair : errorByFactory)
    {
        writer.Key(pair.first.c_str());
        writer.Int(pair.second);
    }
    writer.EndObject();
    
    writer.EndObject(); // 结束error_analysis
    
    // 生产记录摘要
    writer.Key("production_records_summary");
    writer.StartObject();
    writer.Key("total_records");
    writer.Uint64(produceData.GetTotalCount());
    writer.Key("pos_names");
    writer.StartArray();
    vector<string> allPosNames = produceData.GetAllPosNames();
    for (const auto& posName : allPosNames)
    {
        writer.String(posName.c_str());
    }
    writer.EndArray();
    writer.EndObject();
    
    writer.EndObject(); // 结束整个JSON

    
    return buffer.GetString();
}



