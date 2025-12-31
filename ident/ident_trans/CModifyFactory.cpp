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

	this->CheckLogin(sessionMap);

    inMap["usertype"] = sessionMap["usertype"];
    int usertype = atoi(sessionMap["usertype"].c_str());
   if(usertype != 2  && usertype != 1)
    {
        ErrorLog("没有权限: usertype=[%s]" ,inMap["usertype"].c_str());
        throw CTrsExp(ERR_NO_LOWER_LEVER,"没有权限");
    }
    
	CheckParameter(inMap);
	
	//inMap["name"] = Tools::StrToHex(inMap["name"]);

	CIdentRelayApi::ModifyFactory(inMap,outMap,true);

	return 0;
}

// 输入判断
void CPassSend::CheckParameter( CStr2Map& inMap)
{
	if(inMap["factory_id"].empty())
    {
        ErrorLog("关键字段不能为空-factory_id");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段factory_id为空");
    }
	if(inMap["name"].empty())
    {
        ErrorLog("关键字段不能为空-name");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段name为空");
    }
}
