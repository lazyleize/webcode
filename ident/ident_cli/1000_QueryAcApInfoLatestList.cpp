#include "CIdentRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

bool CIdentRelayApi::QueryAcApInfoLatestList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp)
{
    CStr2Map inMap;

    inMap["transcode"] = TRANSCODE_COMM_QUERY;
    inMap["reqid"] = "1861";
    inMap["offset"] = paramap["offset"].empty() ? "0" : paramap["offset"];
    inMap["limit"] = paramap["limit"].empty() ? "0" : paramap["limit"];

    string str = "nono:123456";
	if(!paramap["ap_id"].empty())
	{
		str += "|ap_id:" + paramap["ap_id"];
	}
	if(!paramap["ac_ip"].empty())
	{
		str += "|ac_ip:" + paramap["ac_ip"];
	}
	if(!paramap["status"].empty())
	{
		str += "|status:" + paramap["status"];
	}
	if(!paramap["create_time_beg"].empty())
	{
		str += "|create_time_beg:" + paramap["create_time_beg"];
	}
	if(!paramap["create_time_end"].empty())
	{
		str += "|create_time_end:" + paramap["create_time_end"];
	}
	if(!paramap["collect_time"].empty())
	{
		str += "|collect_time:" + paramap["collect_time"];
	}
	inMap["fields"] = str;

    CRelayCall* m_pRelayCall = CIdentRelayApi::GetRelayObj(CIdentRelayApi::IDENT_COMM_QUERY);
            
    bool ret = m_pRelayCall->Call(inMap);

    InfoLog("send : %s", m_pRelayCall->getSendStr());
    InfoLog("recv : %s", m_pRelayCall->getResultStr());

    if (!CIdentRelayApi::ParseResult(ret, m_pRelayCall, resultmap, throwexp))
    {
        return false;
    }

    const Para_Struct paras[] = {
							    {"Fid"},
								{"Fcollect_time"},
								{"Fac_ip"},
								{"Fap_id"},
								{"Fap_name"},
								{"Fap_ip"},
								{"Fstatus"},
                                {"Fcpu"},
                                {"Fmemory"},
                                {"Fupload"},
                                {"Fdownload"},
                                {"Frun_time"},
                                {"Fonline_time"},
                                {"Fremark"},
								{"Fcreate_time"},
        {NULL}
    };
    resultmap["ret_num"]="0";
    resultmap["total"]="0";
    GetRecFromXml(vectmapArray,resultmap,m_pRelayCall->getResultStr(),&paras[0]);
    return true ;
}

