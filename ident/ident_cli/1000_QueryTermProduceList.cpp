#include "CIdentRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

bool CIdentRelayApi::QueryTermProduceList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp)
{
    CStr2Map inMap;

    inMap["transcode"] = TRANSCODE_COMM_QUERY;
    inMap["reqid"] = "1848";
    inMap["offset"] = paramap["offset"].empty() ? "0" : paramap["offset"];
    inMap["limit"] = paramap["limit"].empty() ? "0" : paramap["limit"];

    string str = "nono:123456";
	if(!paramap["pos_sn"].empty())
	{
		str += "|pos_sn:" + paramap["pos_sn"];
	}
	if(!paramap["term_name"].empty())
	{
		str += "|term_name:" + paramap["term_name"];
	}
    if(!paramap["order"].empty())
	{
		str += "|order:" + paramap["order"];
	}
    if(!paramap["smt_result"].empty())
	{
		str += "|smt_result:" + paramap["smt_result"];
	}
    if(!paramap["fac_result"].empty())
	{
		str += "|fac_result:" + paramap["fac_result"];
	}
    if(!paramap["pack_result"].empty())
	{
		str += "|pack_result:" + paramap["pack_result"];
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
                                {"Forder"},
                                {"Fass_pid"},
                                {"Fpack_pid"},
                                {"Fterm_name"},
                                {"Fpos_sn"},
                                {"Fsmt_result"},
                                {"Fac_result"},
                                {"Fpack_result"},
                                {"Fupdate_time"},
        {NULL}
    };
    resultmap["ret_num"]="0";
    resultmap["total"]="0";
    GetRecFromXml(vectmapArray,resultmap,m_pRelayCall->getResultStr(),&paras[0]);
    return true ;
}

