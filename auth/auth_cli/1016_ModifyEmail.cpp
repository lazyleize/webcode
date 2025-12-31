#include "CAuthRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"

bool CAuthRelayApi::ModifyEmail(const CStr2Map& paramap,CStr2Map& resultmap,bool throwexp)
{
	CStr2Map inMap;
	stringstream ss;
	ss << AU_MODIFY_EMAIL;

	Tools::MapCpy(paramap,inMap);
	inMap["request_type"] = ss.str();
	inMap["transcode"] = TRANSCODE_MODIFY_EMAIL;

	CRelayCall* m_pRelayCall = CAuthRelayApi::GetRelayObj(CAuthRelayApi::AU_MODIFY_EMAIL);
	int ret = m_pRelayCall->Call(inMap);

	InfoLog("send: %s",m_pRelayCall->getSendStr());
	InfoLog("recv: %s",m_pRelayCall->getResultStr());

	//CStr2Map resultmap;
	if(!CAuthRelayApi::ParseResult(ret,m_pRelayCall,resultmap,true))
		return false;
	return true;
}
