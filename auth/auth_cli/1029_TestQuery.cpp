#include "CAuthRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

bool CAuthRelayApi::TestQuery(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp)
{
        CStr2Map inMap;
        stringstream ss;
        ss << CAuthRelayApi::AU_TEST_QUERY;

                Tools::MapCpy(paramap,inMap);
        //relay服务请求
        inMap["request_type"] = ss.str();
        //交易码
        inMap["transcode"] = TRANSCODE_TEST_QUERY;
        
        //inMap["uin"] = paramap["uin"];

        CRelayCall* m_pRelayCall = CAuthRelayApi::GetRelayObj(CAuthRelayApi::AU_TEST_QUERY) ;
        bool ret = m_pRelayCall->Call(inMap) ;

        InfoLog("send : %s", m_pRelayCall->getSendStr()) ;
        InfoLog("recv : %s", m_pRelayCall->getResultStr()) ;
        //返回结果保存容器
        if(!CAuthRelayApi::ParseResult(ret,m_pRelayCall,resultmap,true))
                return false;

    resultmap["name"] = "";
    resultmap["address"] = "";

    vector<CStr2Map> vectmap;
    GetRecFromXml(vectmap,resultmap,m_pRelayCall->getResultStr(),NULL);

   InfoLog("kkkk: %s", resultmap["name"].c_str() );
   return true ;

}
