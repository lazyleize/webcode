#include "CAuthRelayApi.h"
#include "RuntimeGather.h"
#include "ParseXmlData.h"

bool CAuthRelayApi::QueryLoginInfo(const string& uin,CStr2Map& resultmap,bool throwexp)
{
    CStr2Map inMap;

    inMap["transcode"] = TRANSCODE_QUERY_LOGIN_INFO;
    inMap["uin"] = uin;

    CRelayCall* m_pRelayCall = CAuthRelayApi::GetRelayObj(CAuthRelayApi::AU_QUERY_LOGIN_INFO);
    bool ret = m_pRelayCall->Call(inMap);

    InfoLog("send: %s",m_pRelayCall->getSendStr());
    InfoLog("recv: %s",m_pRelayCall->getResultStr());

    if(!CAuthRelayApi::ParseResult(ret,m_pRelayCall,resultmap,true))
        return false;

    resultmap["ret_num"]="0";
    resultmap["last_ip"]="";
    resultmap["last_time"]="";
    resultmap["mobile"]="";
    resultmap["email"]="";
    resultmap["card_no"]="";
    resultmap["id_no"]="";
    resultmap["transpwd_status"]="";
    resultmap["truename_status"]="";
    resultmap["card_status"]="";
    resultmap["answer_status"]="";
    resultmap["cmer_status"]="";
    resultmap["email_status"]="";
    resultmap["mobile_status"]="";
    resultmap["real_name"]="";
    resultmap["name"]="";

    vector<CStr2Map> vectmapArray;
    GetRecFromXml(vectmapArray,resultmap,m_pRelayCall->getResultStr(),NULL);
	if(vectmapArray.size() > 0)
    {
        resultmap.insert(vectmapArray[0].begin(),vectmapArray[0].end());
    }
    return true;
}
