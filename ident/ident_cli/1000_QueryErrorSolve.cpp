#include "CIdentRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

bool CIdentRelayApi::QueryErrorSolve(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp)
{
    CStr2Map inMap;

    inMap["transcode"] = TRANSCODE_COMM_QUERY_NOTOTAL;
    inMap["reqid"] = "1809";

    string str;
    str = "result:" + paramap["Fresult"];
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
        {"Fsolve"},
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
