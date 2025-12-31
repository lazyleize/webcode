#include "CAuthRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

bool CAuthRelayApi::UpdateUserLogin(const CStr2Map& paramap)
{
	CStr2Map inMap;
	stringstream ss;
	ss << CAuthRelayApi::RQ_UPDATE_USER_LOGIN;
	
	Tools::MapCpy(paramap,inMap);

	//relay服务请求
	inMap["request_type"] = ss.str();
	//交易码
	inMap["transcode"] = TRANSCODE_UPDATE_USER_LOGIN;
	//inMap["uin"] = paramap["uin"];
	//inMap["loading_ip"] = paramap["loading_ip"];

	CRelayCall* m_pRelayCall = CAuthRelayApi::GetRelayObj(CAuthRelayApi::RQ_UPDATE_USER_LOGIN) ;
	bool ret = m_pRelayCall->Call(inMap) ;

	InfoLog("send : %s", m_pRelayCall->getSendStr()) ;
	InfoLog("recv : %s", m_pRelayCall->getResultStr()) ;
	//返回结果保存容器
	CStr2Map resultmap; 
	if(!CAuthRelayApi::ParseResult(ret,m_pRelayCall,resultmap,true))
		return false;
	return true;
}
