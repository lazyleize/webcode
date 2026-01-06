#include "CAcControllerRestart.h"
#include "CIdentRelayApi.h"
#include "CSnmpClient.h"
#include "transxmlcfg.h"

#include <base/strHelper.hpp>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <vector>
#include <map>


int CAcControllerRestart::IdentCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map inMap, outMap;
    // 取得所有的请求参数（POST请求，JSON格式）
    string strPostdata = pReqData->GetPostData();
    CIdentPub::parsePubRespJson(strPostdata, inMap);
    
    // ========== 1. 参数验证 ==========
    
    // 获取routerId参数（必填）
    // 当routerId为"ALL"时，表示重启所有AP
    string routerId = inMap["routerId"];
    
    if (routerId.empty())
    {
        ErrorLog("routerId参数为空");
        throw(CTrsExp(ERR_UPFILE_COUNT, "routerId参数为空"));
        return 0;
    }
    
    // 转换为大写，方便比较
    string routerIdUpper = aps::StrHelper::toUpper(routerId);

    bool isRestartAll = (routerIdUpper == "ALL");
    
    InfoLog("请求重启路由器: routerId=%s, isRestartAll=%s", routerId.c_str(), isRestartAll ? "true" : "false");
    
    // ========== 2. 连接AC控制器 ==========
    
    // 从配置获取AC控制器信息
    string acIp = g_AcCfg.ip;
    string acCommunity = g_AcCfg.group_name;
    int acPort = 161;
    
    // 连接AC控制器
    CSnmpClient acClient(acIp, acPort, acCommunity);
    if (acClient.Init() != 0)
    {
        ErrorLog("连接AC控制器失败");
        throw(CTrsExp(ERR_UPFILE_COUNT, "连接AC控制器失败"));
        return 0;
    }
    
    // ========== 3. 处理重启逻辑 ==========
    
    if (isRestartAll)
    {
        // ========== 3.1 批量重启所有AP ==========
        
        InfoLog("开始批量重启所有AP");
        
        // 查询所有AP的类型
        // OID: 1.3.6.1.4.1.2011.6.139.13.3.10.1.4 - hwWlanIDIndexedApTypeInfo
        string apTypeOid = "1.3.6.1.4.1.2011.6.139.13.3.10.1.4";
        map<string, string> apTypeMap;
        
        if (acClient.Walk(apTypeOid, apTypeMap) != 0 || apTypeMap.empty())
        {
            ErrorLog("无法获取AP类型列表");
            acClient.Cleanup();
            throw(CTrsExp(ERR_UPFILE_COUNT, "无法获取AP类型列表"));
            return 0;
        }
        
        InfoLog("获取到 %zu 个AP的类型信息", apTypeMap.size());
        
        // 统计每种类型有哪些AP
        map<string, vector<string> > typeToApIds;  // 类型 -> AP ID列表
        for (map<string, string>::iterator it = apTypeMap.begin(); it != apTypeMap.end(); ++it)
        {
            string oid = it->first;
            string apType = it->second;
            
            // 从OID中提取AP ID
            // OID格式：1.3.6.1.4.1.2011.6.139.13.3.10.1.4.{apId}
            size_t lastDot = oid.find_last_of(".");
            if (lastDot != string::npos)
            {
                string apId = oid.substr(lastDot + 1);
                if (!apId.empty() && !apType.empty())
                {
                    typeToApIds[apType].push_back(apId);
                }
            }
        }
        
        InfoLog("统计到 %zu 种不同的AP类型", typeToApIds.size());
        
        // 设置短超时时间，不等待AC响应
        int originalTimeout = 3000;
        acClient.SetTimeout(100);
        acClient.SetRetries(0);
        
        int resetValue = 1;  // reset(重启AP)
        int successCount = 0;
        int failCount = 0;
        
        // 对每种类型执行批量重启
        for (map<string, vector<string> >::iterator it = typeToApIds.begin(); it != typeToApIds.end(); ++it)
        {
            string apType = it->first;
            vector<string>& apIds = it->second;
            
            InfoLog("重启AP类型: %s (包含 %zu 个AP)", apType.c_str(), apIds.size());
            
            // 使用 hwWlanApTypeReset OID批量重启该类型的所有AP
            // OID: 1.3.6.1.4.1.2011.6.139.13.2.1.1.6.{apType} - hwWlanApTypeReset
            // 注意：这会重启该类型的所有AP！
            string restartOid = "1.3.6.1.4.1.2011.6.139.13.2.1.1.6." + apType;
            
            InfoLog("执行批量重启: OID=%s, AP类型=%s, AP数量=%zu", 
                   restartOid.c_str(), apType.c_str(), apIds.size());
            
            int setResult = acClient.SetInt(restartOid, resetValue);
            
            if (setResult == 0)
            {
                InfoLog("批量重启成功: AP类型=%s, AP数量=%zu", apType.c_str(), apIds.size());
                successCount++;
            }
            else
            {
                string errorMsg = acClient.GetLastError();
                // 超时错误也认为成功（命令已发送）
                if (errorMsg.find("timeout") != string::npos || 
                    errorMsg.find("Timeout") != string::npos ||
                    errorMsg.find("TIMEOUT") != string::npos)
                {
                    InfoLog("批量重启命令已发送（超时是预期的）: AP类型=%s, AP数量=%zu",apType.c_str(), apIds.size());
                    successCount++;
                }
                else
                {
                    ErrorLog("批量重启失败: AP类型=%s, error=%s", apType.c_str(), errorMsg.c_str());
                    failCount++;
                }
            }
        }
        
        // 恢复原始超时设置
        acClient.SetTimeout(originalTimeout);
        acClient.SetRetries(1);
        
        InfoLog("批量重启完成: 成功 %d 种类型, 失败 %d 种类型, 总计 %zu 种类型",successCount, failCount, typeToApIds.size());
        
        if (failCount > 0 && successCount == 0)
        {
            ErrorLog("所有AP类型的批量重启都失败");
            acClient.Cleanup();
            throw(CTrsExp(ERR_UPFILE_COUNT, "批量重启所有AP失败"));
            return 0;
        }
    }
    else
    {
        // ========== 3.2 重启单个AP ==========
        
        // 验证AP是否存在（通过获取AP名称来验证）
        string apNameOid = "1.3.6.1.4.1.2011.6.139.13.3.10.1.5." + routerId;  // hwWlanIDIndexedApName
        string apName;
        if (acClient.Get(apNameOid, apName) != 0 || apName.empty())
        {
            ErrorLog("AP不存在或无法访问: routerId=%s", routerId.c_str());
            acClient.Cleanup();
            throw(CTrsExp(ERR_UPFILE_COUNT, "AP不存在或无法访问"));
            return 0;
        }
        
        InfoLog("验证AP存在: routerId=%s, name=%s", routerId.c_str(), apName.c_str());
        
        // ========== 4. 执行单个AP重启操作（根据AP ID） ==========
        
        // 使用 hwWlanIDIndexedApAdminOper OID重启AP（根据AP ID）
        // OID: 1.3.6.1.4.1.2011.6.139.13.3.10.1.20.{apId} - hwWlanIDIndexedApAdminOper
        // 索引: hwWlanIDIndexedApId (AP ID)
        // 值: 1=reset(重启), 2=confirm(确认), 3=manufacturerConfig(恢复出厂设置)
        // 注意：该节点只支持write，不支持read
        
        string restartOid = "1.3.6.1.4.1.2011.6.139.13.3.10.1.20." + routerId;  // hwWlanIDIndexedApAdminOper
        int resetValue = 1;  // reset(重启AP)
        
        // 设置合理的超时时间
        // 根据测试，正确的OID和权限下，命令会正常返回
        int originalTimeout = 3000;  // 保存原始超时时间
        acClient.SetTimeout(5000);   // 设置5秒超时（给重启操作足够时间）
        acClient.SetRetries(1);     // 重试1次
        
        InfoLog("执行重启操作: OID=%s, routerId=%s, AP name=%s, value=%d (reset)",restartOid.c_str(), routerId.c_str(), apName.c_str(), resetValue);
        
        // 执行SET操作
        int setResult = acClient.SetInt(restartOid, resetValue);
        
        // 恢复原始超时设置
        acClient.SetTimeout(originalTimeout);
        acClient.SetRetries(1);
        
        if (setResult != 0)
        {
            string errorMsg = acClient.GetLastError();
            // 如果是超时错误，认为命令可能已发送
            if (errorMsg.find("timeout") != string::npos || 
                errorMsg.find("Timeout") != string::npos ||
                errorMsg.find("TIMEOUT") != string::npos)
            {
                InfoLog("重启命令可能已发送（超时）: routerId=%s, OID=%s",routerId.c_str(), restartOid.c_str());
            }
            else
            {
                // 其他错误（如权限错误、OID错误等）
                ErrorLog("重启AP失败: routerId=%s, OID=%s, error=%s",routerId.c_str(), restartOid.c_str(), errorMsg.c_str());
                acClient.Cleanup();
                throw(CTrsExp(ERR_UPFILE_COUNT, "重启AP失败: " + errorMsg));
                return 0;
            }
        }
        else
        {
            InfoLog("重启命令发送成功: routerId=%s, AP name=%s, OID=%s",routerId.c_str(), apName.c_str(), restartOid.c_str());
        }
        
        InfoLog("重启AP命令已发送: routerId=%s, name=%s（不等待AP重启完成）",routerId.c_str(), apName.c_str());
    }
    
    // 清理AC控制器连接
    acClient.Cleanup();
    
    return 0;
}
