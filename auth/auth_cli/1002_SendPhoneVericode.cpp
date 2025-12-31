#include "CAuthRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"

bool CAuthRelayApi::SendPhoneVericode(const CStr2Map& paramap,CStr2Map& resultmap,bool throwexp)
{
	CStr2Map inMap;
	stringstream ss;
	ss << RQ_SEND_PHONE_VERICODE;

	Tools::MapCpy(paramap,inMap);
	inMap["request_type"] = ss.str();
	inMap["transcode"] = TRANSCODE_SEND_PHONE_VERICODE;

	CRelayCall* m_pRelayCall = CAuthRelayApi::GetRelayObj(CAuthRelayApi::RQ_SEND_PHONE_VERICODE);
	int ret = m_pRelayCall->Call(inMap);

	InfoLog("send: %s",m_pRelayCall->getSendStr());
	InfoLog("recv: %s",m_pRelayCall->getResultStr());

	if(!CAuthRelayApi::ParseResult(ret,m_pRelayCall,resultmap,true))
		return false;
	return true;
}
