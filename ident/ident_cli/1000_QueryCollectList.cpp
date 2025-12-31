#include "CIdentRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

bool CIdentRelayApi::QueryCollectList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp)
{
    CStr2Map inMap;

    inMap["transcode"] = TRANSCODE_COMM_QUERY;
    inMap["reqid"] = "1813";
    inMap["offset"] = paramap["offset"].empty() ? "0" : paramap["offset"];
    inMap["limit"] = paramap["limit"].empty() ? "0" : paramap["limit"];

	string str = "nono:123456";
	if(!paramap["pos_sn"].empty())
	{
		str += "|pos_sn:" + paramap["pos_sn"];
	}
	if(!paramap["ass_pid"].empty())
	{
		str += "|ass_pid:" + paramap["ass_pid"];
	}
	if(!paramap["result"].empty())
	{
		str += "|result:" + paramap["result"];
	}
    if(!paramap["type"].empty())
	{
		str += "|type:" + paramap["type"];
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
	if(!paramap["create_time"].empty())
	{
		str += "|create_time:" + paramap["create_time"];
	}
	if(!paramap["term_type"].empty())
	{
		str += "|term_type:" + paramap["term_type"];
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
                                {"Fpossn"},
                                {"Fass_pid"},
								{"Ftype"},
								{"Fresult"},
                                {"Fterm_type"},
                                {"Forder_id"},
                                {"Fes_state"},
                                {"Fcreate_time"},
								{"Ftime_map"},
                                {"File_path"},
        						{NULL}
    };
    resultmap["ret_num"]="0";
    resultmap["total"]="0";
    GetRecFromXml(vectmapArray,resultmap,m_pRelayCall->getResultStr(),&paras[0]);
    return true ;

}

bool CIdentRelayApi::QueryPidCollectList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp)
{
    CStr2Map inMap;

    inMap["transcode"] = TRANSCODE_COMM_QUERY;
    inMap["reqid"] = "1835";
    inMap["offset"] = paramap["offset"].empty() ? "0" : paramap["offset"];
    inMap["limit"] = paramap["limit"].empty() ? "0" : paramap["limit"];

	string str = "nono:123456";
	if(!paramap["pos_sn"].empty())
	{
		str += "|pid:" + paramap["pos_sn"];
	}
	if(!paramap["result"].empty())
	{
		str += "|result:" + paramap["result"];
	}
    if(!paramap["type"].empty())
	{
		str += "|type:" + paramap["type"];
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
                                {"Fpossn"},
                                {"Fass_pid"},
								{"Ftype"},
								{"Fresult"},
                                {"Fterm_type"},
                                {"Forder_id"},
                                {"Fes_state"},
                                {"Fcreate_time"},
								{"Ftime_map"},
                                {"File_path"},
        						{NULL}
    };
    resultmap["ret_num"]="0";
    resultmap["total"]="0";
    GetRecFromXml(vectmapArray,resultmap,m_pRelayCall->getResultStr(),&paras[0]);
    return true ;

}


bool CIdentRelayApi::QueryConfPidCollectList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp)
{
    CStr2Map inMap;

    inMap["transcode"] = TRANSCODE_COMM_QUERY;
    inMap["reqid"] = "1852";
    inMap["offset"] = paramap["offset"].empty() ? "0" : paramap["offset"];
    inMap["limit"] = paramap["limit"].empty() ? "0" : paramap["limit"];

	string str = "nono:123456";
	if(!paramap["pos_sn"].empty())
	{
		str += "|pid:" + paramap["pos_sn"];
	}
	if(!paramap["result"].empty())
	{
		str += "|result:" + paramap["result"];
	}
    if(!paramap["type"].empty())
	{
		str += "|type:" + paramap["type"];
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
                                {"Fpossn"},
                                {"Fass_pid"},
								{"Ftype"},
								{"Fresult"},
                                {"Fterm_type"},
                                {"Forder_id"},
                                {"Fes_state"},
                                {"Fcreate_time"},
								{"Ftime_map"},
                                {"File_path"},
        						{NULL}
    };
    resultmap["ret_num"]="0";
    resultmap["total"]="0";
    GetRecFromXml(vectmapArray,resultmap,m_pRelayCall->getResultStr(),&paras[0]);
    return true ;

}