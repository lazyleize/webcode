#include "CAuthRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

bool CAuthRelayApi::UserLogin(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp)
{
	CStr2Map inMap;
	stringstream ss;
	ss << CAuthRelayApi::RQ_USER_LOGIN;

	Tools::MapCpy(paramap,inMap);
	//relay服务请求
	inMap["request_type"] = ss.str();
	//交易码
	inMap["transcode"] = TRANSCODE_USER_LOGIN;

	CRelayCall* m_pRelayCall = CAuthRelayApi::GetRelayObj(CAuthRelayApi::RQ_USER_LOGIN) ;
	bool ret = m_pRelayCall->Call(inMap) ;

	InfoLog("send : %s", m_pRelayCall->getSendStr()) ;
	InfoLog("recv : %s", m_pRelayCall->getResultStr()) ;
	//返回结果保存容器
	if(!CAuthRelayApi::ParseResult(ret,m_pRelayCall,resultmap,true))
		return false;

    resultmap["syscode"] = "";
    resultmap["state"] = "";
	resultmap["name"] = "";
    resultmap["uid"] = "";
    resultmap["last_ip"] = "";
    resultmap["last_time"] = "";
    resultmap["last_mark"] = "";
    resultmap["fail_num"] = "";
    resultmap["transpwd_status"] = "";
    resultmap["truename_status"] = "";
    resultmap["card_status"] = "";
    resultmap["answer_status"] = "";
    resultmap["email_status"] = "";
    resultmap["user_type"] = "";
	resultmap["factory_id"] = "";
	resultmap["factory_name"] = "";

    vector<CStr2Map> vectmap;
    GetRecFromXml(vectmap,resultmap,m_pRelayCall->getResultStr(),NULL);

	return true;
}
