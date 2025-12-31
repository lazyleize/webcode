#include "CFirmCheckHash.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"
#include "xml/unicode.h"

int CFirmCheckHash::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,tmpMap,outMap;

	// 取得请求参数
    pReqData->GetStrMap(inMap);
	
	//检查输入参数
	CheckParameter(inMap);

    if(!CIdentRelayApi::CheckFirmDownLoadHash(inMap,tmpMap,true))
    {
        ErrorLog("hash校验失败");
		//CIdentPub::SendAlarm2("hash校验失败[%s]",ERR_SIGNATURE_INCORRECT);
        //throw CTrsExp(ERR_SIGNATURE_INCORRECT,"hash校验失败");
    }
    
	return 0;
}

// 输入判断
void CFirmCheckHash::CheckParameter( CStr2Map& inMap)
{
	if(inMap["id"].empty())
    {
        ErrorLog("id缺少");
		CIdentPub::SendAlarm2("id缺少[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"id缺少");
    }
    if(inMap["trans_id"].empty())
    {
        ErrorLog("trans_id缺少");
		CIdentPub::SendAlarm2("trans_id缺少[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"trans_id缺少");
    }
}
