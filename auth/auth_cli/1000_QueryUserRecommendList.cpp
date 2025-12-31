#include "CAuthRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

bool CAuthRelayApi::QueryUserRecommendList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp)
{
	CStr2Map inMap;
	inMap["transcode"] = TRANSCODE_COMM_QUERY;
	inMap["offset"] = paramap["offset"].empty() ? "0" : paramap["offset"];
	inMap["limit"] = paramap["limit"].empty() ? "0" : paramap["limit"];
	inMap["reqid"] = "1009";

	string str;
	str = "uid:" + paramap["uid"];
	inMap["fields"] = str;

	CRelayCall* m_pRelayCall = CAuthRelayApi::GetRelayObj(CAuthRelayApi::CDH_COMM_QUERY);
	bool ret = m_pRelayCall->Call(inMap);

	InfoLog("send : %s", m_pRelayCall->getSendStr()) ;
	InfoLog("recv : %s", m_pRelayCall->getResultStr()) ;

	if(!CAuthRelayApi::ParseResult(ret,m_pRelayCall,resultmap,throwexp))
	{
		return false ;
	}
	const Para_Struct paras[] = {
			{"Fname"},
			{"Fcreate_time"},
			{"Fcoupon_money"},
			{NULL}
	};
	resultmap["ret_num"]="0";
	resultmap["total"]="0";
	GetRecFromXml(vectmapArray,resultmap,m_pRelayCall->getResultStr(),&paras[0]);
	return true ;			
}
