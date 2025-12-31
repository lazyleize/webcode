#include "CIdentRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

bool CIdentRelayApi::FirmReportCreate(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp)
{
    CStr2Map inMap;
    stringstream ss;
    ss << CIdentRelayApi::IDENT_COMM_TRANSMIT;
	Tools::MapCpy(paramap,inMap);

	inMap["request_type"] = ss.str();
	inMap["transcode"] = TRANSCODE_TR_CREATEFIRMREPORT;

	CRelayCall* m_pRelayCall = CIdentRelayApi::GetRelayObj(CIdentRelayApi::IDENT_COMM_TRANSMIT);
	bool ret = m_pRelayCall->Call(inMap) ;

    InfoLog("send : %s", m_pRelayCall->getSendStr()) ;
    InfoLog("recv : %s", m_pRelayCall->getResultStr());

    if(!CIdentRelayApi::ParseResult(ret,m_pRelayCall,resultmap,throwexp))
        return false ;

	resultmap["order_id"]="";//订单号
    resultmap["start_time"]="";//开始时间 YYYYMMDD HH:MM:SS
    resultmap["end_time"]="";//结束时间 YYYYMMDD HH:MM:SS
    resultmap["plan_down"]="";//计划下载台数
    resultmap["actual_down"]="";//实际下载次数
    resultmap["down_success"]="";//下载成功台数
    resultmap["pack_size"]="";//单次下载大小 单位M

	vector<CStr2Map> vectmap;
	GetRecFromXml(vectmap,resultmap,m_pRelayCall->getResultStr(),NULL);
	
    return true;

}
