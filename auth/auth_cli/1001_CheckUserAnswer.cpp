#include "CAuthRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

bool CAuthRelayApi::CheckUserAnswer(CStr2Map& paramap,bool throwexp)
{
    CStr2Map inMap;
    inMap = paramap;
    stringstream ss;
    ss << CAuthRelayApi::RQ_QUERY_USER;

    //relay服务请求
    inMap["request_type"] = ss.str();
    //交易码
    inMap["transcode"] = TRANSCODE_CHECK_USER_ANSWER;

    CRelayCall* m_pRelayCall = CAuthRelayApi::GetRelayObj(CAuthRelayApi::RQ_QUERY_USER) ;
    bool ret = m_pRelayCall->Call(inMap) ;

    InfoLog("send : %s", m_pRelayCall->getSendStr()) ;
    InfoLog("recv : %s", m_pRelayCall->getResultStr()) ;
    //返回结果保存容器
    CStr2Map resultmap;
    if(!CAuthRelayApi::ParseResult(ret,m_pRelayCall,resultmap,true))
        return false;
    return true;
}
