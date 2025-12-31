#include "CAuthRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

bool CAuthRelayApi::CheckUsernameExist(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp)
{
		CStr2Map inMap;
		stringstream ss;
		ss << CAuthRelayApi::AU_CHECK_USERNAME_EXIST;

                Tools::MapCpy(paramap,inMap);
		//relay服务请求
		inMap["request_type"] = ss.str();
		//交易码
		inMap["transcode"] = TRANSCODE_CHECK_USERNAME_EXIST;
		
		//inMap["uin"] = paramap["uin"];

		CRelayCall* m_pRelayCall = CAuthRelayApi::GetRelayObj(CAuthRelayApi::AU_CHECK_USERNAME_EXIST) ;
		bool ret = m_pRelayCall->Call(inMap) ;

		InfoLog("send : %s", m_pRelayCall->getSendStr()) ;
		InfoLog("recv : %s", m_pRelayCall->getResultStr()) ;
		//返回结果保存容器
		if(!CAuthRelayApi::ParseResult(ret,m_pRelayCall,resultmap,true))
				return false;


		return true;
}
