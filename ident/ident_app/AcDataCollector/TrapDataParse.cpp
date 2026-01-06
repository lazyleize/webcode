/************************************************************
 Desc:     Trap业务数据解析实现 - 根据Trap_MIB文档.md中的定义解析所有Trap类型
 Auth:     Auto
 Modify:
 data:     2025-12-08
 ***********************************************************/
#include "CTrapReceiver.h"
#include "CIdentAppComm.h"
#include <string>
#include <map>
#include <cstring>
#include <cstdlib>

using namespace std;

// 外部声明日志对象
extern CIdentAppComm* pIdentAppComm;

// 日志宏（使用现有的日志系统）
#define TrapInfoLog(fmt, ...) \
    do { \
        if (pIdentAppComm) { \
            XdInfoLog("[TrapParse] " fmt, ##__VA_ARGS__); \
        } \
    } while(0)

#define TrapErrorLog(fmt, ...) \
    do { \
        if (pIdentAppComm) { \
            XdErrorLog("[TrapParse] " fmt, ##__VA_ARGS__); \
        } \
    } while(0)

// 使用CTrapReceiver类中的FindVarBindValue函数

// 函数声明
static string GetTrapMessage(const string& trapType, const map<string, string>& parsedData);
static string GetTrapLevel(const string& trapType);

// 扩展ParseBusinessData函数 - 解析所有Trap类型
// 注意：此函数是CTrapReceiver类的成员函数，可以直接使用FindVarBindValue
int CTrapReceiver::ParseBusinessData(TrapInfo& trap)
{
    // 设置事件类型（从trapType获取）
    trap.parsedData["event"] = trap.trapType;
    
    // ========== IFNET模块Trap（网络接口相关）==========
    if (trap.trapOid.find("1.3.6.1.4.1.2011.5.25.41.3.5") != string::npos)
    {
        // hwIfFlowDown（接口流量Down告警）
        trap.parsedData["interface"] = FindVarBindValue(trap.vars, "Interface", "");
        trap.parsedData["interface_index"] = FindVarBindValue(trap.vars, "InterfaceIndex", "");
        trap.parsedData["interface_name"] = FindVarBindValue(trap.vars, "InterfaceName", "");
        trap.parsedData["flow_status"] = FindVarBindValue(trap.vars, "FlowStatus", "");
    }
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.5.25.41.3.6") != string::npos)
    {
        // hwIfFlowUp（接口流量Up告警）
        trap.parsedData["interface"] = FindVarBindValue(trap.vars, "Interface", "");
        trap.parsedData["interface_index"] = FindVarBindValue(trap.vars, "InterfaceIndex", "");
        trap.parsedData["interface_name"] = FindVarBindValue(trap.vars, "InterfaceName", "");
        trap.parsedData["flow_status"] = FindVarBindValue(trap.vars, "FlowStatus", "");
    }
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.5.25.41.4.5") != string::npos)
    {
        // hwIfMonitorInputRateRising（接口接收流量带宽利用率超过阈值告警）
        trap.parsedData["interface"] = FindVarBindValue(trap.vars, "Interface", "");
        trap.parsedData["interface_name"] = FindVarBindValue(trap.vars, "InterfaceName", "");
        trap.parsedData["bandwidth_usage"] = FindVarBindValue(trap.vars, "BandWidthUsage", "");
        trap.parsedData["trap_threshold"] = FindVarBindValue(trap.vars, "TrapThreshold", "");
    }
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.5.25.41.4.6") != string::npos)
    {
        // hwIfMonitorInputRateResume（接口接收流量带宽利用率恢复告警）
        trap.parsedData["interface"] = FindVarBindValue(trap.vars, "Interface", "");
        trap.parsedData["interface_name"] = FindVarBindValue(trap.vars, "InterfaceName", "");
        trap.parsedData["bandwidth_usage"] = FindVarBindValue(trap.vars, "BandWidthUsage", "");
        trap.parsedData["trap_threshold"] = FindVarBindValue(trap.vars, "TrapThreshold", "");
    }
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.5.25.41.4.7") != string::npos)
    {
        // hwIfMonitorOutputRateRising（接口发送流量带宽利用率超过阈值告警）
        trap.parsedData["interface"] = FindVarBindValue(trap.vars, "Interface", "");
        trap.parsedData["interface_name"] = FindVarBindValue(trap.vars, "InterfaceName", "");
        trap.parsedData["bandwidth_usage"] = FindVarBindValue(trap.vars, "BandWidthUsage", "");
        trap.parsedData["trap_threshold"] = FindVarBindValue(trap.vars, "TrapThreshold", "");
    }
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.5.25.41.4.8") != string::npos)
    {
        // hwIfMonitorOutputRateResume（接口发送流量带宽利用率恢复告警）
        trap.parsedData["interface"] = FindVarBindValue(trap.vars, "Interface", "");
        trap.parsedData["interface_name"] = FindVarBindValue(trap.vars, "InterfaceName", "");
        trap.parsedData["bandwidth_usage"] = FindVarBindValue(trap.vars, "BandWidthUsage", "");
        trap.parsedData["trap_threshold"] = FindVarBindValue(trap.vars, "TrapThreshold", "");
    }
    
    // ========== ENTITY模块Trap（实体/设备相关）==========
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.5.25.219.2.10.13") != string::npos ||
             trap.trapOid.find("1.3.6.1.4.1.2011.5.25.219.2.10.14") != string::npos ||
             trap.trapOid.find("1.3.6.1.4.1.2011.5.25.219.2.10.15") != string::npos)
    {
        // 单板温度告警/恢复/危险告警
        trap.parsedData["index"] = FindVarBindValue(trap.vars, "Index", "");
        trap.parsedData["entry_physical_index"] = FindVarBindValue(trap.vars, "EntryPhysicalIndex", "");
        trap.parsedData["physical_name"] = FindVarBindValue(trap.vars, "PhysicalName", "");
        trap.parsedData["entity_threshold_type"] = FindVarBindValue(trap.vars, "EntityThresholdType", "");
        trap.parsedData["entity_threshold_value"] = FindVarBindValue(trap.vars, "EntityThresholdValue", "");
        trap.parsedData["entity_threshold_current"] = FindVarBindValue(trap.vars, "EntityThresholdCurrent", "");
        trap.parsedData["entity_trap_fault_id"] = FindVarBindValue(trap.vars, "EntityTrapFaultID", "");
    }
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.5.25.219.2.14.1") != string::npos ||
             trap.trapOid.find("1.3.6.1.4.1.2011.5.25.219.2.14.2") != string::npos)
    {
        // CPU占用率超过阈值/恢复告警
        trap.parsedData["index"] = FindVarBindValue(trap.vars, "Index", "");
        trap.parsedData["hw_entity_physical_index"] = FindVarBindValue(trap.vars, "HwEntityPhysicalIndex", "");
        trap.parsedData["physical_name"] = FindVarBindValue(trap.vars, "PhysicalName", "");
        trap.parsedData["entity_threshold_type"] = FindVarBindValue(trap.vars, "EntityThresholdType", "");
        trap.parsedData["entity_threshold_warning"] = FindVarBindValue(trap.vars, "EntityThresholdWarning", "");
        trap.parsedData["entity_threshold_current"] = FindVarBindValue(trap.vars, "EntityThresholdCurrent", "");
        trap.parsedData["entity_trap_fault_id"] = FindVarBindValue(trap.vars, "EntityTrapFaultID", "");
    }
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.5.25.219.2.14.5") != string::npos ||
             trap.trapOid.find("1.3.6.1.4.1.2011.5.25.219.2.14.6") != string::npos)
    {
        // 转发CPU占用率超过阈值/恢复告警
        trap.parsedData["fwd_core_index"] = FindVarBindValue(trap.vars, "FwdCoreIndex", "");
        trap.parsedData["hw_entity_physical_index"] = FindVarBindValue(trap.vars, "HwEntityPhysicalIndex", "");
        trap.parsedData["physical_name"] = FindVarBindValue(trap.vars, "PhysicalName", "");
        trap.parsedData["entity_threshold_type"] = FindVarBindValue(trap.vars, "EntityThresholdType", "");
        trap.parsedData["entity_threshold_warning"] = FindVarBindValue(trap.vars, "EntityThresholdWarning", "");
        trap.parsedData["entity_threshold_current"] = FindVarBindValue(trap.vars, "EntityThresholdCurrent", "");
        trap.parsedData["entity_trap_fault_id"] = FindVarBindValue(trap.vars, "EntityTrapFaultID", "");
    }
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.5.25.219.2.15.1") != string::npos ||
             trap.trapOid.find("1.3.6.1.4.1.2011.5.25.219.2.15.2") != string::npos)
    {
        // 系统内存占用率超过阈值/恢复告警
        trap.parsedData["index"] = FindVarBindValue(trap.vars, "Index", "");
        trap.parsedData["hw_entity_physical_index"] = FindVarBindValue(trap.vars, "HwEntityPhysicalIndex", "");
        trap.parsedData["physical_name"] = FindVarBindValue(trap.vars, "PhysicalName", "");
        trap.parsedData["entity_threshold_type"] = FindVarBindValue(trap.vars, "EntityThresholdType", "");
        trap.parsedData["entity_threshold_warning"] = FindVarBindValue(trap.vars, "EntityThresholdWarning", "");
        trap.parsedData["entity_threshold_current"] = FindVarBindValue(trap.vars, "EntityThresholdCurrent", "");
        trap.parsedData["entity_trap_fault_id"] = FindVarBindValue(trap.vars, "EntityTrapFaultID", "");
    }
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.5.25.219.2.24.1") != string::npos ||
             trap.trapOid.find("1.3.6.1.4.1.2011.5.25.219.2.24.2") != string::npos)
    {
        // 磁盘利用率超过阈值/恢复告警
        trap.parsedData["hw_entity_physical_index"] = FindVarBindValue(trap.vars, "HwEntityPhysicalIndex", "");
        trap.parsedData["physical_name"] = FindVarBindValue(trap.vars, "PhysicalName", "");
        trap.parsedData["entity_threshold_type"] = FindVarBindValue(trap.vars, "EntityThresholdType", "");
        trap.parsedData["entity_threshold_warning"] = FindVarBindValue(trap.vars, "EntityThresholdWarning", "");
        trap.parsedData["entity_threshold_current"] = FindVarBindValue(trap.vars, "EntityThresholdCurrent", "");
        trap.parsedData["entity_trap_fault_id"] = FindVarBindValue(trap.vars, "EntityTrapFaultID", "");
    }
    
    // ========== 标准SNMP通用Trap ==========
    else if (trap.trapOid.find("1.3.6.1.6.3.1.1.5.3") != string::npos ||
             trap.trapOid.find("linkDown") != string::npos)
    {
        // linkDown（网络接口断开告警）
        trap.parsedData["if_index"] = FindVarBindValue(trap.vars, "ifIndex", "");
        trap.parsedData["if_admin_status"] = FindVarBindValue(trap.vars, "ifAdminStatus", "");
        trap.parsedData["if_oper_status"] = FindVarBindValue(trap.vars, "ifOperStatus", "");
        trap.parsedData["if_desc"] = FindVarBindValue(trap.vars, "ifDescr", "");
    }
    else if (trap.trapOid.find("1.3.6.1.6.3.1.1.5.4") != string::npos ||
             trap.trapOid.find("linkUp") != string::npos)
    {
        // linkUp（网络接口连接告警）
        trap.parsedData["if_index"] = FindVarBindValue(trap.vars, "ifIndex", "");
        trap.parsedData["if_admin_status"] = FindVarBindValue(trap.vars, "ifAdminStatus", "");
        trap.parsedData["if_oper_status"] = FindVarBindValue(trap.vars, "ifOperStatus", "");
        trap.parsedData["if_desc"] = FindVarBindValue(trap.vars, "ifDescr", "");
    }
    
    // ========== 无线用户数告警Trap ==========
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.12.3.1.7") != string::npos ||
             trap.trapOid.find("1.3.6.1.4.1.2011.6.139.12.3.1.8") != string::npos ||
             trap.trapOid.find("1.3.6.1.4.1.2011.6.139.12.3.1.9") != string::npos ||
             trap.trapOid.find("1.3.6.1.4.1.2011.6.139.12.3.1.10") != string::npos)
    {
        // 用户数告警Trap（达到告警阈值/恢复/达到最大值/从最大值恢复）
        string maxNumStr = FindVarBindValue(trap.vars, "MaxStaNum", "");
        string curNumStr = FindVarBindValue(trap.vars, "CurStaNum", "");
        
        if (!maxNumStr.empty())
            trap.parsedData["max_sta_num"] = maxNumStr;
        if (!curNumStr.empty())
            trap.parsedData["cur_sta_num"] = curNumStr;
        
        // 计算使用率百分比
        if (!maxNumStr.empty() && !curNumStr.empty())
        {
            int maxNum = atoi(maxNumStr.c_str());
            int curNum = atoi(curNumStr.c_str());
            if (maxNum > 0)
            {
                int usagePercent = (curNum * 100) / maxNum;
                char percentStr[16];
                snprintf(percentStr, sizeof(percentStr), "%d", usagePercent);
                trap.parsedData["usage_percent"] = string(percentStr);
            }
        }
    }
    
    // ========== AP通讯故障告警Trap ==========
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.1") != string::npos)
    {
        // AP通讯故障告警
        trap.parsedData["ap_mac"] = FindVarBindValue(trap.vars, "APMAC", "");
        trap.parsedData["ap_type"] = FindVarBindValue(trap.vars, "APType", "");
        trap.parsedData["ap_name"] = FindVarBindValue(trap.vars, "APName", "");
        trap.parsedData["ap_fault_time"] = FindVarBindValue(trap.vars, "APFAULTTIME", "");
        trap.parsedData["ap_id"] = FindVarBindValue(trap.vars, "APID", "");
    }
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.2") != string::npos)
    {
        // AP通讯故障恢复告警
        trap.parsedData["ap_mac"] = FindVarBindValue(trap.vars, "APMAC", "");
        trap.parsedData["ap_type"] = FindVarBindValue(trap.vars, "APType", "");
        trap.parsedData["ap_name"] = FindVarBindValue(trap.vars, "APName", "");
        trap.parsedData["ap_id"] = FindVarBindValue(trap.vars, "APID", "");
    }
    
    // ========== AP冷启动/热启动Trap ==========
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.30") != string::npos ||
             trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.31") != string::npos)
    {
        // AP冷启动/恢复告警
        trap.parsedData["ap_mac"] = FindVarBindValue(trap.vars, "APMAC", "");
        trap.parsedData["ap_type"] = FindVarBindValue(trap.vars, "AP TYPE", "");
        trap.parsedData["ap_name"] = FindVarBindValue(trap.vars, "AP Sys Name", "");
        trap.parsedData["ap_sys_time"] = FindVarBindValue(trap.vars, "AP Sys Time", "");
        trap.parsedData["ap_alarm_name"] = FindVarBindValue(trap.vars, "AP Alarm name", "");
        trap.parsedData["ap_id"] = FindVarBindValue(trap.vars, "APID", "");
    }
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.32") != string::npos ||
             trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.33") != string::npos)
    {
        // AP热启动/恢复告警
        trap.parsedData["ap_mac"] = FindVarBindValue(trap.vars, "APMAC", "");
        trap.parsedData["ap_type"] = FindVarBindValue(trap.vars, "APType", "");
        trap.parsedData["ap_name"] = FindVarBindValue(trap.vars, "APName", "");
        trap.parsedData["ap_occur_time"] = FindVarBindValue(trap.vars, "APOccurTime", "");
        trap.parsedData["notify_name"] = FindVarBindValue(trap.vars, "NotifyName", "");
        trap.parsedData["ap_id"] = FindVarBindValue(trap.vars, "APID", "");
    }
    
    // ========== AP CPU/内存利用率告警Trap ==========
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.7") != string::npos ||
             trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.8") != string::npos)
    {
        // AP CPU利用率过高/恢复告警
        trap.parsedData["ap_mac"] = FindVarBindValue(trap.vars, "APMAC", "");
        trap.parsedData["ap_name"] = FindVarBindValue(trap.vars, "APName", "");
        trap.parsedData["cpu_rate"] = FindVarBindValue(trap.vars, "ApCpuRate", "");
        trap.parsedData["ap_id"] = FindVarBindValue(trap.vars, "APID", "");
    }
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.9") != string::npos ||
             trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.10") != string::npos)
    {
        // AP内存利用率过高/恢复告警
        trap.parsedData["ap_mac"] = FindVarBindValue(trap.vars, "APMAC", "");
        trap.parsedData["ap_name"] = FindVarBindValue(trap.vars, "APName", "");
        trap.parsedData["mem_rate"] = FindVarBindValue(trap.vars, "ApMemRate", "");
        trap.parsedData["ap_id"] = FindVarBindValue(trap.vars, "APID", "");
    }
    
    // ========== AP性能告警Trap (139.14.1.2系列) ==========
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.14.1.2") != string::npos)
    {
        // AP性能告警（CPU/内存/流量）
        trap.parsedData["ap_mac"] = FindVarBindValue(trap.vars, "APMAC", "");
        trap.parsedData["ap_name"] = FindVarBindValue(trap.vars, "APName", "");
        trap.parsedData["ap_id"] = FindVarBindValue(trap.vars, "APID", "");
        // 根据具体子OID提取不同的性能指标
        if (trap.trapOid.find(".1.2.1") != string::npos)
        {
            // CPU使用率告警
            trap.parsedData["cpu_rate"] = FindVarBindValue(trap.vars, "CpuRate", "");
            trap.parsedData["cpu_usage"] = FindVarBindValue(trap.vars, "CpuUsage", "");
        }
        else if (trap.trapOid.find(".1.2.2") != string::npos)
        {
            // 内存使用率告警
            trap.parsedData["mem_rate"] = FindVarBindValue(trap.vars, "MemRate", "");
            trap.parsedData["mem_usage"] = FindVarBindValue(trap.vars, "MemUsage", "");
        }
        else if (trap.trapOid.find(".1.2.3") != string::npos)
        {
            // 流量告警
            trap.parsedData["traffic_rate"] = FindVarBindValue(trap.vars, "TrafficRate", "");
            trap.parsedData["traffic_usage"] = FindVarBindValue(trap.vars, "TrafficUsage", "");
        }
    }
    
    // ========== AP用户数已满告警Trap ==========
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.11") != string::npos)
    {
        // AP用户数已满告警
        trap.parsedData["ap_mac"] = FindVarBindValue(trap.vars, "APMAC", "");
        trap.parsedData["ap_name"] = FindVarBindValue(trap.vars, "APName", "");
        trap.parsedData["fail_cause"] = FindVarBindValue(trap.vars, "FailCause", "");
        trap.parsedData["permit_num"] = FindVarBindValue(trap.vars, "PermitNum", "");
        trap.parsedData["ap_id"] = FindVarBindValue(trap.vars, "APID", "");
    }
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.12") != string::npos)
    {
        // AP用户数已满恢复告警
        trap.parsedData["ap_mac"] = FindVarBindValue(trap.vars, "APMAC", "");
        trap.parsedData["ap_name"] = FindVarBindValue(trap.vars, "APName", "");
        trap.parsedData["fail_cause"] = FindVarBindValue(trap.vars, "FailCause", "");
        trap.parsedData["current_num"] = FindVarBindValue(trap.vars, "CurrentNum", "");
        trap.parsedData["ap_id"] = FindVarBindValue(trap.vars, "APID", "");
    }
    
    // ========== AP温度告警Trap ==========
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.17") != string::npos ||
             trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.18") != string::npos ||
             trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.19") != string::npos ||
             trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.20") != string::npos)
    {
        // AP温度过低/过高告警
        trap.parsedData["ap_mac"] = FindVarBindValue(trap.vars, "APMAC", "");
        trap.parsedData["ap_name"] = FindVarBindValue(trap.vars, "APName", "");
        trap.parsedData["temperature"] = FindVarBindValue(trap.vars, "Temperature", "");
        trap.parsedData["ap_id"] = FindVarBindValue(trap.vars, "APID", "");
        trap.parsedData["temperature_type"] = FindVarBindValue(trap.vars, "TemperaturType", "");
    }
    
    // ========== AP磁盘空间告警Trap ==========
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.104") != string::npos ||
             trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.105") != string::npos)
    {
        // AP磁盘空间满/恢复告警
        trap.parsedData["ap_mac"] = FindVarBindValue(trap.vars, "APMAC", "");
        trap.parsedData["ap_name"] = FindVarBindValue(trap.vars, "APName", "");
        trap.parsedData["ap_disk_threshold_warning"] = FindVarBindValue(trap.vars, "APDiskThresholdWarning", "");
        trap.parsedData["ap_disk_threshold_current"] = FindVarBindValue(trap.vars, "APDiskThresholdCurrent", "");
        trap.parsedData["ap_id"] = FindVarBindValue(trap.vars, "APID", "");
    }
    
    // ========== AP供电不足受限模式告警Trap ==========
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.106") != string::npos ||
             trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.107") != string::npos)
    {
        // AP供电不足受限模式/恢复告警
        trap.parsedData["ap_mac"] = FindVarBindValue(trap.vars, "APMAC", "");
        trap.parsedData["ap_name"] = FindVarBindValue(trap.vars, "APName", "");
        trap.parsedData["power_work_mode"] = FindVarBindValue(trap.vars, "PowerWorkMode", "");
        trap.parsedData["expect_power_work_mode"] = FindVarBindValue(trap.vars, "ExpectPowerWorkMode", "");
        trap.parsedData["ap_id"] = FindVarBindValue(trap.vars, "APID", "");
    }
    
    // ========== AP在线个数告警Trap ==========
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.109") != string::npos ||
             trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.110") != string::npos)
    {
        // AP在线个数达到最大规格/恢复告警（无VarBinds）
    }
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.111") != string::npos ||
             trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.112") != string::npos)
    {
        // AP在线个数达到告警阈值/恢复告警
        trap.parsedData["ap_max_num"] = FindVarBindValue(trap.vars, "APMaxNum", "");
    }
    
    // ========== AP电源模块告警Trap ==========
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.119") != string::npos ||
             trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.120") != string::npos)
    {
        // AP电源拔出/插入告警
        trap.parsedData["ap_mac"] = FindVarBindValue(trap.vars, "APMAC", "");
        trap.parsedData["ap_name"] = FindVarBindValue(trap.vars, "APName", "");
        trap.parsedData["ap_id"] = FindVarBindValue(trap.vars, "APID", "");
        trap.parsedData["ap_power_id"] = FindVarBindValue(trap.vars, "APPowerId", "");
    }
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.121") != string::npos ||
             trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.122") != string::npos)
    {
        // AP电源无法使用/恢复告警
        trap.parsedData["ap_mac"] = FindVarBindValue(trap.vars, "APMAC", "");
        trap.parsedData["ap_name"] = FindVarBindValue(trap.vars, "APName", "");
        trap.parsedData["ap_id"] = FindVarBindValue(trap.vars, "APID", "");
        trap.parsedData["ap_power_id"] = FindVarBindValue(trap.vars, "APPowerId", "");
        trap.parsedData["fault_id"] = FindVarBindValue(trap.vars, "FaultID", "");
        trap.parsedData["fault_reason_desc"] = FindVarBindValue(trap.vars, "FaultReasonDesc", "");
    }
    
    // ========== 射频信号环境告警Trap ==========
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.16.1.1.1.1") != string::npos)
    {
        // AP信道变更告警
        trap.parsedData["ap_mac"] = FindVarBindValue(trap.vars, "APMAC", "");
        trap.parsedData["radio_id"] = FindVarBindValue(trap.vars, "RadioID", "");
        trap.parsedData["ap_name"] = FindVarBindValue(trap.vars, "APName", "");
        trap.parsedData["channel1"] = FindVarBindValue(trap.vars, "Channel1", "");
        trap.parsedData["channel2"] = FindVarBindValue(trap.vars, "Channel2", "");
        trap.parsedData["cause_id"] = FindVarBindValue(trap.vars, "CauseId", "");
        trap.parsedData["cause_str"] = FindVarBindValue(trap.vars, "CauseStr", "");
        trap.parsedData["pre_channel1"] = FindVarBindValue(trap.vars, "PreChannel1", "");
        trap.parsedData["pre_channel2"] = FindVarBindValue(trap.vars, "PreChannel2", "");
        trap.parsedData["ap_id"] = FindVarBindValue(trap.vars, "APID", "");
        trap.parsedData["new_channel_has_radar_channel"] = FindVarBindValue(trap.vars, "NewChannelHasRadarChannel", "");
    }
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.16.1.1.1.2") != string::npos)
    {
        // 射频信号环境恶化告警
        trap.parsedData["ap_mac"] = FindVarBindValue(trap.vars, "APMAC", "");
        trap.parsedData["radio_id"] = FindVarBindValue(trap.vars, "RadioID", "");
        trap.parsedData["ap_name"] = FindVarBindValue(trap.vars, "APName", "");
        trap.parsedData["per"] = FindVarBindValue(trap.vars, "PER", "");
        trap.parsedData["retransmission_rate"] = FindVarBindValue(trap.vars, "RetransmissionRate", "");
        trap.parsedData["ap_id"] = FindVarBindValue(trap.vars, "APID", "");
        trap.parsedData["noise_floor"] = FindVarBindValue(trap.vars, "NoiseFloor", "");
        trap.parsedData["reason"] = FindVarBindValue(trap.vars, "Reason", "");
        trap.parsedData["bad_channel"] = FindVarBindValue(trap.vars, "BadChannel", "");
        trap.parsedData["interference_rate"] = FindVarBindValue(trap.vars, "InterferenceRate", "");
    }
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.16.1.1.1.3") != string::npos)
    {
        // 射频信号环境恢复告警
        trap.parsedData["ap_mac"] = FindVarBindValue(trap.vars, "APMAC", "");
        trap.parsedData["radio_id"] = FindVarBindValue(trap.vars, "RadioID", "");
        trap.parsedData["ap_name"] = FindVarBindValue(trap.vars, "APName", "");
        trap.parsedData["ap_id"] = FindVarBindValue(trap.vars, "APID", "");
    }
    
    // ========== 同频/邻频AP干扰告警Trap ==========
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.16.1.1.1.5") != string::npos ||
             trap.trapOid.find("1.3.6.1.4.1.2011.6.139.16.1.1.1.6") != string::npos)
    {
        // 同频AP干扰告警/清除
        trap.parsedData["ap_mac"] = FindVarBindValue(trap.vars, "APMAC", "");
        trap.parsedData["radio_id"] = FindVarBindValue(trap.vars, "RadioID", "");
        trap.parsedData["ap_name"] = FindVarBindValue(trap.vars, "APName", "");
        trap.parsedData["ap_channel"] = FindVarBindValue(trap.vars, "APChannel", "");
        trap.parsedData["ap_id"] = FindVarBindValue(trap.vars, "APID", "");
        trap.parsedData["bss_id"] = FindVarBindValue(trap.vars, "BssId", "");
        trap.parsedData["rssi_percent"] = FindVarBindValue(trap.vars, "RSSI percent", "");
    }
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.16.1.1.1.7") != string::npos ||
             trap.trapOid.find("1.3.6.1.4.1.2011.6.139.16.1.1.1.8") != string::npos)
    {
        // 邻频AP干扰告警/清除
        trap.parsedData["ap_mac"] = FindVarBindValue(trap.vars, "APMAC", "");
        trap.parsedData["radio_id"] = FindVarBindValue(trap.vars, "RadioID", "");
        trap.parsedData["ap_name"] = FindVarBindValue(trap.vars, "APName", "");
        trap.parsedData["ap_channel"] = FindVarBindValue(trap.vars, "APChannel", "");
        trap.parsedData["ap_id"] = FindVarBindValue(trap.vars, "APID", "");
        trap.parsedData["intf_bss_id"] = FindVarBindValue(trap.vars, "IntfBssId", "");
        trap.parsedData["intf_chnl"] = FindVarBindValue(trap.vars, "IntfChnl", "");
        trap.parsedData["rssi_percent"] = FindVarBindValue(trap.vars, "RSSI percent", "");
    }
    
    // ========== 终端干扰告警Trap ==========
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.16.1.1.1.9") != string::npos ||
             trap.trapOid.find("1.3.6.1.4.1.2011.6.139.16.1.1.1.10") != string::npos)
    {
        // 终端干扰告警/清除
        trap.parsedData["ap_mac"] = FindVarBindValue(trap.vars, "APMAC", "");
        trap.parsedData["radio_id"] = FindVarBindValue(trap.vars, "RadioID", "");
        trap.parsedData["ap_name"] = FindVarBindValue(trap.vars, "APName", "");
        trap.parsedData["ap_id"] = FindVarBindValue(trap.vars, "APID", "");
    }
    
    // ========== 用户IP地址与网关地址冲突告警Trap ==========
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.18.1.1.1.12") != string::npos)
    {
        // 用户IP地址与网关地址冲突告警
        trap.parsedData["ap_mac"] = FindVarBindValue(trap.vars, "APMAC", "");
        trap.parsedData["sta_mac"] = FindVarBindValue(trap.vars, "StaMac", "");
        trap.parsedData["ap_name"] = FindVarBindValue(trap.vars, "APName", "");
        trap.parsedData["ip_address"] = FindVarBindValue(trap.vars, "IPAddress", "");
        trap.parsedData["ap_id"] = FindVarBindValue(trap.vars, "APID", "");
    }
    
    // ========== WIDS非Wi-Fi设备检测告警Trap ==========
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.15.1.1.15") != string::npos)
    {
        // WIDS检测到非Wi-Fi设备告警
        // 根据SNMP Trap格式，设备数量通常在VarBinds中
        // 尝试多种可能的字段名匹配
        trap.parsedData["device_count"] = FindVarBindValue(trap.vars, "Device count", "");
        if (trap.parsedData["device_count"].empty())
        {
            trap.parsedData["device_count"] = FindVarBindValue(trap.vars, "DeviceCount", "");
        }
        if (trap.parsedData["device_count"].empty())
        {
            trap.parsedData["device_count"] = FindVarBindValue(trap.vars, "device_count", "");
        }
        if (trap.parsedData["device_count"].empty())
        {
            trap.parsedData["device_count"] = FindVarBindValue(trap.vars, "count", "");
        }
        
        // 如果通过字段名找不到，尝试遍历VarBinds查找INTEGER类型的值
        // 通常设备数量是一个INTEGER类型的VarBind
        if (trap.parsedData["device_count"].empty())
        {
            // 遍历所有VarBinds，查找可能的设备数量
            // 通常设备数量是正整数，且不会太大（比如1-1000）
            for (map<string, string>::const_iterator it = trap.vars.begin(); 
                 it != trap.vars.end(); ++it)
            {
                string oid = it->first;
                string value = it->second;
                
                // 跳过已知的OID（Trap OID、Enterprise OID、时间戳等）
                if (oid.find("1.3.6.1.6.3.1.1.4") != string::npos ||
                    oid.find("1.3.6.1.2.1.1.3") != string::npos ||
                    oid.find("1.3.6.1.2.1.1.1") != string::npos ||
                    oid.find("1.3.6.1.2.1.1.2") != string::npos)
                {
                    continue;
                }
                
                // 检查值是否为纯数字（可能是设备数量）
                if (!value.empty())
                {
                    bool isNumeric = true;
                    for (size_t i = 0; i < value.length(); ++i)
                    {
                        if (!isdigit(value[i]))
                        {
                            isNumeric = false;
                            break;
                        }
                    }
                    
                    // 如果是数字且在合理范围内（1-1000），可能是设备数量
                    if (isNumeric)
                    {
                        int count = atoi(value.c_str());
                        if (count > 0 && count <= 1000)
                        {
                            trap.parsedData["device_count"] = value;
                            // TrapInfoLog("WIDS非Wi-Fi设备告警: 从OID[%s]提取到设备数量=%s", 
                            //            oid.c_str(), value.c_str());
                            break;  // 找到第一个合理的数字值
                        }
                    }
                }
            }
        }
        
        // 如果仍然找不到设备数量，记录所有VarBinds用于调试
        // if (trap.parsedData["device_count"].empty())
        // {
        //     TrapInfoLog("WIDS非Wi-Fi设备告警: 未找到设备数量，VarBinds数量=%zu", trap.vars.size());
        //     int count = 0;
        //     for (map<string, string>::const_iterator it = trap.vars.begin(); 
        //          it != trap.vars.end() && count < 10; ++it, ++count)
        //     {
        //         TrapInfoLog("  VarBind[%d]: OID=%s, Value=%s", count, it->first.c_str(), it->second.c_str());
        //     }
        // }
        
        // 提取OID字段（MIB节点号）
        trap.parsedData["oid"] = FindVarBindValue(trap.vars, "oid", "");
        if (trap.parsedData["oid"].empty())
        {
            trap.parsedData["oid"] = trap.trapOid; // 如果没有单独的oid字段，使用trapOid
        }
    }
    
    // ========== 通用处理：按OID前缀分类处理 ==========
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.") != string::npos)
    {
        // AP相关Trap (139.13.1.1.x系列) - 通用处理
        trap.parsedData["ap_mac"] = FindVarBindValue(trap.vars, "APMAC", "");
        trap.parsedData["ap_name"] = FindVarBindValue(trap.vars, "APName", "");
        trap.parsedData["ap_id"] = FindVarBindValue(trap.vars, "APID", "");
        trap.parsedData["ap_type"] = FindVarBindValue(trap.vars, "APType", "");
        // 提取其他可能的字段
        trap.parsedData["temperature"] = FindVarBindValue(trap.vars, "Temperature", "");
        trap.parsedData["cpu_rate"] = FindVarBindValue(trap.vars, "CpuRate", "");
        trap.parsedData["mem_rate"] = FindVarBindValue(trap.vars, "MemRate", "");
        trap.parsedData["fail_cause"] = FindVarBindValue(trap.vars, "FailCause", "");
    }
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.14.") != string::npos)
    {
        // WLAN性能事件Trap (139.14.x系列) - 通用处理
        trap.parsedData["ap_mac"] = FindVarBindValue(trap.vars, "APMAC", "");
        trap.parsedData["ap_name"] = FindVarBindValue(trap.vars, "APName", "");
        trap.parsedData["ap_id"] = FindVarBindValue(trap.vars, "APID", "");
        trap.parsedData["cpu_rate"] = FindVarBindValue(trap.vars, "CpuRate", "");
        trap.parsedData["cpu_usage"] = FindVarBindValue(trap.vars, "CpuUsage", "");
        trap.parsedData["mem_rate"] = FindVarBindValue(trap.vars, "MemRate", "");
        trap.parsedData["mem_usage"] = FindVarBindValue(trap.vars, "MemUsage", "");
        trap.parsedData["traffic_rate"] = FindVarBindValue(trap.vars, "TrafficRate", "");
        trap.parsedData["traffic_usage"] = FindVarBindValue(trap.vars, "TrafficUsage", "");
    }
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.16.1.1.1.") != string::npos)
    {
        // 射频相关Trap (139.16.1.1.1.x系列) - 通用处理
        trap.parsedData["ap_mac"] = FindVarBindValue(trap.vars, "APMAC", "");
        trap.parsedData["ap_name"] = FindVarBindValue(trap.vars, "APName", "");
        trap.parsedData["ap_id"] = FindVarBindValue(trap.vars, "APID", "");
        trap.parsedData["radio_id"] = FindVarBindValue(trap.vars, "RadioID", "");
        trap.parsedData["ap_channel"] = FindVarBindValue(trap.vars, "APChannel", "");
        trap.parsedData["intf_bss_id"] = FindVarBindValue(trap.vars, "IntfBssId", "");
        trap.parsedData["intf_chnl"] = FindVarBindValue(trap.vars, "IntfChnl", "");
    }
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.18.") != string::npos)
    {
        // 终端相关Trap (139.18.x系列) - 通用处理
        trap.parsedData["ap_mac"] = FindVarBindValue(trap.vars, "APMAC", "");
        trap.parsedData["ap_name"] = FindVarBindValue(trap.vars, "APName", "");
        trap.parsedData["ap_id"] = FindVarBindValue(trap.vars, "APID", "");
        trap.parsedData["sta_mac"] = FindVarBindValue(trap.vars, "StaMac", "");
        trap.parsedData["ip_address"] = FindVarBindValue(trap.vars, "IPAddress", "");
    }
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.6.139.12.") != string::npos)
    {
        // 统计相关Trap (139.12.x系列) - 通用处理
        trap.parsedData["ap_mac"] = FindVarBindValue(trap.vars, "APMAC", "");
        trap.parsedData["ap_name"] = FindVarBindValue(trap.vars, "APName", "");
        trap.parsedData["ap_id"] = FindVarBindValue(trap.vars, "APID", "");
        trap.parsedData["max_sta_num"] = FindVarBindValue(trap.vars, "MaxStaNum", "");
        trap.parsedData["cur_sta_num"] = FindVarBindValue(trap.vars, "CurStaNum", "");
    }
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.5.25.219.") != string::npos)
    {
        // 实体/设备相关Trap (5.25.219.x系列) - 通用处理
        trap.parsedData["index"] = FindVarBindValue(trap.vars, "Index", "");
        trap.parsedData["physical_name"] = FindVarBindValue(trap.vars, "PhysicalName", "");
        trap.parsedData["entity_threshold_type"] = FindVarBindValue(trap.vars, "EntityThresholdType", "");
        trap.parsedData["entity_threshold_value"] = FindVarBindValue(trap.vars, "EntityThresholdValue", "");
        trap.parsedData["entity_threshold_current"] = FindVarBindValue(trap.vars, "EntityThresholdCurrent", "");
        trap.parsedData["entity_threshold_warning"] = FindVarBindValue(trap.vars, "EntityThresholdWarning", "");
    }
    else if (trap.trapOid.find("1.3.6.1.4.1.2011.5.25.41.") != string::npos)
    {
        // 网络接口相关Trap (5.25.41.x系列) - 通用处理
        trap.parsedData["interface"] = FindVarBindValue(trap.vars, "Interface", "");
        trap.parsedData["interface_index"] = FindVarBindValue(trap.vars, "InterfaceIndex", "");
        trap.parsedData["interface_name"] = FindVarBindValue(trap.vars, "InterfaceName", "");
        trap.parsedData["flow_status"] = FindVarBindValue(trap.vars, "FlowStatus", "");
        trap.parsedData["bandwidth_usage"] = FindVarBindValue(trap.vars, "BandWidthUsage", "");
        trap.parsedData["trap_threshold"] = FindVarBindValue(trap.vars, "TrapThreshold", "");
    }
    else if (trap.trapOid.find("1.3.6.1.4.1.2011") != string::npos)
    {
        // 其他华为设备Trap - 通用处理
        // 尝试提取通用的AP信息（如果存在）
        trap.parsedData["ap_mac"] = FindVarBindValue(trap.vars, "APMAC", "");
        trap.parsedData["ap_name"] = FindVarBindValue(trap.vars, "APName", "");
        trap.parsedData["ap_id"] = FindVarBindValue(trap.vars, "APID", "");
        // 尝试提取其他可能的字段
        trap.parsedData["interface"] = FindVarBindValue(trap.vars, "Interface", "");
        trap.parsedData["interface_name"] = FindVarBindValue(trap.vars, "InterfaceName", "");
        trap.parsedData["index"] = FindVarBindValue(trap.vars, "Index", "");
        trap.parsedData["physical_name"] = FindVarBindValue(trap.vars, "PhysicalName", "");
    }
    // ========== 其他未识别的Trap类型 ==========
    else
    {
        // 对于未识别的Trap类型，记录日志并保存原始VarBinds
        if (trap.trapType == "unknown" || trap.trapType == "huawei-unknown")
        {
            TrapInfoLog("未识别的Trap类型，OID=%s, 类型=%s, VarBinds数量=%zu", 
                       trap.trapOid.c_str(), trap.trapType.c_str(), trap.vars.size());
            
            // 记录前几个VarBinds的OID，便于分析
            int count = 0;
            for (map<string, string>::const_iterator it = trap.vars.begin(); 
                 it != trap.vars.end() && count < 5; ++it, ++count)
            {
                TrapInfoLog("  VarBind[%d]: OID=%s, Value=%s", 
                           count, it->first.c_str(), it->second.c_str());
            }
        }
        
        // 尝试提取通用的信息（如果存在）
        trap.parsedData["ap_mac"] = FindVarBindValue(trap.vars, "APMAC", "");
        trap.parsedData["ap_name"] = FindVarBindValue(trap.vars, "APName", "");
        trap.parsedData["ap_id"] = FindVarBindValue(trap.vars, "APID", "");
        trap.parsedData["interface"] = FindVarBindValue(trap.vars, "Interface", "");
        trap.parsedData["interface_name"] = FindVarBindValue(trap.vars, "InterfaceName", "");
    }
    
    // ========== 生成可读的告警消息和设备名称 ==========
    // 生成message（可读的中文告警信息）
    string message = GetTrapMessage(trap.trapType, trap.parsedData);
    trap.parsedData["message"] = message;
    
    // 生成device（设备名称：优先AP名称，其次AC名称）
    string deviceName = "";
    if (trap.parsedData.find("ap_name") != trap.parsedData.end() && 
        !trap.parsedData.at("ap_name").empty())
    {
        deviceName = trap.parsedData.at("ap_name");
    }
    else if (trap.parsedData.find("ap_id") != trap.parsedData.end() && 
             !trap.parsedData.at("ap_id").empty())
    {
        deviceName = "AP-" + trap.parsedData.at("ap_id");
    }
    else if (trap.parsedData.find("ap_mac") != trap.parsedData.end() && 
             !trap.parsedData.at("ap_mac").empty())
    {
        deviceName = trap.parsedData.at("ap_mac");
    }
    else if (!trap.hostname.empty())
    {
        deviceName = trap.hostname;  // 使用AC名称
    }
    else
    {
        deviceName = trap.sourceIp;  // 最后使用IP
    }
    trap.parsedData["device"] = deviceName;
    
    // 生成level（告警级别）
    string level = GetTrapLevel(trap.trapType);
    trap.parsedData["level"] = level;
    
    return 0;
}

// 生成可读的告警消息
static string GetTrapMessage(const string& trapType, const map<string, string>& parsedData)
{
    if (trapType == "ap-fault")
        return "AP通讯故障";
    else if (trapType == "ap-normal")
        return "AP通讯故障已恢复";
    else if (trapType == "ap-online")
        return "AP设备上线";
    else if (trapType == "ap-offline")
        return "AP设备下线";
    else if (trapType == "ap-cold-boot")
        return "AP冷启动";
    else if (trapType == "ap-hot-boot")
        return "AP热启动";
    else if (trapType == "sta-num-warning")
    {
        if (parsedData.find("max_sta_num") != parsedData.end() && 
            parsedData.find("cur_sta_num") != parsedData.end())
        {
            int maxNum = atoi(parsedData.at("max_sta_num").c_str());
            int curNum = atoi(parsedData.at("cur_sta_num").c_str());
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
        if (parsedData.find("cpu_rate") != parsedData.end())
        {
            char msg[128];
            snprintf(msg, sizeof(msg), "AP CPU利用率过高：%s%%", parsedData.at("cpu_rate").c_str());
            return string(msg);
        }
        return "AP CPU利用率过高";
    }
    else if (trapType == "ap-memory-overload")
    {
        if (parsedData.find("mem_rate") != parsedData.end())
        {
            char msg[128];
            snprintf(msg, sizeof(msg), "AP内存利用率过高：%s%%", parsedData.at("mem_rate").c_str());
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
    else if (trapType == "wlan-wids-event")
        return "WLAN WIDS事件";
    else if (trapType == "wlan-radio-event")
        return "WLAN射频事件";
    else if (trapType == "wlan-station-event")
        return "WLAN终端事件";
    else if (trapType == "huawei-entity-event")
        return "华为设备实体事件";
    else if (trapType == "huawei-ifnet-event")
        return "华为网络接口事件";
    
    // ========== 射频相关Trap ==========
    else if (trapType == "radio-channel-changed")
        return "射频信道变更";
    else if (trapType == "radio-signal-env-deterioration")
        return "射频信号环境恶化";
    else if (trapType == "radio-signal-env-resume")
        return "射频信号环境恢复";
    else if (trapType == "ap-co-interf-detected")
        return "检测到同频AP干扰";
    else if (trapType == "ap-co-interf-clear")
        return "同频AP干扰已清除";
    else if (trapType == "ap-neighbor-interf-detected")
        return "检测到邻频AP干扰";
    else if (trapType == "ap-neighbor-interf-clear")
        return "邻频AP干扰已清除";
    else if (trapType == "sta-interf-detected")
        return "检测到终端干扰";
    else if (trapType == "sta-interf-clear")
        return "终端干扰已清除";
    else if (trapType == "other-device-interf-detected")
        return "检测到其他设备干扰";
    else if (trapType == "other-device-interf-clear")
        return "其他设备干扰已清除";
    else if (trapType == "radio-max-sta-reach")
        return "射频最大用户数已满";
    else if (trapType == "radio-max-sta-clear")
        return "射频最大用户数已恢复";
    
    // ========== 其他AP相关Trap ==========
    else if (trapType == "ap-offline")
        return "AP设备下线";
    else if (trapType == "unauthorized-ap-exist")
        return "检测到未认证AP";
    else if (trapType == "unauthorized-ap-clear")
        return "未认证AP已清除";
    else if (trapType == "ap-cold-boot")
        return "AP冷启动";
    else if (trapType == "ap-cold-boot-restore")
        return "AP冷启动恢复";
    else if (trapType == "ap-hot-boot")
        return "AP热启动";
    else if (trapType == "ap-hot-boot-restore")
        return "AP热启动恢复";
    else if (trapType == "ap-sta-full-recover")
        return "AP用户数已满恢复";
    else if (trapType == "ap-optical-rx-power-too-high")
        return "AP光模块接收功率过高";
    else if (trapType == "ap-optical-rx-power-too-high-restore")
        return "AP光模块接收功率过高恢复";
    else if (trapType == "ap-optical-rx-power-too-low")
        return "AP光模块接收功率过低";
    else if (trapType == "ap-optical-rx-power-too-low-restore")
        return "AP光模块接收功率过低恢复";
    else if (trapType == "ap-optical-temp-too-high")
        return "AP光模块温度过高";
    else if (trapType == "ap-optical-temp-too-high-restore")
        return "AP光模块温度过高恢复";
    else if (trapType == "ap-optical-temp-too-low")
        return "AP光模块温度过低";
    else if (trapType == "ap-optical-temp-too-low-restore")
        return "AP光模块温度过低恢复";
    else if (trapType == "ap-optical-tx-power-too-high")
        return "AP光模块发送功率过高";
    else if (trapType == "ap-optical-tx-power-too-high-restore")
        return "AP光模块发送功率过高恢复";
    else if (trapType == "ap-optical-tx-power-too-low")
        return "AP光模块发送功率过低";
    else if (trapType == "ap-optical-tx-power-too-low-restore")
        return "AP光模块发送功率过低恢复";
    else if (trapType == "ap-sub-firmware-mismatch")
        return "AP子固件版本不匹配";
    else if (trapType == "ap-fan-invalid")
        return "AP风扇故障";
    else if (trapType == "ap-fan-invalid-restore")
        return "AP风扇故障恢复";
    else if (trapType == "ap-storage-dev-remove")
        return "AP存储卡拔出";
    else if (trapType == "ap-optical-invalid")
        return "AP光模块功能异常";
    else if (trapType == "ap-optical-invalid-restore")
        return "AP光模块功能异常恢复";
    else if (trapType == "ap-power-insufficient")
        return "AP供电不足";
    else if (trapType == "ap-power-insufficient-resume")
        return "AP供电不足恢复";
    else if (trapType == "port-vlan-secure-mac")
        return "端口安全MAC地址告警";
    else if (trapType == "ap-cable-snr-normal")
        return "AP网线质量正常";
    else if (trapType == "ap-cable-snr-abnormal")
        return "AP网线质量异常";
    else if (trapType == "ap-cable-snr-detect-not-support")
        return "AP网线质量检测不支持";
    else if (trapType == "ap-version-not-recommended")
        return "AP版本不配套";
    else if (trapType == "ap-version-not-recommended-restore")
        return "AP版本不配套恢复";
    else if (trapType == "ap-disk-overload-restore")
        return "AP磁盘空间满恢复";
    else if (trapType == "ap-power-limited-resume")
        return "AP供电不足受限模式恢复";
    else if (trapType == "ap-ip-conflict")
        return "AP IP地址冲突";
    else if (trapType == "ap-num-reach-max")
        return "AP在线个数达到最大值";
    else if (trapType == "ap-num-reach-max-resume")
        return "AP在线个数达到最大值恢复";
    else if (trapType == "ap-num-reach-warning")
        return "AP在线个数达到告警阈值";
    else if (trapType == "ap-num-reach-warning-resume")
        return "AP在线个数达到告警阈值恢复";
    else if (trapType == "ap-fan-remove")
        return "AP风扇模块拔出";
    else if (trapType == "ap-fan-insert")
        return "AP风扇模块插入";
    else if (trapType == "ap-power-remove")
        return "AP电源模块拔出";
    else if (trapType == "ap-power-insert")
        return "AP电源模块插入";
    else if (trapType == "ap-power-fail")
        return "AP电源模块故障";
    else if (trapType == "ap-power-fail-resume")
        return "AP电源模块故障恢复";
    else if (trapType == "ap-power-invalid")
        return "AP电源模块无效";
    else if (trapType == "ap-power-invalid-resume")
        return "AP电源模块无效恢复";
    else if (trapType == "ap-optical-voltage-too-high")
        return "AP光模块电压过高";
    else if (trapType == "ap-optical-voltage-too-high-restore")
        return "AP光模块电压过高恢复";
    else if (trapType == "ap-optical-voltage-too-low")
        return "AP光模块电压过低";
    else if (trapType == "ap-optical-voltage-too-low-restore")
        return "AP光模块电压过低恢复";
    else if (trapType == "ap-optical-current-too-high-restore")
        return "AP光模块电流过高恢复";
    else if (trapType == "ap-optical-current-too-low")
        return "AP光模块电流过低";
    else if (trapType == "ap-optical-current-too-low-restore")
        return "AP光模块电流过低恢复";
    
    // ========== 网络接口相关Trap ==========
    else if (trapType == "if-flow-down")
        return "接口流量Down";
    else if (trapType == "if-flow-up")
        return "接口流量Up";
    else if (trapType == "if-input-rate-rising")
        return "接口接收流量带宽利用率超过阈值";
    else if (trapType == "if-input-rate-resume")
        return "接口接收流量带宽利用率恢复";
    else if (trapType == "if-output-rate-rising")
        return "接口发送流量带宽利用率超过阈值";
    else if (trapType == "if-output-rate-resume")
        return "接口发送流量带宽利用率恢复";
    
    // ========== 用户相关Trap ==========
    else if (trapType == "sta-num-warning-restore")
        return "用户数告警阈值恢复";
    else if (trapType == "sta-num-max-restore")
        return "用户数达到最大值恢复";
    else if (trapType == "station-ip-conflict")
        return "用户IP地址与网关地址冲突";
    
    // ========== WIDS相关Trap ==========
    else if (trapType == "wids-non-wifi-device")
    {
        // 根据设备数量生成更详细的消息
        if (parsedData.find("device_count") != parsedData.end() && 
            !parsedData.at("device_count").empty())
        {
            return "检测到非Wi-Fi设备（设备数量：" + parsedData.at("device_count") + "）";
        }
        return "检测到非Wi-Fi设备";
    }
    
    // ========== 实体相关Trap ==========
    else if (trapType == "entity-brd-temp-alarm")
        return "单板温度告警";
    else if (trapType == "entity-brd-temp-resume")
        return "单板温度告警恢复";
    else if (trapType == "entity-brd-temp-fatal")
        return "单板温度危险告警";
    else if (trapType == "entity-cpu-rising")
        return "CPU占用率超过阈值";
    else if (trapType == "entity-cpu-resume")
        return "CPU占用率恢复";
    else if (trapType == "entity-fwd-cpu-rising")
        return "转发CPU占用率超过阈值";
    else if (trapType == "entity-fwd-cpu-resume")
        return "转发CPU占用率恢复";
    else if (trapType == "entity-mem-rising")
        return "系统内存占用率超过阈值";
    else if (trapType == "entity-mem-resume")
        return "系统内存占用率恢复";
    else if (trapType == "entity-disk-rising")
        return "磁盘利用率超过阈值";
    else if (trapType == "entity-disk-resume")
        return "磁盘利用率恢复";
    
    // 未知类型Trap的消息
    if (trapType == "unknown" || trapType == "huawei-unknown")
    {
        return "未知类型Trap告警（OID未解析）";
    }
    
    // 默认消息
    return "Trap告警";
}

// 生成告警级别
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
        trapType.find("interf") != string::npos ||
        trapType == "wids-non-wifi-device")
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

