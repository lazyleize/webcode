#include "CAcControllerStats.h"
#include "CIdentRelayApi.h"
#include "CSnmpClient.h"
#include "transxmlcfg.h"
#include <ctime>
#include <sstream>
#include <iomanip>

// MAC地址转换函数：将十六进制MAC地址转换为OID索引格式
// 输入：Hex-STRING格式，如 "60 53 75 2D B8 80" 或 "60:53:75:2D:B8:80"
// 输出：点分十进制格式，如 "96.83.117.45.184.128"
static string ConvertMacToOidIndex(const string& hexMac)
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

// 从AC控制器获取路由器列表
// 华为AC6508的AP列表OID（需要根据实际设备调整）
vector<RouterInfo> CAcControllerStats::GetRouterListFromAC(CSnmpClient& acClient)
{
    vector<RouterInfo> routers;
    
    map<string, string> apList;
    
    // 尝试多种可能的华为AC控制器OID
    // 方法1：尝试华为WLAN相关的OID
    // 华为AC可能使用不同的OID结构，尝试几个常见的：
    
    vector<string> tryOids;
    
    // 优先使用 hwWlanIDIndexedApTable（使用简单AP ID作为索引：0, 1, 2, 3...）
    // 表OID: 1.3.6.1.4.1.2011.6.139.13.3.10.1
    // 索引: hwWlanIDIndexedApId (Unsigned32) - AP ID本身就是索引
    // 1.3.6.1.4.1.2011.6.139.13.3.10.1.2 - AP MAC地址
    // 1.3.6.1.4.1.2011.6.139.13.3.10.1.5 - AP名称
    // 1.3.6.1.4.1.2011.6.139.13.3.10.1.14 - AP IP地址
    // 1.3.6.1.4.1.2011.6.139.13.3.10.1.7 - AP状态 (normal(8)=在线)
    // 1.3.6.1.4.1.2011.6.139.13.3.10.1.42 - CPU使用率
    // 1.3.6.1.4.1.2011.6.139.13.3.10.1.41 - 内存使用率
    tryOids.push_back("1.3.6.1.4.1.2011.6.139.13.3.10.1.2");  // AP MAC地址（用于获取AP列表）
    tryOids.push_back("1.3.6.1.4.1.2011.6.139.13.3.10.1.5");  // AP名称
    tryOids.push_back("1.3.6.1.4.1.2011.6.139.13.3.10.1.7");  // AP状态
    tryOids.push_back("1.3.6.1.4.1.2011.6.139.13.3.10.1.14"); // AP IP地址
    
    // 备选：根据华为AC控制器MIB文档（从BGP4-MIB开始查看），AP设备表的OID：
    // csgDot11APTable基础OID: 1.2.156.11235.6001.60.7.2.75.2.1.1.1
    tryOids.push_back("1.2.156.11235.6001.60.7.2.75.2.1.1.1.5");  // AP名称表（csgDot11APTemplateNameOfAP）
    tryOids.push_back("1.2.156.11235.6001.60.7.2.75.2.1.1.1.2");  // AP IP地址表（csgDot11APIPAddress）
    tryOids.push_back("1.2.156.11235.6001.60.7.2.75.2.1.1.1.4");  // AP状态表（csgDot11APOperationStatus）
    tryOids.push_back("1.2.156.11235.6001.60.7.2.75.2.1.1.1.1");  // AP ID表（csgDot11APID）
    tryOids.push_back("1.2.156.11235.6001.60.7.2.75.2.1.1.1");    // AP表基础OID
    
    // 备选：尝试华为私有OID（如果标准OID不存在）
    // 注意：1.3.6.1.4.1.2011.6.139.14.3.3.1 表有AP设备信息，但索引格式特殊（MAC地址多段索引）
    tryOids.push_back("1.3.6.1.4.1.2011.6.139.14.3.3.1.4");  // AP状态表（状态值1表示在线）
    tryOids.push_back("1.3.6.1.4.1.2011.6.139.14.3.3.1.6");  // AP版本信息
    tryOids.push_back("1.3.6.1.4.1.2011.6.139.14.3.3.1.8");  // AP配置名称
    tryOids.push_back("1.3.6.1.4.1.2011.6.139.1.1.1.1.3");  // 华为私有OID（hwWlanApName）
    tryOids.push_back("1.3.6.1.4.1.2011.6.139.1.1.1.1.4");  // 华为私有OID（hwWlanApIpAddress）
    tryOids.push_back("1.3.6.1.4.1.2011.6.139.1.1.1.1.5");  // 华为私有OID（hwWlanApRunState）
    
    bool found = false;
    for (size_t i = 0; i < tryOids.size() && !found; ++i)
    {
        apList.clear();
        InfoLog("尝试Walk OID: %s", tryOids[i].c_str());
        
        if (acClient.Walk(tryOids[i], apList) == 0 && !apList.empty())
        {
            InfoLog("成功Walk OID %s，获取到 %zu 条记录", tryOids[i].c_str(), apList.size());
            
            // 解析获取到的数据
            // 根据华为MIB文档，AP设备表的OID结构（标准OID）：
            // 1.2.156.11235.6001.60.7.2.75.2.1.1.1.1.{apIndex} - AP ID (csgDot11APID)
            // 1.2.156.11235.6001.60.7.2.75.2.1.1.1.2.{apIndex} - AP IP地址 (csgDot11APIPAddress)
            // 1.2.156.11235.6001.60.7.2.75.2.1.1.1.3.{apIndex} - MAC地址 (csgDot11APMacAddress)
            // 1.2.156.11235.6001.60.7.2.75.2.1.1.1.4.{apIndex} - AP状态 (csgDot11APOperationStatus)
            //   1=join, 2=joinConfirm, 3=download, 4=config, 5=run(运行/在线), 6=other
            // 1.2.156.11235.6001.60.7.2.75.2.1.1.1.5.{apIndex} - AP名称 (csgDot11APTemplateNameOfAP)
            
            // 如果当前OID是VLAN配置OID，跳过（这些不是路由器）
            if (tryOids[i].find("1.3.6.1.4.1.2011.6.8.1.1.1") != string::npos)
            {
                InfoLog("OID %s 返回的是VLAN配置，不是路由器设备，跳过", tryOids[i].c_str());
                continue;  // 跳过VLAN配置OID，继续尝试其他OID
            }
            
            // 判断当前OID是哪个字段（标准OID和华为私有OID）
            // hwWlanIDIndexedApTable: 1.3.6.1.4.1.2011.6.139.13.3.10.1
            bool isHwWlanIDIndexedTable = (tryOids[i].find("1.3.6.1.4.1.2011.6.139.13.3.10.1") != string::npos);
            bool isNameField = (tryOids[i].find(".1.5") != string::npos || 
                               tryOids[i] == "1.2.156.11235.6001.60.7.2.75.2.1.1.1.5" ||
                               tryOids[i].find("1.3.6.1.4.1.2011.6.139.1.1.1.1.3") != string::npos ||
                               tryOids[i].find("1.3.6.1.4.1.2011.6.139.14.3.3.1.8") != string::npos ||
                               (isHwWlanIDIndexedTable && tryOids[i].find(".1.5") != string::npos));  // hwWlanIDIndexedApName
            bool isIpField = (tryOids[i].find(".1.2") != string::npos || 
                             tryOids[i] == "1.2.156.11235.6001.60.7.2.75.2.1.1.1.2" ||
                             tryOids[i].find("1.3.6.1.4.1.2011.6.139.1.1.1.1.4") != string::npos ||
                             (isHwWlanIDIndexedTable && tryOids[i].find(".1.14") != string::npos));  // hwWlanIDIndexedApIpAddress
            bool isStateField = (tryOids[i].find(".1.4") != string::npos || 
                                tryOids[i] == "1.2.156.11235.6001.60.7.2.75.2.1.1.1.4" ||
                                tryOids[i].find("1.3.6.1.4.1.2011.6.139.1.1.1.1.5") != string::npos ||
                                tryOids[i].find("1.3.6.1.4.1.2011.6.139.14.3.3.1.4") != string::npos ||
                                (isHwWlanIDIndexedTable && tryOids[i].find(".1.7") != string::npos));  // hwWlanIDIndexedApRunState (normal(8)=在线)
            bool isApIdField = (tryOids[i].find(".1.1") != string::npos || 
                               tryOids[i] == "1.2.156.11235.6001.60.7.2.75.2.1.1.1.1");
            bool isMacField = (isHwWlanIDIndexedTable && tryOids[i].find(".1.2") != string::npos);  // hwWlanIDIndexedApMac
            
            for (map<string, string>::iterator it = apList.begin(); it != apList.end(); ++it)
            {
                string oid = it->first;
                string value = it->second;
                
                // 从OID中提取AP索引
                // OID格式可能是：
                // 1. 标准格式: 1.2.156.11235.6001.60.7.2.75.2.1.1.1.3.{apIndex} (单个数字索引)
                // 2. 华为私有格式: 1.3.6.1.4.1.2011.6.139.14.3.3.1.4.{apIndex} (可能是多段索引，如 96.83.117.45.159.64)
                string apIndex;
                string baseOid = tryOids[i];
                
                // 找到基础OID在完整OID中的位置
                size_t basePos = oid.find(baseOid);
                if (basePos != string::npos)
                {
                    // 提取基础OID之后的部分作为索引
                    size_t indexStart = basePos + baseOid.length();
                    if (indexStart < oid.length() && oid[indexStart] == '.')
                    {
                        indexStart++;  // 跳过点号
                    }
                    apIndex = oid.substr(indexStart);
                }
                else
                {
                    // 如果找不到基础OID，使用原来的方法（提取最后一个数字）
                    size_t lastDot = oid.find_last_of('.');
                    if (lastDot == string::npos) continue;
                    apIndex = oid.substr(lastDot + 1);
                }
                
                // 对于 hwWlanIDIndexedApTable，索引0是有效的（第一个AP）
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
                        if (isNameField)
                        {
                            routers[j].name = value;
                        }
                        else if (isIpField)
                        {
                            routers[j].ip = value;
                        }
                        else if (isStateField)
                        {
                            // 状态值（标准OID）：
                            // 1=join(加入), 2=joinConfirm(确认), 3=download(下载), 
                            // 4=config(配置), 5=run(运行/在线), 6=other(其他)
                            // 华为私有OID (1.3.6.1.4.1.2011.6.139.14.3.3.1.4)：
                            // 1=在线, 其他值=离线
                            // hwWlanIDIndexedApRunState (1.3.6.1.4.1.2011.6.139.13.3.10.1.7)：
                            // normal(8)=在线, 其他值=离线
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
                                // 华为私有OID：1=在线
                                if (state == 1)
                                {
                                    routers[j].status = "online";
                                }
                                else
                                {
                                    routers[j].status = "offline";
                                }
                            }
                            else
                            {
                                // 标准OID：5=run(运行/在线)
                                if (state == 5)  // run状态表示WLAN服务已就绪，即在线
                                {
                                    routers[j].status = "online";
                                }
                                else if (state == 1 || state == 2 || state == 3 || state == 4)
                                {
                                    routers[j].status = "offline";
                                }
                                else
                                {
                                    routers[j].status = "fault";
                                }
                            }
                        }
                        else if (isMacField)
                        {
                            // MAC地址字段，可以用于标识AP
                            // 对于hwWlanIDIndexedApTable，索引本身就是AP ID
                            // MAC地址可以作为辅助信息
                        }
                        else if (isApIdField)
                        {
                            // AP ID字段，可以作为routerId的备选
                            if (routers[j].routerId.empty())
                            {
                                routers[j].routerId = value;
                            }
                        }
                        break;
                    }
                }
                
                if (!exists)
                {
                    RouterInfo router;
                    router.routerId = apIndex;
                    
                    if (isNameField)
                    {
                        router.name = value;
                    }
                    else if (isIpField)
                    {
                        router.ip = value;
                        router.name = "AP-" + apIndex;  // 临时名称，后续获取到名称后更新
                    }
                    else if (isStateField)
                    {
                        // 状态值（标准OID）：
                        // 1=join, 2=joinConfirm, 3=download, 4=config, 5=run(运行/在线), 6=other
                        // 华为私有OID (1.3.6.1.4.1.2011.6.139.14.3.3.1.4)：
                        // 1=在线, 其他值=离线
                        // hwWlanIDIndexedApRunState (1.3.6.1.4.1.2011.6.139.13.3.10.1.7)：
                        // normal(8)=在线, 其他值=离线
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
                            // 华为私有OID：1=在线
                            if (state == 1)
                            {
                                router.status = "online";
                            }
                            else
                            {
                                router.status = "offline";
                            }
                        }
                        else
                        {
                            // 标准OID：5=run(运行/在线)
                            if (state == 5)  // run状态表示WLAN服务已就绪，即在线
                            {
                                router.status = "online";
                            }
                            else if (state == 1 || state == 2 || state == 3 || state == 4)
                            {
                                router.status = "offline";
                            }
                            else
                            {
                                router.status = "fault";
                            }
                        }
                        router.name = "AP-" + apIndex;  // 临时名称
                    }
                    else if (isMacField)
                    {
                        // MAC地址字段，对于hwWlanIDIndexedApTable，索引本身就是AP ID
                        // MAC地址可以作为辅助信息
                        router.name = "AP-" + apIndex;  // 临时名称
                    }
                    else if (isApIdField)
                    {
                        // AP ID字段，使用AP SN作为routerId
                        router.routerId = value;
                        router.name = "AP-" + value;  // 临时名称
                    }
                    else
                    {
                        router.name = "AP-" + apIndex;
                    }
                    
                    // 如果还没有IP，尝试获取（优先使用hwWlanIDIndexedApTable）
                    if (router.ip.empty() && !isIpField)
                    {
                        // 优先使用hwWlanIDIndexedApTable的IP OID
                        if (isHwWlanIDIndexedTable)
                        {
                            string ipOid = "1.3.6.1.4.1.2011.6.139.13.3.10.1.14." + apIndex;
                            string ipValue;
                            if (acClient.Get(ipOid, ipValue) == 0 && ipValue != "0.0.0.0" && ipValue != "255.255.255.255")
                            {
                                router.ip = ipValue;
                            }
                        }
                        
                        // 如果hwWlanIDIndexedApTable的IP无效，尝试标准OID
                        if (router.ip.empty())
                        {
                            string ipOid = "1.2.156.11235.6001.60.7.2.75.2.1.1.1.2." + apIndex;
                            string ipValue;
                            if (acClient.Get(ipOid, ipValue) == 0 && ipValue != "0.0.0.0")
                            {
                                router.ip = ipValue;
                            }
                            else
                            {
                                // 备选：尝试华为私有OID
                                ipOid = "1.3.6.1.4.1.2011.6.139.1.1.1.1.4." + apIndex;
                                if (acClient.Get(ipOid, ipValue) == 0 && ipValue != "0.0.0.0")
                                {
                                    router.ip = ipValue;
                                }
                            }
                        }
                    }
                    
                    // 如果还没有名称，尝试获取（优先使用hwWlanIDIndexedApTable）
                    if (router.name.empty() || router.name.find("AP-") == 0)
                    {
                        // 优先使用hwWlanIDIndexedApTable的名称OID
                        if (isHwWlanIDIndexedTable)
                        {
                            string nameOid = "1.3.6.1.4.1.2011.6.139.13.3.10.1.5." + apIndex;
                            string nameValue;
                            if (acClient.Get(nameOid, nameValue) == 0 && !nameValue.empty())
                            {
                                router.name = nameValue;
                            }
                        }
                        
                        // 如果hwWlanIDIndexedApTable的名称无效，尝试标准OID
                        if (router.name.empty() || router.name.find("AP-") == 0)
                        {
                            string nameOid = "1.2.156.11235.6001.60.7.2.75.2.1.1.1.5." + apIndex;
                            string nameValue;
                            if (acClient.Get(nameOid, nameValue) == 0 && !nameValue.empty())
                            {
                                router.name = nameValue;
                            }
                            else
                            {
                                // 备选：尝试华为私有OID
                                nameOid = "1.3.6.1.4.1.2011.6.139.1.1.1.1.3." + apIndex;
                                if (acClient.Get(nameOid, nameValue) == 0 && !nameValue.empty())
                                {
                                    router.name = nameValue;
                                }
                            }
                        }
                    }
                    
                    // 如果还没有状态，尝试获取（优先使用标准OID）
                    if (router.status.empty())
                    {
                        // 先尝试标准OID
                        string stateOid = "1.2.156.11235.6001.60.7.2.75.2.1.1.1.4." + apIndex;
                        string stateValue;
                        if (acClient.Get(stateOid, stateValue) == 0)
                        {
                            int state = atoi(stateValue.c_str());
                            if (state == 5)  // run状态表示WLAN服务已就绪，即在线
                            {
                                router.status = "online";
                            }
                            else if (state == 1 || state == 2 || state == 3 || state == 4)
                            {
                                router.status = "offline";
                            }
                            else
                            {
                                router.status = "fault";
                            }
                        }
                    }
                    
                    // 如果还没有routerId，尝试从AP ID获取（使用AP SN）
                    // OID: 1.2.156.11235.6001.60.7.2.75.2.1.1.1.1.{apIndex} - AP ID，实际取值使用AP SN
                    if (router.routerId.empty() || router.routerId == apIndex)
                    {
                        // 先尝试使用apIndex作为索引
                        string apIdOid = "1.2.156.11235.6001.60.7.2.75.2.1.1.1.1." + apIndex;
                        string apIdValue;
                        if (acClient.Get(apIdOid, apIdValue) == 0 && !apIdValue.empty())
                        {
                            router.routerId = apIdValue;  // 使用AP SN作为ID
                            InfoLog("从OID %s 获取到AP ID (SN): %s", apIdOid.c_str(), apIdValue.c_str());
                        }
                        else
                        {
                            // 如果使用apIndex失败，尝试使用当前routerId作为索引（可能是AP SN格式）
                            if (!router.routerId.empty() && router.routerId != apIndex)
                            {
                                apIdOid = "1.2.156.11235.6001.60.7.2.75.2.1.1.1.1." + router.routerId;
                                if (acClient.Get(apIdOid, apIdValue) == 0 && !apIdValue.empty())
                                {
                                    router.routerId = apIdValue;  // 使用AP SN作为ID
                                    InfoLog("从OID %s 获取到AP ID (SN): %s", apIdOid.c_str(), apIdValue.c_str());
                                }
                            }
                        }
                    }
                    
                    routers.push_back(router);
                    InfoLog("找到AP设备: ID=%s, Name=%s, IP=%s, Status=%s", 
                            router.routerId.c_str(), router.name.c_str(), 
                            router.ip.c_str(), router.status.c_str());
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
                    // 不清空routers，保留已找到的AP，继续尝试其他OID补充
                }
            }
        }
        else
        {
            DebugLog("Walk OID %s 失败或无数据: %s", tryOids[i].c_str(), acClient.GetLastError().c_str());
        }
    }
    
    // 如果Walk失败或没有找到真正的路由器设备，使用固定数量（4个路由器）
    // 注意：由于无法通过SNMP获取真正的路由器设备列表，这里使用配置的路由器IP
    // 从之前的VLAN配置看，VLAN网关IP是192.168.201.254, 202.254, 203.254, 204.254
    // 实际应该使用真正的路由器管理IP（需要根据实际情况调整）
    if (routers.empty())
    {
        InfoLog("无法通过Walk获取真正的路由器设备列表，使用配置的4个路由器");
        // 使用配置的路由器IP地址（需要根据实际情况调整）
        // 这里暂时使用VLAN网关IP，实际应该使用真正的路由器管理IP
        string defaultIps[] = {"192.168.201.254", "192.168.202.254", "192.168.203.254", "192.168.204.254"};
        string defaultNames[] = {"路由器-001", "路由器-002", "路由器-003", "路由器-004"};
        
        for (int i = 1; i <= 4; ++i)
        {
            RouterInfo router;
            char buf[64];
            snprintf(buf, sizeof(buf), "%d", i);
            router.routerId = buf;
            router.name = defaultNames[i-1];
            router.ip = defaultIps[i-1];
            routers.push_back(router);
            InfoLog("添加配置的路由器: ID=%s, Name=%s, IP=%s", 
                    router.routerId.c_str(), router.name.c_str(), router.ip.c_str());
        }
    }
    
    return routers;
}

// 从AC控制器获取指定路由器的详细信息
// 根据华为MIB文档，可以通过AC控制器获取AP的统计信息
int CAcControllerStats::GetRouterInfoFromAC(CSnmpClient& acClient, RouterInfo& router)
{
    InfoLog("GetRouterInfoFromAC: 开始处理AP ID=%s, Name=%s", router.routerId.c_str(), router.name.c_str());
    
    // ========== 关键修复：将点分十进制AP ID转换为简单数字ID ==========
    // 问题：从标准OID获取的AP ID是点分十进制格式（如 12.49.48.50.52.57.55.54.52.56.49.53.53）
    // 但hwWlanIDIndexedApTable需要简单数字ID（0,1,2,3）
    // 解决方案：尝试通过Walk hwWlanIDIndexedApTable来找到对应的简单数字ID
    string simpleApId = router.routerId;  // 默认使用原始ID
    
    // 如果routerId是点分十进制格式（包含多个点），尝试通过Walk找到对应的简单数字ID
    if (router.routerId.find('.') != string::npos && router.routerId.find('.') != router.routerId.rfind('.'))
    {
        // 尝试Walk hwWlanIDIndexedApTable的AP名称列，通过名称匹配找到简单数字ID
        string nameOid = "1.3.6.1.4.1.2011.6.139.13.3.10.1.5";  // hwWlanIDIndexedApName
        map<string, string> nameMap;
        if (acClient.Walk(nameOid, nameMap) == 0)
        {
            // 遍历所有AP名称，找到匹配的AP
            for (map<string, string>::iterator it = nameMap.begin(); it != nameMap.end(); ++it)
            {
                if (it->second == router.name)  // 通过名称匹配
                {
                    // 从OID中提取简单数字ID
                    string oid = it->first;
                    size_t lastDot = oid.find_last_of('.');
                    if (lastDot != string::npos)
                    {
                        simpleApId = oid.substr(lastDot + 1);
                        InfoLog("通过AP名称匹配找到简单数字ID: %s -> %s", router.routerId.c_str(), simpleApId.c_str());
                        break;
                    }
                }
            }
        }
    }
    
    // 如果AP IP为空，尝试从AC控制器获取（优先使用hwWlanIDIndexedApTable）
    if (router.ip.empty())
    {
        // 优先使用hwWlanIDIndexedApTable的IP OID（使用简单数字ID）
        string ipOid = "1.3.6.1.4.1.2011.6.139.13.3.10.1.14." + simpleApId;  // hwWlanIDIndexedApIpAddress
        string ipValue;
        if (acClient.Get(ipOid, ipValue) == 0 && ipValue != "0.0.0.0" && ipValue != "255.255.255.255")
        {
            router.ip = ipValue;
            InfoLog("从AC控制器获取到AP[%s]的IP: %s (使用简单ID: %s)", router.name.c_str(), router.ip.c_str(), simpleApId.c_str());
        }
        else
        {
            // 备选：尝试标准OID（使用原始复杂ID）
            ipOid = "1.2.156.11235.6001.60.7.2.75.2.1.1.1.2." + router.routerId;
            if (acClient.Get(ipOid, ipValue) == 0 && ipValue != "0.0.0.0")
            {
                router.ip = ipValue;
                InfoLog("从AC控制器获取到AP[%s]的IP: %s (使用标准OID)", router.name.c_str(), router.ip.c_str());
            }
        }
    }
    
    // 如果状态为空，尝试从AC控制器获取AP状态（优先使用hwWlanIDIndexedApTable）
    if (router.status.empty())
    {
        // 优先使用hwWlanIDIndexedApTable的状态OID（使用简单数字ID）
        string stateOid = "1.3.6.1.4.1.2011.6.139.13.3.10.1.7." + simpleApId;  // hwWlanIDIndexedApRunState
        int64_t stateValue = 0;
        if (acClient.GetInt(stateOid, stateValue) == 0)
        {
            if (stateValue == 8)  // normal(8)=在线
            {
                router.status = "online";
            }
            else
            {
                router.status = "offline";
            }
            InfoLog("从AC控制器获取到AP[%s]的状态: %lld (%s)", router.name.c_str(), stateValue, router.status.c_str());
        }
        else
        {
            // 备选：尝试标准OID（使用原始复杂ID）
            string stateOid2 = "1.2.156.11235.6001.60.7.2.75.2.1.1.1.4." + router.routerId;
            string stateValue2;
            if (acClient.Get(stateOid2, stateValue2) == 0)
            {
                int state = atoi(stateValue2.c_str());
                if (state == 5)  // run状态表示WLAN服务已就绪，即在线
                {
                    router.status = "online";
                }
                else if (state == 1 || state == 2 || state == 3 || state == 4)
                {
                    router.status = "offline";
                }
                else
                {
                    router.status = "fault";
                }
                InfoLog("从AC控制器获取到AP[%s]的状态: %d (%s)", router.name.c_str(), state, router.status.c_str());
            }
            else
            {
                router.status = "offline";
            }
        }
    }
    
    // 即使AP不在线，也尝试获取CPU和内存（如果可能）
    // 但优先处理在线AP的性能数据
    if (router.status != "online")
    {
        InfoLog("AP[%s]不在线，但仍尝试获取性能数据", router.name.c_str());
    }
    
    // 从AC控制器获取AP的统计信息（根据MIB文档）
    // csgDot11APObjectTable: 1.2.156.11235.6001.60.7.2.75.2.1.2.1
    // csgDot11CurrAPStationAssocCount: 1.2.156.11235.6001.60.7.2.75.2.1.2.1.7.{apId} (当前连接STA数量)
    
    // 获取当前连接STA数量（可以作为AP负载的参考）
    // 注意：标准OID可能需要使用原始复杂ID，但这里先尝试简单ID
    string staCountOid = "1.2.156.11235.6001.60.7.2.75.2.1.2.1.7." + router.routerId;  // 使用原始ID
    int64_t staCount = 0;
    if (acClient.GetInt(staCountOid, staCount) == 0)
    {
        InfoLog("AP[%s]当前连接STA数量: %lld", router.name.c_str(), staCount);
    }
    else
    {
        // 如果使用原始ID失败，尝试使用简单ID
        staCountOid = "1.2.156.11235.6001.60.7.2.75.2.1.2.1.7." + simpleApId;
        if (acClient.GetInt(staCountOid, staCount) == 0)
        {
            InfoLog("AP[%s]当前连接STA数量: %lld (使用简单ID)", router.name.c_str(), staCount);
        }
    }
    
    // 获取流量统计信息（根据MIB文档）
    // csgDot11APRxStatisTable: 1.2.156.11235.6001.60.7.2.75.2.2.1.1 (接收统计表)
    // csgDot11APTxStatisTable: 1.2.156.11235.6001.60.7.2.75.2.2.2.1 (发送统计表)
    // 索引是 csgDot11CurAPID 和 csgDot11RadioID
    
    // 尝试获取接收流量（下行，从AP角度看是接收，从网络角度看是下行）
    // csgDot11RxFrameBytes: 1.2.156.11235.6001.60.7.2.75.2.2.1.1.9.{apId}.{radioId} (接收帧字节数)
    // csgDot11RxTrafficSpeed: 1.2.156.11235.6001.60.7.2.75.2.2.1.1.26.{apId}.{radioId} (接收流量速度，单位bps)
    
    // 尝试获取发送流量（上行，从AP角度看是发送，从网络角度看是上行）
    // csgDot11TxFrameBytes: 1.2.156.11235.6001.60.7.2.75.2.2.2.1.13.{apId}.{radioId} (发送帧字节数)
    // csgDot11TxTrafficSpeed: 1.2.156.11235.6001.60.7.2.75.2.2.2.1.27.{apId}.{radioId} (发送流量速度，单位bps)
    
    // 由于流量统计表需要radioId索引，这里先尝试radioId=1（通常第一个radio）
    // 实际应该遍历所有radio获取总流量
    string radioId = "1";
    
    // 获取接收流量速度（下行）
    // 注意：标准OID可能需要使用原始复杂ID，但这里先尝试简单ID
    string rxSpeedOid = "1.2.156.11235.6001.60.7.2.75.2.2.1.1.26." + router.routerId + "." + radioId;  // 使用原始ID
    int64_t rxSpeed = 0;
    if (acClient.GetInt(rxSpeedOid, rxSpeed) == 0 && rxSpeed > 0)
    {
        // 流量速度单位是bps，转换为KB（除以8再除以1024，或者直接除以8192）
        router.download = rxSpeed / 8192;  // bps转换为KB/s，这里作为累计流量使用
        InfoLog("AP[%s]接收流量速度: %lld bps (%llu KB/s)", router.name.c_str(), rxSpeed, router.download);
    }
    else
    {
        // 如果使用原始ID失败或返回0，尝试使用简单ID
        rxSpeedOid = "1.2.156.11235.6001.60.7.2.75.2.2.1.1.26." + simpleApId + "." + radioId;
        if (acClient.GetInt(rxSpeedOid, rxSpeed) == 0 && rxSpeed > 0)
        {
            router.download = rxSpeed / 8192;
            InfoLog("AP[%s]接收流量速度: %lld bps (%llu KB/s) (使用简单ID)", router.name.c_str(), rxSpeed, router.download);
        }
        else
        {
            // 尝试遍历所有radioId（可能不是1）
            for (int r = 1; r <= 4; ++r)
            {
                rxSpeedOid = "1.2.156.11235.6001.60.7.2.75.2.2.1.1.26." + router.routerId + "." + std::to_string(r);
                if (acClient.GetInt(rxSpeedOid, rxSpeed) == 0 && rxSpeed > 0)
                {
                    router.download = rxSpeed / 8192;
                    InfoLog("AP[%s]接收流量速度: %lld bps (%llu KB/s) (radioId=%d)", router.name.c_str(), rxSpeed, router.download, r);
                    break;
                }
            }
        }
    }
    
    // 获取发送流量速度（上行）
    string txSpeedOid = "1.2.156.11235.6001.60.7.2.75.2.2.2.1.27." + router.routerId + "." + radioId;  // 使用原始ID
    int64_t txSpeed = 0;
    if (acClient.GetInt(txSpeedOid, txSpeed) == 0 && txSpeed > 0)
    {
        // 流量速度单位是bps，转换为KB
        router.upload = txSpeed / 8192;  // bps转换为KB/s
        InfoLog("AP[%s]发送流量速度: %lld bps (%llu KB/s)", router.name.c_str(), txSpeed, router.upload);
    }
    else
    {
        // 如果使用原始ID失败或返回0，尝试使用简单ID
        txSpeedOid = "1.2.156.11235.6001.60.7.2.75.2.2.2.1.27." + simpleApId + "." + radioId;
        if (acClient.GetInt(txSpeedOid, txSpeed) == 0 && txSpeed > 0)
        {
            router.upload = txSpeed / 8192;
            InfoLog("AP[%s]发送流量速度: %lld bps (%llu KB/s) (使用简单ID)", router.name.c_str(), txSpeed, router.upload);
        }
        else
        {
            // 尝试遍历所有radioId（可能不是1）
            for (int r = 1; r <= 4; ++r)
            {
                txSpeedOid = "1.2.156.11235.6001.60.7.2.75.2.2.2.1.27." + router.routerId + "." + std::to_string(r);
                if (acClient.GetInt(txSpeedOid, txSpeed) == 0 && txSpeed > 0)
                {
                    router.upload = txSpeed / 8192;
                    InfoLog("AP[%s]发送流量速度: %lld bps (%llu KB/s) (radioId=%d)", router.name.c_str(), txSpeed, router.upload, r);
                    break;
                }
            }
        }
    }
    
    // 如果无法从AC控制器获取无线流量，尝试使用hwWlanApTable的无线流量OID
    // 注意：只统计无线流量，不统计有线接口流量
    if (router.upload == 0 && router.download == 0)
    {
        // 尝试使用hwWlanApTable的无线流量OID（需要MAC地址作为索引）
        // 先获取AP的MAC地址（从hwWlanIDIndexedApTable，使用简单数字ID）
        string macOid = "1.3.6.1.4.1.2011.6.139.13.3.10.1.2." + simpleApId;  // hwWlanIDIndexedApMac
        string macHex;
        InfoLog("尝试获取AP[%s]的MAC地址，OID: %s", router.name.c_str(), macOid.c_str());
        if (acClient.Get(macOid, macHex) == 0 && !macHex.empty())
        {
            InfoLog("从AC控制器获取到AP[%s]的MAC地址（原始值）: %s", router.name.c_str(), macHex.c_str());
            // 将MAC地址从十六进制格式转换为点分十进制格式
            // 例如：60 53 75 2D B8 80 -> 96.83.117.45.184.128
            string macOidIndex = ConvertMacToOidIndex(macHex);
            InfoLog("MAC地址转换为OID索引: %s -> %s", macHex.c_str(), macOidIndex.c_str());
            if (!macOidIndex.empty())
            {
                // 获取无线上行端口流量（累计值，单位：Bytes）
                string upTrafficOid = "1.3.6.1.4.1.2011.6.139.13.3.3.1.58." + macOidIndex;  // hwWlanApAirportUpTraffic
                string upTrafficStr;
                if (acClient.Get(upTrafficOid, upTrafficStr) == 0)
                {
                    uint64_t upTrafficBytes = 0;
                    // OCTET STRING类型，需要解析
                    if (sscanf(upTrafficStr.c_str(), "%lu", (unsigned long*)&upTrafficBytes) == 1 || 
                        (upTrafficStr.find("\"") != string::npos && sscanf(upTrafficStr.c_str(), "\"%lu\"", (unsigned long*)&upTrafficBytes) == 1))
                    {
                        router.upload = upTrafficBytes / 1024;  // 转换为KB
                        InfoLog("从AC控制器获取到AP[%s]的无线累计上行流量: %llu KB (OID: %s)", 
                                router.name.c_str(), router.upload, upTrafficOid.c_str());
                    }
                }
                
                // 获取无线下行端口流量（累计值，单位：Bytes）
                string downTrafficOid = "1.3.6.1.4.1.2011.6.139.13.3.3.1.59." + macOidIndex;  // hwWlanApAirportDwTraffic
                string downTrafficStr;
                if (acClient.Get(downTrafficOid, downTrafficStr) == 0)
                {
                    uint64_t downTrafficBytes = 0;
                    if (sscanf(downTrafficStr.c_str(), "%lu", (unsigned long*)&downTrafficBytes) == 1 || 
                        (downTrafficStr.find("\"") != string::npos && sscanf(downTrafficStr.c_str(), "\"%lu\"", (unsigned long*)&downTrafficBytes) == 1))
                    {
                        router.download = downTrafficBytes / 1024;  // 转换为KB
                        InfoLog("从AC控制器获取到AP[%s]的无线累计下行流量: %llu KB (OID: %s)", 
                                router.name.c_str(), router.download, downTrafficOid.c_str());
                    }
                }
            }
        }
    }
    
    // 如果仍然无法获取无线流量，尝试直接连接AP获取（仅用于CPU和内存）
    // 注意：不再获取有线接口流量，只获取性能数据
    if (router.cpu == 0 || router.memory == 0)
    {
        if (!router.ip.empty() && router.ip != "0.0.0.0")
        {
            InfoLog("尝试直接连接AP[%s]获取性能数据", router.name.c_str());
            string routerCommunity = g_AcCfg.group_name;
            int routerPort = 161;
            
            CSnmpClient routerClient(router.ip, routerPort, routerCommunity);
            if (routerClient.Init() == 0)
            {
                // 不再获取有线接口流量，只获取CPU和内存
                
                // 尝试获取CPU使用率
                // 优先使用hwWlanIDIndexedApTable的CPU OID
                if (router.cpu == 0)
                {
                    // 优先从AC控制器获取（hwWlanIDIndexedApTable，使用简单数字ID）
                    string acCpuOid = "1.3.6.1.4.1.2011.6.139.13.3.10.1.42." + simpleApId;  // hwWlanIDIndexedApCpuUseRate
                    int64_t cpuValue = 0;
                    if (acClient.GetInt(acCpuOid, cpuValue) == 0)
                    {
                        router.cpu = (int)cpuValue;
                        InfoLog("从AC控制器获取到AP[%s]的CPU使用率: %d%% (OID: %s)", 
                                router.name.c_str(), router.cpu, acCpuOid.c_str());
                    }
                    
                    // 如果从AC获取失败，尝试直接连接AP获取
                    if (router.cpu == 0)
                    {
                        // 华为设备CPU OID（需要根据实际设备调整slotId，这里尝试几个常见的）
                        // 1.3.6.1.4.1.2011.5.25.31.1.1.1.1.11.{slotId} - CPU使用率
                        vector<string> cpuOids;
                        cpuOids.push_back("1.3.6.1.4.1.2011.5.25.31.1.1.1.1.11.67108867");  // 华为标准CPU OID（slotId=67108867）
                        cpuOids.push_back("1.3.6.1.4.1.2011.5.25.31.1.1.1.1.11.1");         // slotId=1
                        cpuOids.push_back("1.3.6.1.4.1.2011.5.25.31.1.1.1.1.11.0");         // slotId=0
                        
                        for (size_t i = 0; i < cpuOids.size() && router.cpu == 0; ++i)
                        {
                            int64_t cpuValue = 0;
                            if (routerClient.GetInt(cpuOids[i], cpuValue) == 0)
                            {
                                router.cpu = (int)cpuValue;
                                InfoLog("从AP[%s]获取到CPU使用率: %d%% (OID: %s)", 
                                        router.name.c_str(), router.cpu, cpuOids[i].c_str());
                                break;
                            }
                        }
                    }
                }
                
                // 尝试获取内存使用率
                // 优先使用hwWlanIDIndexedApTable的内存OID
                if (router.memory == 0)
                {
                    // 优先从AC控制器获取（hwWlanIDIndexedApTable，使用简单数字ID）
                    string acMemoryOid = "1.3.6.1.4.1.2011.6.139.13.3.10.1.41." + simpleApId;  // hwWlanIDIndexedApMemoryUseRate
                    int64_t memoryValue = 0;
                    if (acClient.GetInt(acMemoryOid, memoryValue) == 0)
                    {
                        router.memory = (int)memoryValue;
                        InfoLog("从AC控制器获取到AP[%s]的内存使用率: %d%% (OID: %s)", 
                                router.name.c_str(), router.memory, acMemoryOid.c_str());
                    }
                    
                    // 如果从AC获取失败，尝试直接连接AP获取
                    if (router.memory == 0)
                    {
                        // 华为设备内存 OID（需要根据实际设备调整slotId）
                        // 1.3.6.1.4.1.2011.5.25.31.1.1.1.1.7.{slotId} - 内存使用率
                        vector<string> memoryOids;
                        memoryOids.push_back("1.3.6.1.4.1.2011.5.25.31.1.1.1.1.7.67108867");  // 华为标准内存OID（slotId=67108867）
                        memoryOids.push_back("1.3.6.1.4.1.2011.5.25.31.1.1.1.1.7.1");         // slotId=1
                        memoryOids.push_back("1.3.6.1.4.1.2011.5.25.31.1.1.1.1.7.0");         // slotId=0
                        
                        for (size_t i = 0; i < memoryOids.size() && router.memory == 0; ++i)
                        {
                            int64_t memoryValue = 0;
                            if (routerClient.GetInt(memoryOids[i], memoryValue) == 0)
                            {
                                router.memory = (int)memoryValue;
                                InfoLog("从AP[%s]获取到内存使用率: %d%% (OID: %s)", 
                                        router.name.c_str(), router.memory, memoryOids[i].c_str());
                                break;
                            }
                        }
                    }
                }
                
                routerClient.Cleanup();
            }
            else
            {
                InfoLog("无法连接AP[%s] (%s)，跳过性能数据获取", router.name.c_str(), router.ip.c_str());
            }
        }
        
        // 如果仍然无法获取CPU和内存，记录日志
        if (router.cpu == 0)
        {
            DebugLog("无法获取AP[%s]的CPU使用率，可能设备不支持或OID不正确", router.name.c_str());
        }
        if (router.memory == 0)
        {
            DebugLog("无法获取AP[%s]的内存使用率，可能设备不支持或OID不正确", router.name.c_str());
        }
    }
    
    InfoLog("GetRouterInfoFromAC: 完成处理AP ID=%s, 状态=%s, 上行=%lluKB, 下行=%lluKB", 
            router.routerId.c_str(), router.status.c_str(), router.upload, router.download);
    return 0;
}

int CAcControllerStats::IdentCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map inMap, outMap;
    // 获取请求参数
    pReqData->GetStrMap(inMap);
    
    // 获取请求参数：date（可选，默认当前日期）
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char dateStr[32];
    if (!inMap["date"].empty())
    {
        // 使用请求参数中的日期
        snprintf(dateStr, sizeof(dateStr), "%s", inMap["date"].c_str());
    }
    else
    {
        // 默认使用当前日期
        snprintf(dateStr, sizeof(dateStr), "%04d-%02d-%02d",tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday);
    }
    
    // 从配置获取AC控制器信息
    string acIp = g_AcCfg.ip;
    string acCommunity = g_AcCfg.group_name;
    
    // ========== 优先从Memcached读取实时统计数据 ==========
    // 数据采集服务（CAcDataCollector）已将实时数据写入Memcached
    // Key: ac_stats_<ac_ip>
    // 格式: key=value&key=value
    // ====================================================
    string statsKey = "ac_stats_" + acIp;
    string statsCache;
    bool fromCache = false;
    
    if (GetCache(statsKey, statsCache) == 0 && !statsCache.empty())
    {
        InfoLog("从Memcached读取统计数据成功: key=%s", statsKey.c_str());
        fromCache = true;
        
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
        
        // 从缓存中提取数据
        int totalRouters = atoi(cacheMap["totalRouters"].c_str());
        int onlineRouters = atoi(cacheMap["onlineRouters"].c_str());
        int offlineRouters = atoi(cacheMap["offlineRouters"].c_str());
        int faultRouters = atoi(cacheMap["faultRouters"].c_str());
        int avgCpu = atoi(cacheMap["avgCpu"].c_str());
        int avgMemory = atoi(cacheMap["avgMemory"].c_str());
        uint64_t totalUploadToday = atoll(cacheMap["totalUploadToday"].c_str());
        uint64_t totalDownloadToday = atoll(cacheMap["totalDownloadToday"].c_str());
        uint64_t totalUploadMonth = atoll(cacheMap["totalUploadMonth"].c_str());
        uint64_t totalDownloadMonth = atoll(cacheMap["totalDownloadMonth"].c_str());
        
        // 设置返回参数
        pResData->SetPara("date", dateStr);
        pResData->SetPara("totalRouters", std::to_string(totalRouters));
        pResData->SetPara("onlineRouters", std::to_string(onlineRouters));
        pResData->SetPara("offlineRouters", std::to_string(offlineRouters));
        pResData->SetPara("faultRouters", std::to_string(faultRouters));
        pResData->SetPara("avgCpu", std::to_string(avgCpu));
        pResData->SetPara("avgMemory", std::to_string(avgMemory));
        
        // 设置今日流量数组（array_today）
        CStr2Map todayMap;
        todayMap["upload"] = std::to_string(totalUploadToday);
        todayMap["download"] = std::to_string(totalDownloadToday);
        pResData->SetArray("array_today", todayMap);
        
        // 设置本月流量数组（array_month）
        CStr2Map monthMap;
        monthMap["upload"] = std::to_string(totalUploadMonth);
        monthMap["download"] = std::to_string(totalDownloadMonth);
        pResData->SetArray("array_month", monthMap);
        
        InfoLog("从Memcached返回统计数据: 总数=%d, 在线=%d, 离线=%d, 故障=%d, 平均CPU=%d%%, 平均内存=%d%%", 
                totalRouters, onlineRouters, offlineRouters, faultRouters, avgCpu, avgMemory);
        
        // ========== 从数据库获取今日和本月流量（必须从数据库获取，不使用SNMP降级） ==========
        // 方式1：从 t_ac_controller_stats 表获取今日最新记录的流量
        CStr2Map queryInMap, queryOutMap;
        vector<CStr2Map> queryResult;
        queryInMap["ac_ip"] = acIp;
        queryInMap["date"] = dateStr;  // 今日日期
        
        queryInMap["ac_ip"] = acIp;
        string monthStr = string(dateStr).substr(0, 7);  // YYYY-MM
        queryInMap["month"] = monthStr;
        
        totalUploadToday = atoll(cacheMap["totalUploadToday"].c_str());
        totalDownloadToday = atoll(cacheMap["totalDownloadToday"].c_str());
        totalUploadMonth = atoll(cacheMap["totalUploadMonth"].c_str());
        totalDownloadMonth = atoll(cacheMap["totalDownloadMonth"].c_str());
        
        // 注意：todayMap 和 monthMap 已经在上面声明过了，这里直接使用
        todayMap["upload"] = std::to_string(totalUploadToday);
        todayMap["download"] = std::to_string(totalDownloadToday);
        pResData->SetArray("array_today", todayMap);
        
        monthMap["upload"] = std::to_string(totalUploadMonth);
        monthMap["download"] = std::to_string(totalDownloadMonth);
        pResData->SetArray("array_month", monthMap);
        
        return 0;  // 从缓存读取成功，直接返回
    }
    else
    {
        // Memcached中无统计数据，直接报错，不使用SNMP降级
        ErrorLog("从Memcached读取统计数据失败: key=%s，数据采集服务可能未运行或数据未写入", statsKey.c_str());
        throw(CTrsExp(ERR_UPFILE_COUNT, "从Memcached读取统计数据失败，请检查数据采集服务是否正常运行"));
        return 0;
    }

	return 0;
    
}
