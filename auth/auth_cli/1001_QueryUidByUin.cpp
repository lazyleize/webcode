#include "CAuthRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

string CAuthRelayApi::QueryUidByUin(const string& uin)
{
    CStr2Map inMap;
    stringstream ss;
    ss << CAuthRelayApi::RQ_QUERY_USER;

    //relay服务请求
    inMap["request_type"] = ss.str();
    //交易码
    inMap["transcode"] = TRANSCODE_QUERY_UID_BY_UIN;
    inMap["uin"] = uin;

    CRelayCall* m_pRelayCall = CAuthRelayApi::GetRelayObj(CAuthRelayApi::RQ_QUERY_USER) ;
    bool ret = m_pRelayCall->Call(inMap) ;

    InfoLog("send : %s", m_pRelayCall->getSendStr()) ;
    InfoLog("recv : %s", m_pRelayCall->getResultStr()) ;
    //返回结果保存容器
    CStr2Map resultmap;
    CAuthRelayApi::ParseResult(ret,m_pRelayCall,resultmap,true);
    vector<CStr2Map> vectmap;
    resultmap["ret_num"] = "0";
    resultmap["uid"] = "";
    GetRecFromXml(vectmap,resultmap,m_pRelayCall->getResultStr(),NULL);
    if(resultmap["ret_num"] != "1")
    {
        return "";
    }
    else
    {
        return resultmap["uid"];
    }
}
