#include "CCreateUserInfo.h"
#include "CAuthRelayApi.h"

int CCreateUserInfo::AuthCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap, outMap,sessionMap;

	//取得所有的请求参数
	string strPostdata=pReqData->GetPostData();
	CAuthPub::parsePubRespJson(strPostdata,inMap);

	this->CheckLogin(sessionMap);

	CheckParameter(inMap);

	inMap["usertype"] = sessionMap["usertype"];
    int usertype = atoi(sessionMap["usertype"].c_str());
	if(usertype != 2  && usertype != 1)
    {
        ErrorLog("没有权限: usertype=[%s]" ,inMap["usertype"].c_str());
        throw CTrsExp(ERR_DATA_FORMAT,"没有权限");
    }
	if(usertype == 2)
	{
		if(atoi(inMap["user_type"].c_str()) != 3)
		{
			ErrorLog("没有权限: usertype=[%s]" ,inMap["usertype"].c_str());
			throw CTrsExp(ERR_OPT,"没有权限");
		}
		if(inMap["relation"].empty())
		{
			ErrorLog("关键字段不能为空-relation");
			throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段relation为空");
		}
	}

   
    inMap["pwd"] = CAuthPub::WebrsaToAdsdes(1,inMap["pwd"],0,TIMEOUT_RSA);
    inMap["pwd_repeat"] = CAuthPub::WebrsaToAdsdes(1,inMap["pwd_repeat"],0,TIMEOUT_RSA);
    if(inMap["pwd"] != inMap["pwd_repeat"])
    {
         ErrorLog("请输入相同的登录密码 pwd=[%s] pwd_repeat=[%s]",
                    inMap["pwd"].c_str(),inMap["pwd_repeat"].c_str());
         throw CTrsExp(ERR_USER_PASSWORD,"请输入相同的登录密码");
    }

	int relation = atoi(inMap["relation"].c_str());
	//调用relay接口
	if(relation != 1)
		CAuthRelayApi::CreateUserInfo(inMap,outMap,true);
	
	if(relation == 2 || relation == 1)
		CAuthRelayApi::CreateAppUser(inMap,outMap,true);
	
	return 0;
}

// 输入判断
void CCreateUserInfo::CheckParameter( CStr2Map& inMap)
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
    if(inMap["user_type"].empty())
    {
        ErrorLog("关键字段不能为空-user_type");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段user_type为空");
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
	
}
