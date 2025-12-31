#include "CIdentRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

bool CIdentRelayApi::GetTermProducInfo(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp)
{
	CStr2Map inMap;
    stringstream ss;
    ss << CIdentRelayApi::IDENT_COMM_TRANSMIT;
	Tools::MapCpy(paramap,inMap);

	inMap["request_type"] = ss.str();
	inMap["transcode"] = TRANSCODE_TR_GETTERMPRODUCINFO;

	CRelayCall* m_pRelayCall = CIdentRelayApi::GetRelayObj(CIdentRelayApi::IDENT_COMM_TRANSMIT);
	bool ret = m_pRelayCall->Call(inMap) ;

    InfoLog("send : %s", m_pRelayCall->getSendStr()) ;
    InfoLog("recv : %s", m_pRelayCall->getResultStr());

	if(!CIdentRelayApi::ParseResult(ret,m_pRelayCall,resultmap,throwexp))
        return false ;

	resultmap["total"]="";//总下载次数
    resultmap["success"]="";//出货总数
    resultmap["totalFail"]="";//失败总数
    resultmap["noSeriFail"]="";//非串口失败总数
    resultmap["SeriFail"]="";//串口类失败次数
    resultmap["difference"]="";//较上月出货数
	vector<CStr2Map> vectmap;
	GetRecFromXml(vectmap,resultmap,m_pRelayCall->getResultStr(),NULL);

    return true;

}