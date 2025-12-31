#include "CIdentRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

// 数据库操作说明：
// 一天一个记录（每个AP每天一条今日流量记录）
// 使用 INSERT ... ON DUPLICATE KEY UPDATE 语法：
//   - 如果当天没有记录：执行 INSERT
//   - 如果当天已有记录：执行 UPDATE（累加今日流量，更新最后流量和时间）
// 
// SQL示例：
// INSERT INTO t_ac_ap_today_traffic (Fac_ip, Fap_id, Fdate, Ftoday_upload, Ftoday_download, 
//                                    Ffirst_upload, Ffirst_download, Flast_upload, Flast_download, 
//                                    Ffirst_time, Flast_time, Fupdate_time)
// VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
// ON DUPLICATE KEY UPDATE 
//   Ftoday_upload = Ftoday_upload + VALUES(Ftoday_upload),
//   Ftoday_download = Ftoday_download + VALUES(Ftoday_download),
//   Flast_upload = VALUES(Flast_upload),
//   Flast_download = VALUES(Flast_download),
//   Flast_time = VALUES(Flast_time),
//   Fupdate_time = VALUES(Fupdate_time)
// 
// 注意：
// 1. 唯一索引：uk_ac_ap_date (Fac_ip, Fap_id, Fdate) 确保每个AP每天只有一条记录
// 2. 今日流量是累加的（增量方式），首次记录只在第一次插入时设置
// 3. 如果瞬时速率 <= 当前峰值，不更新数据库（减少不必要的数据库操作）

bool CIdentRelayApi::InsertAcApTodayTraffic(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp)
{
	CStr2Map inMap;
    stringstream ss;
    ss << CIdentRelayApi::IDENT_COMM_TRANSMIT;
	Tools::MapCpy(paramap,inMap);

	inMap["request_type"] = ss.str();
	inMap["transcode"] = TRANSCODE_APP_ACAPTODAYTRAFFIC; 

	CRelayCall* m_pRelayCall = CIdentRelayApi::GetRelayObj(CIdentRelayApi::IDENT_COMM_TRANSMIT);
	bool ret = m_pRelayCall->Call(inMap) ;

    InfoLog("send : %s", m_pRelayCall->getSendStr()) ;
    InfoLog("recv : %s", m_pRelayCall->getResultStr());

	if(!CIdentRelayApi::ParseResult(ret,m_pRelayCall,resultmap,throwexp))
        return false ;

    return true;
}

