#include "CIdentRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

bool CIdentRelayApi::QueryFirmRulesList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp)
{
    CStr2Map inMap;

    inMap["transcode"] = TRANSCODE_COMM_QUERY;
    inMap["reqid"] = "1816";
    inMap["offset"] = paramap["offset"].empty() ? "0" : paramap["offset"];
    inMap["limit"] = paramap["limit"].empty() ? "0" : paramap["limit"];

    string str = "nono:123456";
	if(!paramap["pack_id"].empty())
	{
		str += "|pack_id:" + paramap["pack_id"];
	}
    if(!paramap["order_id"].empty())
	{
		str += "|order_id:" + paramap["order_id"];
	}
	if(!paramap["create_time_beg"].empty())
	{
		str += "|create_time_beg:" + paramap["create_time_beg"];
	}
	if(!paramap["create_time_end"].empty())
	{
		str += "|create_time_end:" + paramap["create_time_end"];
	}
	if(!paramap["state"].empty())
	{
		str += "|state:" + paramap["state"];
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
							    {"Fpack_id"}, 
                                {"Fall_id"},
                                {"Fmater_num"},
                                {"Fpid_head"}, 
                                {"Fpid_start"}, 
                                {"Fpid_end"}, 
                                {"Fpid_num_total"}, 
                                {"Fpid_complete_num"},
                                {"Fterm_type"},
                                {"Fterm_down_total"}, 
                                {"Forder_id"}, 
                                {"Fsvn_count"},
                                {"Fsvn_path"}, 
                                {"Fpack_size"}, 
                                {"Fprogress"}, 
                                {"Fstate"}, 
                                {"Fcreate_time"}, 
                                {"Fmodify_time"}, 
                                {"Ftime_spent"}, 
                                {NULL}
    };
    resultmap["ret_num"]="0";
    resultmap["total"]="0";
    GetRecFromXml(vectmapArray,resultmap,m_pRelayCall->getResultStr(),&paras[0]);
    return true ;
}

