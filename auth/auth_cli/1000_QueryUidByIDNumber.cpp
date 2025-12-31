#include "CAuthRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

string CAuthRelayApi::QueryUidByIDNumber(const string& id_no,bool throwexp)
{
	CStr2Map inMap,resultmap;

	inMap["transcode"] = TRANSCODE_COMM_QUERY;
	inMap["reqid"] = "1008";

	inMap["fields"] = "id_no:" + id_no;

	CRelayCall* m_pRelayCall = CAuthRelayApi::GetRelayObj(CAuthRelayApi::CDH_COMM_QUERY);
			
	bool ret = m_pRelayCall->Call(inMap);

	InfoLog("send : %s", m_pRelayCall->getSendStr());
	InfoLog("recv : %s", m_pRelayCall->getResultStr());

	if (!CAuthRelayApi::ParseResult(ret, m_pRelayCall, resultmap, throwexp))
	{
		return "";
	}
	const Para_Struct paras[] = {
		{"Fuid"},
		{NULL}
	};
	resultmap["ret_num"]="0";
	vector<CStr2Map> vectmapArray;
	//解析xml格式获取数据
	GetRecFromXml(vectmapArray, resultmap, m_pRelayCall->getResultStr(),&paras[0]);
	if(vectmapArray.size() > 0)
	{
			return vectmapArray[0]["Fuid"];
	}
	return "";
}

