#include "CAuthRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

bool CAuthRelayApi::QueryGypayHmjrOrderList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp)
{
	CStr2Map inMap;
	inMap["transcode"] = TRANSCODE_COMM_QUERY;
	inMap["offset"] = paramap["offset"].empty() ? "0" : paramap["offset"];
	inMap["limit"] = paramap["limit"].empty() ? "0" : paramap["limit"];
    inMap["trans_id"] = paramap["trans_id"].empty() ? "0" : paramap["trans_id"];
	inMap["reqid"] = "1014";

	string str;
	str = "account:" + paramap["account"];
	inMap["fields"] = str;

    inMap["trade_time_beg"] = paramap["trade_time_beg"];
    inMap["trade_time_end"] = paramap["trade_time_end"];

	CRelayCall* m_pRelayCall = CAuthRelayApi::GetRelayObj(CAuthRelayApi::CDH_COMM_QUERY);
	bool ret = m_pRelayCall->Call(inMap);

	InfoLog("send : %s", m_pRelayCall->getSendStr()) ;
	InfoLog("recv : %s", m_pRelayCall->getResultStr()) ;

	if(!CAuthRelayApi::ParseResult(ret,m_pRelayCall,resultmap,throwexp))
	{
		return false ;
	}
    //SELECT Forder_id,Faccount,Famount,Fresult,Ftrade_date
	const Para_Struct paras[] = {
			{"Ftrans_id"},
			{"Faccount"},
            {"Famount"},
			{"Fresult"},
			{"Ftrade_date"},
			{"Fpay_mode"},
			{NULL}
	};
	resultmap["ret_num"]="0";
	resultmap["total"]="0";
	GetRecFromXml(vectmapArray,resultmap,m_pRelayCall->getResultStr(),&paras[0]);
	return true ;			
}
