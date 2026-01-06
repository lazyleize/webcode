/************************************************************
 Desc:     AC控制器Trap告警信息列表接口 - 从Memcached读取Trap数据并返回给前端
 Modify:   2025-12-08
 ***********************************************************/
 #include "CAcTrapMessageList.h"
 #include "CIdentComm.h"
 #include "CIdentRelayApi.h"
#include "transxmlcfg.h"
 #include <sstream>
 #include <iomanip>
#include <algorithm>
#include <ctime>
#include <cstring>

// 取消宏定义，避免冲突
#undef max
#undef min
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
 
 using namespace std;
using namespace rapidjson;

// Trap级别映射表
static string GetTrapLevel(const string& trapType)
{
    // 恢复类告警（Restore/Clear）通常映射为info级别
    if (trapType.find("-restore") != string::npos || 
        trapType.find("-clear") != string::npos ||
        trapType.find("-resume") != string::npos ||
        trapType.find("-recover") != string::npos ||
        trapType == "ap-normal" ||
        trapType == "link-up" ||
        trapType == "if-flow-up" ||
        trapType == "if-input-rate-resume" ||
        trapType == "if-output-rate-resume")
    {
        return "info";
    }
    
    // 故障类告警
    if (trapType == "ap-fault" ||
        trapType == "link-down" ||
        trapType == "if-flow-down" ||
        trapType.find("entity-") != string::npos ||
        trapType.find("ap-power-fail") != string::npos ||
        trapType.find("ap-power-invalid") != string::npos ||
        trapType.find("ap-fan-invalid") != string::npos ||
        trapType.find("ap-optical-invalid") != string::npos)
    {
        return "error";
    }
    
    // 警告类告警
    if (trapType.find("warning") != string::npos ||
        trapType.find("overload") != string::npos ||
        trapType.find("too-high") != string::npos ||
        trapType.find("too-low") != string::npos ||
        trapType.find("conflict") != string::npos ||
        trapType.find("interf") != string::npos)
    {
        return "warning";
    }
    
    // 正常状态变更
    if (trapType == "ap-online" ||
        trapType == "ap-cold-boot" ||
        trapType == "ap-hot-boot" ||
        trapType == "ap-power-insert" ||
        trapType == "ap-fan-insert")
    {
        return "info";
    }
    
    // 未知类型（包括unknown和huawei-unknown）
    if (trapType == "unknown" || trapType == "huawei-unknown" || trapType.empty())
    {
        return "warning";  // 未知类型默认为warning级别
    }
    
    // 默认
    return "warning";
}

// 生成告警消息
static string GetTrapMessage(const string& trapType, const Document& parsedData)
{
    if (trapType == "ap-fault")
        return "AP通讯故障";
    else if (trapType == "ap-normal")
        return "AP通讯故障已恢复";
    else if (trapType == "ap-online")
        return "AP设备上线";
    else if (trapType == "ap-offline")
        return "AP设备下线";
    else if (trapType == "sta-num-warning")
    {
        if (parsedData.HasMember("max_sta_num") && parsedData["max_sta_num"].IsString() &&
            parsedData.HasMember("cur_sta_num") && parsedData["cur_sta_num"].IsString())
        {
            int maxNum = atoi(parsedData["max_sta_num"].GetString());
            int curNum = atoi(parsedData["cur_sta_num"].GetString());
            int percent = maxNum > 0 ? (curNum * 100) / maxNum : 0;
            char msg[128];
            snprintf(msg, sizeof(msg), "用户数达到告警阈值：当前 %d/%d (%d%%)", curNum, maxNum, percent);
            return string(msg);
        }
        return "用户数达到告警阈值";
    }
    else if (trapType == "sta-num-max")
        return "用户数达到最大值";
    else if (trapType == "ap-cpu-overload")
    {
        if (parsedData.HasMember("cpu_rate") && parsedData["cpu_rate"].IsString())
        {
            char msg[128];
            snprintf(msg, sizeof(msg), "AP CPU利用率过高：%s%%", parsedData["cpu_rate"].GetString());
            return string(msg);
        }
        return "AP CPU利用率过高";
    }
    else if (trapType == "ap-memory-overload")
    {
        if (parsedData.HasMember("mem_rate") && parsedData["mem_rate"].IsString())
        {
            char msg[128];
            snprintf(msg, sizeof(msg), "AP内存利用率过高：%s%%", parsedData["mem_rate"].GetString());
            return string(msg);
        }
        return "AP内存利用率过高";
    }
    else if (trapType == "ap-sta-full")
        return "AP用户数已满";
    else if (trapType == "ap-temp-too-high")
        return "AP温度过高";
    else if (trapType == "ap-temp-too-low")
        return "AP温度过低";
    else if (trapType == "ap-disk-overload")
        return "AP磁盘空间满";
    else if (trapType == "ap-power-limited")
        return "AP供电不足，工作在受限模式";
    else if (trapType == "link-down")
        return "网络接口断开";
    else if (trapType == "link-up")
        return "网络接口连接";
    else if (trapType.find("entity-") != string::npos)
    {
        if (trapType.find("cpu-rising") != string::npos)
            return "CPU占用率超过阈值";
        else if (trapType.find("mem-rising") != string::npos)
            return "系统内存占用率超过阈值";
        else if (trapType.find("disk-rising") != string::npos)
            return "磁盘利用率超过阈值";
        else if (trapType.find("brd-temp") != string::npos)
            return "单板温度告警";
    }
    else if (trapType == "wlan-event")
        return "WLAN事件";
    else if (trapType == "wlan-ap-event")
        return "WLAN AP事件";
    else if (trapType == "wlan-stats-event")
        return "WLAN统计事件";
    else if (trapType == "wlan-radio-event")
        return "WLAN射频事件";
    else if (trapType == "wlan-station-event")
        return "WLAN终端事件";
    else if (trapType == "huawei-entity-event")
        return "华为设备实体事件";
    else if (trapType == "huawei-ifnet-event")
        return "华为网络接口事件";
    
    // 未知类型Trap的消息
    if (trapType == "unknown" || trapType == "huawei-unknown")
    {
        return "未知类型Trap告警（OID未解析）";
    }
    
    // 默认消息
    return "Trap告警";
}

// 解析时间字符串为时间戳
static time_t ParseTimeString(const string& timeStr)
{
    if (timeStr.empty()) return 0;
    
    struct tm tm_time = {0};
    if (sscanf(timeStr.c_str(), "%d-%d-%d %d:%d:%d",
               &tm_time.tm_year, &tm_time.tm_mon, &tm_time.tm_mday,
               &tm_time.tm_hour, &tm_time.tm_min, &tm_time.tm_sec) == 6)
    {
        tm_time.tm_year -= 1900;
        tm_time.tm_mon -= 1;
        return mktime(&tm_time);
    }
    return 0;
}

// 检查时间是否在范围内
static bool IsTimeInRange(const string& timeStr, const string& startTime, const string& endTime)
{
    if (startTime.empty() && endTime.empty())
        return true;
    
    time_t trapTime = ParseTimeString(timeStr);
    if (trapTime == 0) return false;
    
    if (!startTime.empty())
    {
        time_t start = ParseTimeString(startTime);
        if (start > 0 && trapTime < start)
            return false;
    }
    
    if (!endTime.empty())
    {
        time_t end = ParseTimeString(endTime);
        if (end > 0 && trapTime > end)
            return false;
    }
    
    return true;
}
 
 int CAcTrapMessageList::IdentCommit(CReqData *pReqData, CResData *pResData)
 {
    InfoLog("========== AC控制器Trap告警信息列表接口 ==========");
    
    CStr2Map inMap, outMap;
    pReqData->GetStrMap(inMap);
    
    // ========== 获取请求参数 ==========
    string acIp = inMap["ac_ip"];
    if (acIp.empty())
    {
        // 如果没有指定ac_ip，从配置获取
        acIp = g_AcCfg.ip;
    }
    
    string trapType = inMap["trap_type"];      // Trap类型过滤
    string level = inMap["level"];              // 告警级别过滤
    string startTime = inMap["start_time"];     // 开始时间
    string endTime = inMap["end_time"];         // 结束时间
    string format = inMap["format"];             // 返回格式：simple/detail
    if (format.empty()) format = "simple";
    
    int page = atoi(inMap["page"].c_str());
    if (page <= 0) page = 1;
    int pageSize = atoi(inMap["page_size"].c_str());
    if (pageSize <= 0) pageSize = 20;
    if (pageSize > 100) pageSize = 100;
    
    InfoLog("查询参数: ac_ip=%s, trap_type=%s, level=%s, start_time=%s, end_time=%s, page=%d, page_size=%d, format=%s",
            acIp.c_str(), trapType.c_str(), level.c_str(), startTime.c_str(), endTime.c_str(), page, pageSize, format.c_str());
    
    // ========== 从Memcached读取Trap列表 ==========
    string listKey = "ac_trap_list_" + acIp;
    string listData;
    
    if (GetCache(listKey, listData) != 0 || listData.empty())
    {
        InfoLog("从Memcached读取Trap列表失败或为空: key=%s", listKey.c_str());
        // 返回空列表
        pResData->SetPara("ret_num", "0");
        pResData->SetPara("total", "0");
        pResData->SetPara("page", std::to_string(page));
        pResData->SetPara("page_size", std::to_string(pageSize));
        return 0;
    }
    
    InfoLog("从Memcached读取Trap列表成功: key=%s, 数据长度=%zu", listKey.c_str(), listData.length());
    
    // ========== 解析Trap列表（每行一个JSON对象）==========
    // 注意：rapidjson::Document 不能被复制，使用指针存储
    vector<Document*> allTraps;
    string::size_type pos = 0;
    int lineCount = 0;
    
    while (pos < listData.length())
    {
        // 查找一行（以换行符分隔）
        string::size_type lineEnd = listData.find('\n', pos);
        string line;
        if (lineEnd == string::npos)
        {
            line = listData.substr(pos);
            pos = listData.length();
        }
        else
        {
            line = listData.substr(pos, lineEnd - pos);
            pos = lineEnd + 1;
        }
        
        // 跳过空行
        if (line.empty() || line.find_first_not_of(" \t\r\n") == string::npos)
            continue;
        
        // 解析JSON（使用new创建，后续需要delete）
        Document* doc = new Document();
        if (doc->Parse(line.c_str()).HasParseError())
        {
            ErrorLog("解析Trap JSON失败，跳过该行: %s", line.substr(0, 100).c_str());
            delete doc;
            continue;
        }
        
        allTraps.push_back(doc);
        lineCount++;
    }
    
    InfoLog("解析到 %zu 个Trap记录", allTraps.size());
    
    // ========== 过滤Trap ==========
    vector<Document*> filteredTraps;
    
    for (size_t i = 0; i < allTraps.size(); ++i)
    {
        Document* trap = allTraps[i];
        
        // 1. 过滤ac_ip（如果指定了不同的ac_ip）
        if (!acIp.empty() && trap->HasMember("source_ip"))
        {
            if (string((*trap)["source_ip"].GetString()) != acIp)
                continue;
        }
        
        // 2. 过滤trap_type
        if (!trapType.empty() && trap->HasMember("trap_type"))
        {
            string trapTypeValue = (*trap)["trap_type"].GetString();
            // 如果查询的是unknown或huawei-unknown，也要匹配
            if (trapTypeValue != trapType)
            {
                // 特殊处理：如果查询的是"unknown"，也要匹配"huawei-unknown"
                if (trapType == "unknown" && trapTypeValue == "huawei-unknown")
                {
                    // 允许通过
                }
                else if (trapType == "huawei-unknown" && trapTypeValue == "unknown")
                {
                    // 允许通过
                }
                else
                {
                    continue;  // 不匹配，跳过
                }
            }
        }
        
        // 3. 过滤level（需要先计算level）
        if (!level.empty())
        {
            string trapTypeValue = trap->HasMember("trap_type") ? (*trap)["trap_type"].GetString() : "";
            string trapLevel = GetTrapLevel(trapTypeValue);
            if (trapLevel != level)
                continue;
        }
        
        // 4. 过滤时间范围
        if (trap->HasMember("receive_time"))
        {
            string receiveTime = (*trap)["receive_time"].GetString();
            if (!IsTimeInRange(receiveTime, startTime, endTime))
                continue;
        }
        
        filteredTraps.push_back(trap);
    }
    
    InfoLog("过滤后剩余 %zu 个Trap记录", filteredTraps.size());
    
    // ========== 排序（按时间倒序，最新的在前）==========
    sort(filteredTraps.begin(), filteredTraps.end(), 
         [](const Document* a, const Document* b) {
             time_t timeA = 0, timeB = 0;
             if (a->HasMember("receive_timestamp") && (*a)["receive_timestamp"].IsInt64())
                 timeA = (*a)["receive_timestamp"].GetInt64();
             else if (a->HasMember("receive_time"))
                 timeA = ParseTimeString((*a)["receive_time"].GetString());
             
             if (b->HasMember("receive_timestamp") && (*b)["receive_timestamp"].IsInt64())
                 timeB = (*b)["receive_timestamp"].GetInt64();
             else if (b->HasMember("receive_time"))
                 timeB = ParseTimeString((*b)["receive_time"].GetString());
             
             return timeA > timeB;  // 倒序
         });
    
    // ========== 分页 ==========
    int total = filteredTraps.size();
    int offset = (page - 1) * pageSize;
    int endPos = offset + pageSize;
    if (endPos > total) endPos = total;
    
    int retNum = endPos - offset;
    
    // ========== 设置返回参数 ==========
    pResData->SetPara("ret_num", std::to_string(retNum));  // 当前返回的条数（必填）
    pResData->SetPara("total", std::to_string(total));     // 总共有多少记录（必填）
    pResData->SetPara("page", std::to_string(page));
    pResData->SetPara("page_size", std::to_string(pageSize));
    
    // ========== 转换为接口格式并设置数组数据 ==========
    for (int i = offset; i < endPos; ++i)
    {
        try
        {
            Document* trap = filteredTraps[i];
            if (trap == NULL)
            {
                ErrorLog("Trap指针为空，跳过: index=%d", i);
                continue;
            }
            
            CStr2Map returnMap;
        
        // 基础字段（添加类型检查，避免异常）
        string sourceIp = "";
        if (trap->HasMember("source_ip") && (*trap)["source_ip"].IsString())
            sourceIp = (*trap)["source_ip"].GetString();
        
        string hostname = "";
        if (trap->HasMember("hostname") && (*trap)["hostname"].IsString())
            hostname = (*trap)["hostname"].GetString();
        
        string receiveTime = "";
        if (trap->HasMember("receive_time") && (*trap)["receive_time"].IsString())
            receiveTime = (*trap)["receive_time"].GetString();
        
        string receiveTimestamp = "";
        if (trap->HasMember("receive_timestamp"))
        {
            if ((*trap)["receive_timestamp"].IsInt64())
                receiveTimestamp = std::to_string((*trap)["receive_timestamp"].GetInt64());
            else if ((*trap)["receive_timestamp"].IsString())
                receiveTimestamp = (*trap)["receive_timestamp"].GetString();
        }
        
        string trapOid = "";
        if (trap->HasMember("trap_oid") && (*trap)["trap_oid"].IsString())
            trapOid = (*trap)["trap_oid"].GetString();
        
        string trapTypeValue = "unknown";
        if (trap->HasMember("trap_type") && (*trap)["trap_type"].IsString())
            trapTypeValue = (*trap)["trap_type"].GetString();
        
        string enterpriseOid = "";
        if (trap->HasMember("enterprise_oid") && (*trap)["enterprise_oid"].IsString())
            enterpriseOid = (*trap)["enterprise_oid"].GetString();
        
        // 解析parsed_data
        const Value* parsedDataPtr = NULL;
        if (trap->HasMember("parsed_data") && (*trap)["parsed_data"].IsObject())
        {
            parsedDataPtr = &(*trap)["parsed_data"];
        }
        
        // ========== 按固定顺序添加所有字段（确保每个记录字段完全一致）==========
        // 1. id
        string trapId = "trap_" + sourceIp + "_" + receiveTimestamp;
        returnMap["id"] = trapId;
        
        // 2. level（优先使用采集时生成的level，否则重新生成）
        string trapLevel = "warning";
        if (parsedDataPtr && parsedDataPtr->HasMember("level") && 
            (*parsedDataPtr)["level"].IsString() && (*parsedDataPtr)["level"].GetStringLength() > 0)
        {
            // 使用采集时生成的level字段
            trapLevel = (*parsedDataPtr)["level"].GetString();
        }
        else
        {
            // 如果没有，则重新生成（兼容旧数据）
            trapLevel = GetTrapLevel(trapTypeValue);
        }
        returnMap["level"] = trapLevel;
        
        // 3. device（优先使用采集时生成的device，其次按优先级：AP名称 > AC名称 > IP）
        string deviceName = "";
        if (parsedDataPtr && parsedDataPtr->HasMember("device") && 
            (*parsedDataPtr)["device"].IsString() && (*parsedDataPtr)["device"].GetStringLength() > 0)
        {
            // 使用采集时生成的device字段
            deviceName = (*parsedDataPtr)["device"].GetString();
        }
        else if (parsedDataPtr && parsedDataPtr->HasMember("ap_name") && 
                 (*parsedDataPtr)["ap_name"].IsString() && (*parsedDataPtr)["ap_name"].GetStringLength() > 0)
        {
            deviceName = (*parsedDataPtr)["ap_name"].GetString();
        }
        else if (parsedDataPtr && parsedDataPtr->HasMember("ap_id") && (*parsedDataPtr)["ap_id"].IsString())
        {
            deviceName = "AP-" + string((*parsedDataPtr)["ap_id"].GetString());
        }
        else if (parsedDataPtr && parsedDataPtr->HasMember("ap_mac") && (*parsedDataPtr)["ap_mac"].IsString())
        {
            deviceName = (*parsedDataPtr)["ap_mac"].GetString();
        }
        else if (!hostname.empty())
        {
            deviceName = hostname;  // 使用AC名称（hostname）
        }
        else
        {
            deviceName = sourceIp;  // 最后使用IP
        }
        returnMap["device"] = deviceName;
        
        // 4. type
        returnMap["type"] = trapTypeValue;
        
        // 5. message（优先使用采集时生成的message，否则重新生成）
        string message = "Trap告警";
        if (parsedDataPtr && parsedDataPtr->HasMember("message") && 
            (*parsedDataPtr)["message"].IsString() && (*parsedDataPtr)["message"].GetStringLength() > 0)
        {
            // 使用采集时生成的message字段
            message = (*parsedDataPtr)["message"].GetString();
        }
        else
        {
            // 如果没有，则重新生成（兼容旧数据）
            try
            {
                Document parsedDataForMsg;
                if (parsedDataPtr)
                {
                    StringBuffer buffer;
                    Writer<StringBuffer> writer(buffer);
                    parsedDataPtr->Accept(writer);
                    rapidjson::ParseResult result = parsedDataForMsg.Parse(buffer.GetString());
                    if (!result)
                    {
                        ErrorLog("解析parsed_data失败，使用默认消息");
                        message = GetTrapMessage(trapTypeValue, Document());
                    }
                    else
                    {
                        message = GetTrapMessage(trapTypeValue, parsedDataForMsg);
                    }
                }
                else
                {
                    message = GetTrapMessage(trapTypeValue, Document());
                }
            }
            catch (const std::exception& e)
            {
                ErrorLog("生成告警消息时发生异常: %s", e.what());
                message = "Trap告警";
            }
            catch (...)
            {
                ErrorLog("生成告警消息时发生未知异常");
                message = "Trap告警";
            }
        }
        returnMap["message"] = message;
        
        // 6. time
        returnMap["time"] = receiveTime;
        
        // 7. source_ip
        returnMap["source_ip"] = sourceIp;
        
        // 8. hostname（始终存在，即使为空）
        returnMap["hostname"] = hostname;
        
        // 9. trap_oid
        returnMap["trap_oid"] = trapOid;
        
        // 10. enterprise_oid
        returnMap["enterprise_oid"] = enterpriseOid;
        
        // 11. timestamp
        returnMap["timestamp"] = receiveTimestamp;
        
        // 12. vars（始终存在，format=detail时返回实际值，否则为空字符串）
        string varsStr = "";
        if (format == "detail" && trap->HasMember("vars") && (*trap)["vars"].IsObject())
        {
            try
            {
                StringBuffer varsBuffer;
                Writer<StringBuffer> varsWriter(varsBuffer);
                (*trap)["vars"].Accept(varsWriter);
                varsStr = varsBuffer.GetString();
            }
            catch (...)
            {
                varsStr = "";
            }
        }
        returnMap["vars"] = varsStr;
        
        // 添加到返回数组
        pResData->SetArray(returnMap);
        }
        catch (const std::exception& e)
        {
            ErrorLog("处理Trap记录时发生异常 (index=%d): %s", i, e.what());
            // 继续处理下一个记录
            continue;
        }
        catch (...)
        {
            ErrorLog("处理Trap记录时发生未知异常 (index=%d)", i);
            // 继续处理下一个记录
            continue;
        }
    }
    
    // ========== 清理内存 ==========
    // 释放所有Document对象
    for (size_t i = 0; i < allTraps.size(); ++i)
    {
        delete allTraps[i];
    }
    allTraps.clear();
    filteredTraps.clear();  // filteredTraps中的指针已经在allTraps中，不需要重复delete
    
    InfoLog("返回Trap列表: 总数=%d, 当前页=%d, 每页=%d, 返回条数=%d", total, page, pageSize, retNum);
     return 0;
 }
 