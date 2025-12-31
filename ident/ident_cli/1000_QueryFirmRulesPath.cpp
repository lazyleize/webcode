#include "CIdentRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

bool CIdentRelayApi::QueryFirmRulesPath(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp)
{
    CStr2Map inMap;

    inMap["transcode"] = TRANSCODE_COMM_QUERY_NOTOTAL;
    inMap["reqid"] = "1817";

    string str = "nono:123456";
	if(!paramap["pid"].empty())
	{
		str += "|pid:" + paramap["pid"];
	}
    if(!paramap["order_id"].empty())
	{
		str += "|order_id:" + paramap["order_id"];
	}
    if(!paramap["mater_num"].empty())
	{
		str += "|mater_num:" + paramap["mater_num"];
	}
    if(!paramap["term_type"].empty())
	{
		str += "|term_type:" + paramap["term_type"];
	}
    inMap["fields"] = str;

    CRelayCall* m_pRelayCall = CIdentRelayApi::GetRelayObj(CIdentRelayApi::IDENT_COMM_QUERY_NOTOTAL);
            
    bool ret = m_pRelayCall->Call(inMap);

    InfoLog("send : %s", m_pRelayCall->getSendStr());
    InfoLog("recv : %s", m_pRelayCall->getResultStr());

    if (!CIdentRelayApi::ParseResult(ret, m_pRelayCall, resultmap, throwexp))
    {
        return false;
    }

    const Para_Struct paras[] = {
        {"Fpack_id"},
        {"Fdown_count"},
        {"Fpack_size"},
        {"Fversion"},
        {"Forder_id"},
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
