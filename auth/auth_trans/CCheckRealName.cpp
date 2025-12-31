#include "CCheckRealName.h"
#include "CAuthRelayApi.h"

int CCheckRealName::AuthCommit(CReqData *pReqData, CResData *pResData)
{
	//检查用户登录态
        /*---add---by---zhangwuyu---20151104---begin---*/
        CStr2Map sessMap;
        /*---add---by---zhangwuyu---20151104---end---*/
    CheckLogin(sessMap);
	
	CStr2Map inMap,outMap;
	//取得所有的请求参数
	pReqData->GetStrMap(inMap);

	
	//查询用户是否实名
	CAuthRelayApi::QueryLoginInfo(sessMap["uin"],outMap,true);
	
	string is_realname = outMap["is_realname"].empty()?"0":outMap["is_realname"];
	
/*	目前可以认为用户登录信息表是关键信息表 不用理睬用户实名表 那个是核心表 不允许私自查询
	//查询用户实名信息
	CAuthRelayApi::QueryUserInfo(this->GetCookieUin(),outMap,true);

*/
	//判断用户是否实名
	if(is_realname == "1")
	{
		InfoLog("该用户已进行实名认证,uin:[%s]",sessMap["uin"].c_str());
		pResData->SetPara("is_realname",is_realname);
	}else{
		InfoLog("该用户未进行实名认证,uin:[%s]",sessMap["uin"].c_str());
		pResData->SetPara("is_realname","0");
	}
	return 0;
}
