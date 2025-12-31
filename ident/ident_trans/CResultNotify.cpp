#include "CResultNotify.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"

int CResultNotify::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,outMap,sessionMap;
	string uid;

	//取得所有的请求参数
	string strPostdata=pReqData->GetPostData();
	CIdentPub::parsePubRespJson(strPostdata,inMap);

	//检查输入参数
	CheckParameter(inMap);

	//暂时屏蔽，方便调试
	this->CheckAppLogin(sessionMap,inMap["token"],uid);

	CIdentRelayApi::RecordLog(inMap,outMap,true);

	return 0;
}

// 输入判断
void CResultNotify::CheckParameter( CStr2Map& inMap)
{
	if(inMap["uid"].empty())
    {
        ErrorLog("关键字段不能为空-uid");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段uid为空");
    }
    if(inMap["trans_time"].empty())
    {
        ErrorLog("关键字段不能为空-trans_time");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段trans_time为空");
    }
    if(inMap["trans_time"].length() != 14)           
    {
        ErrorLog("关键字段不能为空-trans_time长度错误");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段trans_time长度错误");
    }
	if(inMap["token"].empty())
    {
        ErrorLog("关键字段不能为空-token");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段token为空");
    }
	if(inMap["log_id"].empty())
	{
		ErrorLog("关键字段不能为空-log_id");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段log_id为空");
	}
	if(inMap["result"].empty())
	{
		ErrorLog("关键字段不能为空-result");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段result为空");
	}
	if(inMap["log_type"].empty())
	{
		ErrorLog("关键字段不能为空-log_type");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段log_type为空");
	}
	if(!inMap["flash_id"].empty() && inMap["flash_id"].length() > 128)
	{
		string strtmp = inMap["flash_id"];
		inMap["flash_id"] = strtmp.substr(0,125);
	}
}
