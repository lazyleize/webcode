#include "CAlarmSet.h"
#include "CIdentRelayApi.h"

int CAlarmSet::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,outMap;
	string uid;
	// 取得请求参数
    string strPostdata=pReqData->GetPostData();
	CIdentPub::parsePubRespJson(strPostdata,inMap);
    CheckParameter(inMap);

    CIdentRelayApi::SetAlarm(inMap,outMap,true);
	
	
	return 0;

}


// 输入判断
void CAlarmSet::CheckParameter( CStr2Map& inMap)
{
	if(inMap["type"].empty())
    {
        ErrorLog("告警消息设置,type为空");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"告警消息设置,type为空");
	}
    if(inMap["method"].empty())
    {
        ErrorLog("告警消息设置,结果为空-method");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"警消息设置,结果为空-method");
	}
	if(inMap["node"].empty())
    {
        ErrorLog("告警消息设置,node为空");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"告警消息设置,node为空");
	}
}

