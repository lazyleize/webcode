#include "CIdentRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

// 数据库操作说明：
// 根据时间范围、AC IP、AP ID查询流量趋势数据，并按指定间隔聚合
// 
// SQL示例（AC级别，routerId为空）：
// SELECT 
//     DATE_FORMAT(Fcollect_time, '%Y-%m-%d %H:%i:%s') as time,
//     AVG(Fupload_speed) as upload,
//     AVG(Fdownload_speed) as download
// FROM t_ac_traffic_trend
// WHERE Fac_ip = ? 
//   AND Fap_id IS NULL
//   AND Fcollect_time >= ? 
//   AND Fcollect_time <= ?
// GROUP BY UNIX_TIMESTAMP(Fcollect_time) DIV ?
// ORDER BY Fcollect_time

// 明天改
bool CIdentRelayApi::QueryAcTrafficTrendList(CStr2Map& paramap, CStr2Map& resultmap, vector<CStr2Map>& vectmapArray, bool throwexp)
{
	CStr2Map inMap;

    inMap["transcode"] = TRANSCODE_COMM_QUERY;
    inMap["reqid"] = "1863";
    inMap["offset"] = paramap["offset"].empty() ? "0" : paramap["offset"];
    inMap["limit"] = paramap["limit"].empty() ? "0" : paramap["limit"];

    string str = "nono:123456";
	if(!paramap["sn"].empty())
	{
		str += "|sn:" + paramap["sn"];
	}
	if(!paramap["factory"].empty())
	{
		str += "|factory:" + paramap["factory"];
	}
	if(!paramap["role"].empty())
	{
		str += "|role:" + paramap["role"];
	}
	if(!paramap["create_time_beg"].empty())
	{
		str += "|create_time_beg:" + paramap["create_time_beg"];
	}
	if(!paramap["create_time_end"].empty())
	{
		str += "|create_time_end:" + paramap["create_time_end"];
	}
	if(!paramap["result"].empty())
	{
		str += "|result:" + paramap["result"];
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
							    {"Fate_log_id"},
								{"Fsn"},
								{"Factory"},
								{"Frole"},
								{"File_path"},
								{"Fresult"},
								{"File_path"},
								{"Fcreate_time"},
        {NULL}
    };
    resultmap["ret_num"]="0";
    resultmap["total"]="0";
    GetRecFromXml(vectmapArray,resultmap,m_pRelayCall->getResultStr(),&paras[0]);
    return true ;
}

