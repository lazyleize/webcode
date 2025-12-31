#include "CQueryAtmLog.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"
#include "xml/unicode.h"

int CQueryAtmLog::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,tmpMap,outMap,sessionMap;
	string uid;

	// 取得请求参数
    pReqData->GetStrMap(inMap);

	this->CheckLogin(sessionMap);
	
	//检查输入参数
	CheckParameter(inMap);

	tmpMap["ate_log_id"] = inMap["ate_log_id"];
	CIdentRelayApi::GetAtmLogPath(tmpMap,outMap,true);

	CStr2Map returnMap;
	CIdentComm::DelMapF(outMap,returnMap);
	string  downLoadPath = returnMap["ile_path"];
	pResData->SetPara("file_path",downLoadPath);
	return 0;
}

// 输入判断
void CQueryAtmLog::CheckParameter( CStr2Map& inMap)
{
	if(inMap["ate_log_id"].empty())
    {
        ErrorLog("关键字段不能为空-ate_log_id");
		CIdentPub::SendAlarm2("Key Fields-ate_log_id is empty[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段ate_log_id为空");
	}
}
