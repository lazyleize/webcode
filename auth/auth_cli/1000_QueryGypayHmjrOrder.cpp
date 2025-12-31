#include "CAuthRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

bool CAuthRelayApi::QueryGypayHmjrOrder(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp)
{
	CStr2Map inMap;

	inMap["transcode"] = TRANSCODE_COMM_QUERY;
	inMap["reqid"] = "1013";
	inMap["account"] = paramap["account"].empty() ? "0" : paramap["account"];

	inMap["fields"] = "trans_id:" + paramap["trans_id"];

	CRelayCall* m_pRelayCall = CAuthRelayApi::GetRelayObj(CAuthRelayApi::CDH_COMM_QUERY);
			
	bool ret = m_pRelayCall->Call(inMap);

	InfoLog("send : %s", m_pRelayCall->getSendStr());
	InfoLog("recv : %s", m_pRelayCall->getResultStr());

	if (!CAuthRelayApi::ParseResult(ret, m_pRelayCall, resultmap, throwexp))
	{
		return false;
	}

    //SELECT Ftrans_id,Faccount,Famount,Fresult,Ftrade_date,Ffact_money,Fsucc_time,Fpay_mode

	const Para_Struct paras[] = {
		{"Ftrans_id"},
		{"Faccount"},
		{"Famount"},
		{"Fresult"},
		{"Ftrade_date"},
        {"Ffact_money"},
        {"Fsucc_time"},
        {"Fpay_mode"},
		{NULL}
	};
	resultmap["ret_num"]="0";
	vector<CStr2Map> vectmapArray;
	//解析xml格式获取数据
	GetRecFromXml(vectmapArray, resultmap, m_pRelayCall->getResultStr(),&paras[0]);
	if(vectmapArray.size() > 0)
	{
		resultmap.insert(vectmapArray[0].begin(),vectmapArray[0].end());
	}
	return true;
}

