#include "CCreateAppUser.h"
#include "CAuthRelayApi.h"

int CCreateAppUser::AuthCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,sessionMap,outMap;

	//取得所有的请求参数
	string strPostdata=pReqData->GetPostData();
	CAuthPub::parsePubRespJson(strPostdata,inMap);

	this->CheckLogin(sessionMap);

	inMap["usertype"] = sessionMap["usertype"];
    int usertype = atoi(sessionMap["usertype"].c_str());
    if(usertype != 2  && usertype != 1)
    {
        ErrorLog("没有权限: usertype=[%s]" ,inMap["usertype"].c_str());
        throw CTrsExp(ERR_SYS_ERROR,"没有权限");
    }

    inMap["pwd"] = CAuthPub::WebrsaToAdsdes(2,inMap["pwd"],0,TIMEOUT_RSA);
    inMap["pwd_repeat"] = CAuthPub::WebrsaToAdsdes(2,inMap["pwd_repeat"],0,TIMEOUT_RSA);
    if(inMap["pwd"] != inMap["pwd_repeat"])
    {
         ErrorLog("请输入相同的登录密码 pwd=[%s] pwd_repeat=[%s]",
                    inMap["pwd"].c_str(),inMap["pwd_repeat"].c_str());
         throw CTrsExp(ERR_USER_PASSWORD,"请输入相同的登录密码");
    }

	//调用relay接口
	CAuthRelayApi::CreateAppUser(inMap,outMap,true);
	
	return 0;
}

// 输入判断
void CCreateAppUser::CheckParameter( CStr2Map& inMap)
{
	if(inMap["name"].empty())
    {
        ErrorLog("关键字段不能为空-name");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段name为空");
    }
	if(inMap["name"].length() > 19)
    {
        ErrorLog("账号太长");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"账号超出长度限制");
    }
    if(inMap["pwd"].empty())
    {
        ErrorLog("关键字段不能为空-pwd");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段pwd为空");
    }
	if(inMap["pwd_repeat"].empty())
    {
        ErrorLog("关键字段不能为空-pwd_repeat");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段pwd_repeat为空");
    }
	if(inMap["factory"].empty())
    {
        ErrorLog("关键字段不能为空-factory");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段factory为空");
    }
	if(inMap["factory_id"].empty())
    {
        ErrorLog("关键字段不能为空-factory_id");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段factory_id为空");
    }
}
