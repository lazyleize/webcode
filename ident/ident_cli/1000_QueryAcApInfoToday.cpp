#include "CIdentRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

bool CIdentRelayApi::QueryAcApInfoToday(CStr2Map& paramap, CStr2Map& resultmap, vector<CStr2Map>& vectmapArray, bool throwexp)
{
    CStr2Map inMap;
    
    inMap["transcode"] = TRANSCODE_COMM_QUERY;
    inMap["reqid"] = "1862";  // 需要配置新的reqid用于查询AP今日流量
    inMap["offset"] = "0";
    inMap["limit"] = "100";  // 足够大的值，确保能获取所有记录
    
    // 构建查询条件
    string str = "nono:123456";
    if (!paramap["ac_ip"].empty())
    {
        str += "|ac_ip:" + paramap["ac_ip"];
    }
    if (!paramap["ap_id"].empty())
    {
        str += "|ap_id:" + paramap["ap_id"];
    }
    if (!paramap["date"].empty())
    {
        str += "|date:" + paramap["date"];  // 今日日期，格式：YYYY-MM-DD
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
        {"Fid"},
        {"Fac_ip"},
        {"Fap_id"},
        {"Fdate"},
        {"Ftoday_upload"},
        {"Ftoday_download"},
        {"Ffirst_upload"},
        {"Ffirst_download"},
        {"Flast_upload"},
        {"Flast_download"},
        {"Ffirst_time"},
        {"Flast_time"},
        {"Fupdate_time"},
        {NULL}
    };
    
    resultmap["ret_num"] = "0";
    resultmap["total"] = "0";
    GetRecFromXml(vectmapArray, resultmap, m_pRelayCall->getResultStr(), &paras[0]);
    
    return true;
}

