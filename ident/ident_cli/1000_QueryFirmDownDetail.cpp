#include "CIdentRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

bool CIdentRelayApi::QueryFirmDownDetail(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp)
{
    CStr2Map inMap;

    inMap["transcode"] = TRANSCODE_COMM_QUERY_NOTOTAL;
    inMap["reqid"] = "1829";

    string str;
    str = "trans_id:" + paramap["trans_id"];
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
        {"Ftask_id"},
        {"Fpack_id"},
        {"Fmater_num"},
        {"Fpid"},
        {"Fterm_type"},
        {"Forder_id"},
        {"Fdown_type"},
        {"Fstate_node"},
        {"Fail_num"},
        {"Fis_5g"},
        {"Fbegin_time"},
        {"Fend_time"},
        {"Fmodify_time"},
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

