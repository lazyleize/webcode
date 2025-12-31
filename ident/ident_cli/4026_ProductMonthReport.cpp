#include "CIdentRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

bool CIdentRelayApi::ProductMonthReport(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp)
{
    CStr2Map inMap;
    stringstream ss;
    ss << CIdentRelayApi::IDENT_COMM_TRANSMIT;
	Tools::MapCpy(paramap,inMap);

	inMap["request_type"] = ss.str();
	inMap["transcode"] = TRANSCODE_TR_PRODUCTMONTHREPORT;

	CRelayCall* m_pRelayCall = CIdentRelayApi::GetRelayObj(CIdentRelayApi::IDENT_COMM_TRANSMIT);
	bool ret = m_pRelayCall->Call(inMap) ;

    InfoLog("send : %s", m_pRelayCall->getSendStr()) ;
    InfoLog("recv : %s", m_pRelayCall->getResultStr());

    if(!CIdentRelayApi::ParseResult(ret,m_pRelayCall,resultmap,throwexp))
        return false ;

	resultmap["asspid_count"]="";//组装阶段成功且PID不同的数量
    resultmap["asspid_total"]="";//组装阶段生产次数
    resultmap["asspid_fail_total"]="";//组装阶段失败个数
    resultmap["asspid_batch_num"]="";//组装阶段不同批次号个数
    resultmap["pack_success_count"]="";//包装阶段成功且SN不同的数量
    resultmap["pack_total"]="";//包装阶段生产次数
    resultmap["pack_fail_total"]="";//包装阶段失败个数
    resultmap["pack_batch_num"]="";//包装阶段批次号不同

	vector<CStr2Map> vectmap;
	GetRecFromXml(vectmap,resultmap,m_pRelayCall->getResultStr(),NULL);
	
    return true;

}
