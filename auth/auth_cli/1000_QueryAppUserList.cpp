#include "CAuthRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

bool CAuthRelayApi::QueryAppUserList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray, bool throwexp)
{
	CStr2Map inMap;

	inMap["transcode"] = TRANSCODE_COMM_QUERY;
	inMap["reqid"] = "1803";

	inMap["offset"] = paramap["offset"].empty() ? "0" : paramap["offset"];
    inMap["limit"] = paramap["limit"].empty() ? "0" : paramap["limit"];
	string str = "nono:123456";
	if(!paramap["factory_id"].empty())
	{
		str += "|factory_id:" + paramap["factory_id"];
	}
	if(!paramap["name"].empty())
	{
		str += "|name:" + paramap["name"];
	}
	if(!paramap["create_time_beg"].empty())
	{
		str += "|create_time_beg:" + paramap["create_time_beg"];
	}
	if(!paramap["create_time_end"].empty())
	{
		str += "|create_time_end:" + paramap["create_time_end"];
	}
	inMap["fields"] = str;

	CRelayCall* m_pRelayCall = CAuthRelayApi::GetRelayObj(CAuthRelayApi::CDH_COMM_QUERY);
			
	bool ret = m_pRelayCall->Call(inMap);

	InfoLog("send : %s", m_pRelayCall->getSendStr());
	InfoLog("recv : %s", m_pRelayCall->getResultStr());

	if (!CAuthRelayApi::ParseResult(ret, m_pRelayCall, resultmap, throwexp))
	{
		return false;
	}
	const Para_Struct paras[] = {
							    {"Fuid"},
								{"Fname"},
								{"Factory"},
								{"Fstate"},
								{"Fcreate_time"},
        {NULL}
    };
	resultmap["ret_num"]="0";
	resultmap["total"]="0";
	GetRecFromXml(vectmapArray,resultmap,m_pRelayCall->getResultStr(),&paras[0]);
	if(vectmapArray.size() > 0)
		resultmap.insert(vectmapArray[0].begin(),vectmapArray[0].end());
	return true ;
}

