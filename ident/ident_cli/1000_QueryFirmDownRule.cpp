#include "CIdentRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

bool CIdentRelayApi::QueryFirmDownRule(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp)
{
    CStr2Map inMap;

    inMap["transcode"] = TRANSCODE_COMM_QUERY_NOTOTAL;
    inMap["reqid"] = "1830";

    string str;
    str = "pack_id:" + paramap["pack_id"];
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
        {"Fversion"},
        {"Fap_num"},
        {"Fgo_num"},
        {"Fsvn_count"},
        {"Fsvn_path"},
        {"Fdown_count"},
        {"Fpack_size"},
        {"Fprogress"},
        {"Ftime_spent"},
        {"Fstate"},
        {"Fcreate_time"},
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

