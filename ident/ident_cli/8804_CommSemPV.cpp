#include "CIdentRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

bool CIdentRelayApi::CommSemPV(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp)
{
    CStr2Map inMap;
    stringstream ss;
    ss << CIdentRelayApi::IDENT_COMM_BATCH;
    Tools::MapCpy(paramap,inMap);
    
    //relay服务请求
    inMap["request_type"] = ss.str();

    //交易码
    inMap["transcode"] = TRANSCODE_ID_SEM_PV;

    CRelayCall* m_pRelayCall = CIdentRelayApi::GetRelayObj(CIdentRelayApi::IDENT_COMM_BATCH) ;
    bool ret = m_pRelayCall->Call(inMap) ;

    InfoLog("send : %s", m_pRelayCall->getSendStr()) ;
    InfoLog("recv : %s", m_pRelayCall->getResultStr());

    if(!CIdentRelayApi::ParseResult(ret,m_pRelayCall,resultmap,throwexp))
    {
        return false ;
    }
  
    resultmap["pv_result"] = "";
    resultmap["sem_state"] = "";

    vector<CStr2Map> vectmap;
    GetRecFromXml(vectmap,resultmap,m_pRelayCall->getResultStr(),NULL);
  
    return true;
}


