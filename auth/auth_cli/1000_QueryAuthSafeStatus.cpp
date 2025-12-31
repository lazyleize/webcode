#include "CAuthRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

bool CAuthRelayApi::QueryAuthSafeStatus(const string& uid,CStr2Map& resultmap,bool throwexp)
{
	CStr2Map inMap;

	inMap["transcode"] = TRANSCODE_COMM_QUERY;
	inMap["reqid"] = "1005";

	inMap["fields"] = "uid:" + uid;

	CRelayCall* m_pRelayCall = CAuthRelayApi::GetRelayObj(CAuthRelayApi::CDH_COMM_QUERY);
			
	bool ret = m_pRelayCall->Call(inMap);

	InfoLog("send : %s", m_pRelayCall->getSendStr());
	InfoLog("recv : %s", m_pRelayCall->getResultStr());

	if (!CAuthRelayApi::ParseResult(ret, m_pRelayCall, resultmap, throwexp))
	{
		return false;
	}
	const Para_Struct paras[] = {
		{"Ftruename_status"},
		{"Ftranspwd_status"},
		{"Fmobile_status"},
		{"Femail_status"},
		{"Fcard_status"},
		{"Fanswer_status"},
		{"Fcard_no"},
		{"Fmobile"},
		{NULL}
	};
	resultmap["ret_num"]="0";
	vector<CStr2Map> vectmapArray;
	GetRecFromXml(vectmapArray,resultmap,m_pRelayCall->getResultStr(),&paras[0]);
	if(vectmapArray.size() > 0)
			resultmap.insert(vectmapArray[0].begin(),vectmapArray[0].end());
	return true ;
}

