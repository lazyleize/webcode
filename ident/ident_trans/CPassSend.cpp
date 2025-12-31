#include "CPassSend.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"

int CPassSend::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,outMap,sessionMap;
	string uid;

	//取得所有的请求参数
	string strPostdata=pReqData->GetPostData();
	CIdentPub::parsePubRespJson(strPostdata,inMap);

	//检查输入参数
	CheckParameter(inMap);

	// 间隔不能超过30s
	CIdentPub::CheckTransTime(inMap["trans_time"],30);
    
	//暂时屏蔽，方便调试
	this->CheckAppLogin(sessionMap,inMap["token"],uid);

	CIdentRelayApi::IdentTrans(inMap,outMap,true);
	pResData->SetPara("context",outMap["rebuf"]);
	pResData->SetPara("uid",uid);

	return 0;
}

// 输入判断
void CPassSend::CheckParameter( CStr2Map& inMap)
{
	if(inMap["context"].empty())
    {
        ErrorLog("关键字段不能为空-context");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段context为空");
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
	if(inMap["safe_addr"].empty())
	{
		ErrorLog("关键字段不能为空-safe_addr");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段safe_addr为空");
	}
}

