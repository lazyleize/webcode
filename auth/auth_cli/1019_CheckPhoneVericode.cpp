#include "CAuthRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

bool CAuthRelayApi::CheckPhoneVericode(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp)
{
		CStr2Map inMap;
		stringstream ss;
		ss << CAuthRelayApi::AU_CHECK_PHONE_VERICODE;

		Tools::MapCpy(paramap,inMap);
		//relay服务请求
		inMap["request_type"] = ss.str();
		//交易码
		inMap["transcode"] = TRANSCODE_CHECK_PHONE_VERICODE;
		
		//inMap["uin"] = paramap["uin"];
		//inMap["password"] = paramap["password"];

		CRelayCall* m_pRelayCall = CAuthRelayApi::GetRelayObj(CAuthRelayApi::AU_CHECK_PHONE_VERICODE) ;
		bool ret = m_pRelayCall->Call(inMap) ;

		InfoLog("send : %s", m_pRelayCall->getSendStr()) ;
		InfoLog("recv : %s", m_pRelayCall->getResultStr()) ;
		//返回结果保存容器
		if(!CAuthRelayApi::ParseResult(ret,m_pRelayCall,resultmap,true))
				return false;

	//	resultmap["ret_num"] = "0";
	//	resultmap["name"] = "";
	//	resultmap["state"] = "";
	//	resultmap["user_type"] = "";
	//	vector<CStr2Map> vectmapArray;
	//	GetRecFromXml(vectmapArray,resultmap,m_pRelayCall->getResultStr(),NULL);
		return true;
}
