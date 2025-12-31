#include "CAuthRelayApi.h"
#include "RuntimeGather.h"
#include "ParseXmlData.h"

bool CAuthRelayApi::SetAuthMobile(const CStr2Map& paramap,CStr2Map& resultmap,bool throwexp)
{
	CStr2Map inMap;
	stringstream ss;
	ss << AU_SET_AUTH_MOBILE;

	Tools::MapCpy(paramap,inMap);
	inMap["request_type"] = ss.str();
	inMap["transcode"] = TRANSCODE_SET_AUTH_MOBILE;

	CRelayCall* m_pRelayCall = CAuthRelayApi::GetRelayObj(CAuthRelayApi::AU_SET_AUTH_MOBILE);
	int ret = m_pRelayCall->Call(inMap);

	InfoLog("send: %s",m_pRelayCall->getSendStr());
	InfoLog("recv: %s",m_pRelayCall->getResultStr());

	//CStr2Map resultmap;
	if(!CAuthRelayApi::ParseResult(ret,m_pRelayCall,resultmap,true))
		return false;

	resultmap["mobile"]="";
    resultmap["op_type"]="";
    resultmap["op_stat"]="";

	vector<CStr2Map> vectmapArray;
    GetRecFromXml(vectmapArray,resultmap,m_pRelayCall->getResultStr(),NULL);
    if(vectmapArray.size() > 0)
    {
        resultmap.insert(vectmapArray[0].begin(),vectmapArray[0].end());
    }
	return true;
}
