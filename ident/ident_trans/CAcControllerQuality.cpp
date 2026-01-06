/************************************************************
 Desc:     AC控制器网络质量统计接口 - 返回终端速率达标率等质量指标
 Auth:     Auto
 Modify:   2025-12-02
 ***********************************************************/
#include "CAcControllerQuality.h"
#include "CIdentComm.h"
#include "CSnmpClient.h"
#include "CIdentRelayApi.h"
#include <sstream>
#include <iomanip>

using namespace std;

// ========== 接口实现 ==========

int CAcControllerQuality::IdentCommit(CReqData *pReqData, CResData *pResData)
{
    InfoLog("========== AC控制器网络质量统计接口 ==========");
    string acIp = g_AcCfg.ip;
    // 连接AC控制器
    string acCommunity = g_AcCfg.group_name;
    int acPort = 161;
    CSnmpClient acClient(acIp, acPort, acCommunity);
    
    if (acClient.Init() != 0)
    {
        ErrorLog("连接AC控制器失败: ac_ip=%s, error=%s", acIp.c_str(), acClient.GetLastError().c_str());
        throw(CTrsExp(ERR_UPFILE_COUNT, "连接AC控制器失败"));
        return -1;
    }
    
    int staRateComplianceRate = 0;  // 终端速率达标率（%）
    int totalUsers = 0;             // 总用户数
    int onlineRouters = 0;          // 在线AP数量
    int totalRouters = 0;           // AP总数
    
    // 获取终端速率达标率
    string complianceRateOid = "1.3.6.1.4.1.2011.6.139.12.1.3.1.0";  // hwWlanStaRateComplianceRate
    int64_t complianceRateValue = 0;
    if (acClient.GetInt(complianceRateOid, complianceRateValue) == 0 && 
        complianceRateValue >= 0 && complianceRateValue <= 100)
    {
        staRateComplianceRate = (int)complianceRateValue;
        InfoLog("从AC获取到终端速率达标率: %d%%", staRateComplianceRate);
    }
    else
    {
        ErrorLog("从AC获取终端速率达标率失败: OID=%s, error=%s",complianceRateOid.c_str(), acClient.GetLastError().c_str());
        staRateComplianceRate = 0;  // 默认值
    }
    
    // 获取总用户数
    string userCountOid = "1.2.156.11235.6001.60.7.2.75.1.1.2.2.0";
    int64_t userCountValue = 0;
    if (acClient.GetInt(userCountOid, userCountValue) == 0)
    {
        totalUsers = (int)userCountValue;
        InfoLog("从AC获取到总用户数: %d", totalUsers);
    }
    else
    {
        ErrorLog("从AC获取总用户数失败: OID=%s, error=%s",userCountOid.c_str(), acClient.GetLastError().c_str());
    }
    
    // 获取AP总数
    string apCountOid = "1.3.6.1.4.1.2011.6.139.12.1.5.7.0";  // hwWlanApCount
    int64_t apCountValue = 0;
    if (acClient.GetInt(apCountOid, apCountValue) == 0)
    {
        totalRouters = (int)apCountValue;
        InfoLog("从AC获取到AP总数: %d", totalRouters);
    }
    else
    {
        ErrorLog("从AC获取AP总数失败: OID=%s, error=%s",apCountOid.c_str(), acClient.GetLastError().c_str());
    }
    
    // 获取在线AP数量
    string onlineApOid = "1.3.6.1.4.1.2011.6.139.12.1.2.1.0";  // hwWlanCurJointApNum
    int64_t onlineApValue = 0;
    if (acClient.GetInt(onlineApOid, onlineApValue) == 0)
    {
        onlineRouters = (int)onlineApValue;
        InfoLog("从AC获取到在线AP数量: %d", onlineRouters);
    }
    else
    {
        ErrorLog("从AC获取在线AP数量失败: OID=%s, error=%s",onlineApOid.c_str(), acClient.GetLastError().c_str());
    }
    
    acClient.Cleanup();
    
    // ========== 4. 计算质量等级 ==========
    string qualityLevel;  // 质量等级：excellent/good/fair/poor
    string qualityDesc;    // 质量描述
    
    if (staRateComplianceRate >= 80)
    {
        qualityLevel = "excellent";
        qualityDesc = "优秀";
    }
    else if (staRateComplianceRate >= 50)
    {
        qualityLevel = "good";
        qualityDesc = "良好";
    }
    else if (staRateComplianceRate >= 30)
    {
        qualityLevel = "fair";
        qualityDesc = "一般";
    }
    else
    {
        qualityLevel = "poor";
        qualityDesc = "较差";
    }
    
    // ========== 5. 设置返回参数 ==========
    
    // 基本信息
    pResData->SetPara("ac_ip", acIp);
    pResData->SetPara("total_routers", std::to_string(totalRouters));
    pResData->SetPara("online_routers", std::to_string(onlineRouters));
    pResData->SetPara("total_users", std::to_string(totalUsers));
    
    // 网络质量指标
    pResData->SetPara("sta_rate_compliance_rate", std::to_string(staRateComplianceRate));  // 终端速率达标率（%）
    pResData->SetPara("quality_level", qualityLevel);      // 质量等级
    pResData->SetPara("quality_desc", qualityDesc);        // 质量描述
    
    // 达标标准说明
    pResData->SetPara("compliance_threshold", "24");       // 达标阈值（Mbps）
    pResData->SetPara("compliance_desc", "终端速率 > 24Mbps 为达标");
    
    // 数据来源
    pResData->SetPara("data_source", "realtime");
    
    InfoLog("返回网络质量数据: staRateComplianceRate=%d%%, qualityLevel=%s, totalUsers=%d",staRateComplianceRate, qualityLevel.c_str(), totalUsers);
    
    return 0;
}

