#include "CIdentRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

bool CIdentRelayApi::QueryProduceRecordList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp)
{
    CStr2Map inMap;

    inMap["transcode"] = TRANSCODE_COMM_QUERY;
    inMap["reqid"] = "1800";
    inMap["offset"] = paramap["offset"].empty() ? "0" : paramap["offset"];
    inMap["limit"] = paramap["limit"].empty() ? "0" : paramap["limit"];

	//string str ="YYYY:2022-01-29";
	string str = "nono:2021-01-29";
	if(!paramap["pos_sn"].empty())
	{
		str += "|sn:" + paramap["pos_sn"];
	}
	if(!paramap["factory_id"].empty())
	{
		str += "|factory_id:" + paramap["factory_id"];
	}
	if(!paramap["log_type"].empty())
	{
		str += "|log_type:" + paramap["log_type"];
	}
	if(!paramap["pos_name"].empty())
	{
		str += "|pos_name:" + paramap["pos_name"];
	}
	if(paramap["result"]=="0")
	{
		str += "|result_ok:0";
	}
	if(paramap["result"]=="1")
	{
		str += "|result_error:1";
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
								{"Fapp_uid"},
								{"Fpos_sn"},
								{"Forder"},
								{"Factory"},
								{"Fresult"},
								{"Fcreate_time"},
								{"Flog_type"},
								{"Fmessage"},
								{"Flog_id"},
								{"Fass_pid"},
								{"Fpack_pid"},
								{"Foperator"},
								{"Fpos_name"},
        						{NULL}
    };
    resultmap["ret_num"]="0";
    resultmap["total"]="0";
    GetRecFromXml(vectmapArray,resultmap,m_pRelayCall->getResultStr(),&paras[0]);
    return true ;

}

bool CIdentRelayApi::QueryProducePIDRecordList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp)
{
    CStr2Map inMap;

    inMap["transcode"] = TRANSCODE_COMM_QUERY;
    inMap["reqid"] = "1811";
    inMap["offset"] = paramap["offset"].empty() ? "0" : paramap["offset"];
    inMap["limit"] = paramap["limit"].empty() ? "0" : paramap["limit"];

	string str = "nono:2021-01-29";
	if(!paramap["pos_sn"].empty())
	{
		str += "|pid:" + paramap["pos_sn"];
	}
	if(!paramap["factory_id"].empty())
	{
		str += "|factory_id:" + paramap["factory_id"];
	}
	if(paramap["result"]=="0")
	{
		str += "|result_ok:0";
	}
	if(!paramap["pos_name"].empty())
	{
		str += "|pos_name:" + paramap["pos_name"];
	}
	if(paramap["result"]=="2")
	{
		str += "|result_error:2";
	}
	if(!paramap["log_type"].empty())
	{
		str += "|log_type:" + paramap["log_type"];
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
								{"Fapp_uid"},
								{"Fpos_sn"},
								{"Forder"},
								{"Factory"},
								{"Fresult"},
								{"Fcreate_time"},
								{"Flog_type"},
								{"Fmessage"},
								{"Flog_id"},
								{"Fass_pid"},
								{"Foperator"},
								{"Fpos_name"},
                                {NULL}
    };
    resultmap["ret_num"]="0";
    resultmap["total"]="0";
    GetRecFromXml(vectmapArray,resultmap,m_pRelayCall->getResultStr(),&paras[0]);
    return true ;

}

bool CIdentRelayApi::QueryProduceSNRecordList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp)
{
	CStr2Map inMap;
	vector<CStr2Map> TmpvectmapArray;
	//先根据SN获取其它关联的SN，再用这个SN去查询所有生产相关联的记录
    inMap["transcode"] = TRANSCODE_COMM_QUERY;
    inMap["reqid"] = "1815";
    inMap["offset"] = paramap["offset"].empty() ? "0" : paramap["offset"];
    inMap["limit"] = paramap["limit"].empty() ? "0" : paramap["limit"];

	string str = "nono:2021-01-29";
	if(!paramap["pos_sn"].empty())
	{
		str += "|pos_sn:" + paramap["pos_sn"];
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

    const Para_Struct paras1[] = {
							    {"Fpossn"},
        						{NULL}
    };
	vector<CStr2Map> TmpVectmap;
    resultmap["ret_num"]="0";
    resultmap["total"]="0";
    GetRecFromXml(TmpVectmap,resultmap,m_pRelayCall->getResultStr(),&paras1[0]);

	int nSize = TmpVectmap.size();
	if(nSize==0)
		return true;

	for(int i = 0;i <= nSize; i++)
	{
		CStr2Map inMap;

    	inMap["transcode"] = TRANSCODE_COMM_QUERY_NOTOTAL;
    	inMap["reqid"] = "1800";
    	inMap["offset"] = paramap["offset"].empty() ? "0" : paramap["offset"];
    	inMap["limit"] = paramap["limit"].empty() ? "0" : paramap["limit"];

		string str = "nono:2021-01-29";
		if(!paramap["pos_sn"].empty())
		{
			str += "|sn:" + TmpVectmap[i]["pos_sn"];
		}
		if(!paramap["factory_id"].empty())
		{
			str += "|factory_id:" + paramap["factory_id"];
		}
		if(paramap["result"]=="")
		{
		}
		if(paramap["result"]=="0")
		{
			str += "|result_ok:0";
		}
		if(paramap["result"]=="1")
		{
			str += "|result_error:1";
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
									{"Fapp_uid"},
									{"Fpos_sn"},
									{"Forder"},
									{"Factory"},
									{"Fresult"},
									{"Fcreate_time"},
									{"Flog_type"},
									{"Fmessage"},
									{"Flog_id"},
									{"Fass_pid"},
									{"Fpack_pid"},
									{"Foperator"},
									{"Fpos_name"},
        							{NULL}
    	};
		GetRecFromXml(vectmapArray,resultmap,m_pRelayCall->getResultStr(),&paras[0]);
	}
	//int转string
	/*resultmap["ret_num"]="0";
    resultmap["total"]="0";*/
    return true ;
}