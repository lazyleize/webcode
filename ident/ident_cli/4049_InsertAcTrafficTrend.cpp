#include "CIdentRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

// 数据库操作说明：
// 每分钟一条记录（每个AC/AP每分钟一条流量趋势记录）
// 使用 INSERT ... ON DUPLICATE KEY UPDATE 语法：
//   - 如果该分钟没有记录：执行 INSERT
//   - 如果该分钟已有记录：执行 UPDATE（更新流量速率）
// 
// SQL示例：
// INSERT INTO t_ac_traffic_trend (Fcollect_time, Fac_ip, Fap_id, Fupload_speed, Fdownload_speed)
// VALUES (?, ?, ?, ?, ?)
// ON DUPLICATE KEY UPDATE 
//   Fupload_speed = VALUES(Fupload_speed),
//   Fdownload_speed = VALUES(Fdownload_speed)
// 
// 注意：
// 1. 唯一索引：uk_ac_ap_time (Fac_ip, Fap_id, Fcollect_time) 确保每个AC/AP每分钟只有一条记录
// 2. Fap_id为NULL表示AC级别数据，非NULL表示AP级别数据
// 3. Fcollect_time精确到分钟（格式：YYYY-MM-DD HH:MM:00）

bool CIdentRelayApi::InsertAcTrafficTrend(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp)
{
	CStr2Map inMap;
    stringstream ss;
    ss << CIdentRelayApi::IDENT_COMM_TRANSMIT;
	Tools::MapCpy(paramap,inMap);

	inMap["request_type"] = ss.str();
	inMap["transcode"] = TRANSCODE_APP_ACTRAFFICTREND; 

	CRelayCall* m_pRelayCall = CIdentRelayApi::GetRelayObj(CIdentRelayApi::IDENT_COMM_TRANSMIT);
	bool ret = m_pRelayCall->Call(inMap) ;

    InfoLog("send : %s", m_pRelayCall->getSendStr()) ;
    InfoLog("recv : %s", m_pRelayCall->getResultStr());

	if(!CIdentRelayApi::ParseResult(ret,m_pRelayCall,resultmap,throwexp))
        return false ;

    return true;
}

