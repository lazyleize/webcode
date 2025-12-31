#include "CIdentRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

bool CIdentRelayApi::QueryAcControllerPeakTraffic(CStr2Map& paramap, CStr2Map& resultmap, bool throwexp)
{
    CStr2Map inMap;
    
    inMap["transcode"] = TRANSCODE_COMM_QUERY_NOTOTAL;
    inMap["reqid"] = "1860";  // 需要配置新的reqid用于峰值流量查询
    inMap["offset"] = "0";
    inMap["limit"] = "1";  // 只需要返回峰值记录
    
    // 构建查询条件
    string str = "nono:123456";
    if (!paramap["ac_ip"].empty())
    {
        str += "|ac_ip:" + paramap["ac_ip"];
    }
    if (!paramap["date"].empty())
    {
        str += "|date:" + paramap["date"];  // today/week/month
    }
    
    inMap["fields"] = str;
    
    CRelayCall* m_pRelayCall = CIdentRelayApi::GetRelayObj(CIdentRelayApi::IDENT_COMM_QUERY);
    
    bool ret = m_pRelayCall->Call(inMap);
    
    InfoLog("send : %s", m_pRelayCall->getSendStr());
    InfoLog("recv : %s", m_pRelayCall->getResultStr());
    
    if (!CIdentRelayApi::ParseResult(ret, m_pRelayCall, resultmap, throwexp))
    {
        return false;
    }
    
    // 解析返回结果
    const Para_Struct paras[] = {
        {"peak_time"},      // 峰值时间
        {"peak_traffic"},   // 峰值流量（KB/s）
        {NULL}
    };
    
    vector<CStr2Map> vectmapArray;
    resultmap["ret_num"] = "0";
    resultmap["total"] = "0";
    GetRecFromXml(vectmapArray, resultmap, m_pRelayCall->getResultStr(), &paras[0]);
    
    return true;
}

