#include "CAuthRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"

bool CAuthRelayApi::UserActive(const CStr2Map& paramap,bool throwexp)
{
	CStr2Map inMap = paramap;
	stringstream ss;
	ss << AU_USER_ACTIVE;
	inMap["request_type"] = ss.str();
	inMap["transcode"] = TRANSCODE_USER_ACTIVE;

	CRelayCall* m_pRelayCall = CAuthRelayApi::GetRelayObj(CAuthRelayApi::AU_USER_ACTIVE);
	int ret = m_pRelayCall->Call(inMap);

	InfoLog("send: %s",m_pRelayCall->getSendStr());
	InfoLog("recv: %s",m_pRelayCall->getResultStr());

	CStr2Map resultmap;
	if(!CAuthRelayApi::ParseResult(ret,m_pRelayCall,resultmap,true))
		return false;
	return true;
}
