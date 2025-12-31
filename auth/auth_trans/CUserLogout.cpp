#include "CUserLogout.h"
#include "CAuthRelayApi.h"
#include "cgicomm/adsmem_rules.h"
#include "tlib_all.h"

int CUserLogout::AuthCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map sessMap;
	CStr2Map inMap;
	//检查用户登录态
    CheckLogin(sessMap);
	inMap["uin"] = sessMap["uin"];
	
	//更新数据库用户上次和本次登录ip和时间
    if(!CAuthRelayApi::UpdateUserLogout(inMap))
    {
        ErrorLog("[%s %d]更新用户退出信息失败:uin=[%s]",
                                __FILE__, __LINE__,
                                inMap["uin"].c_str());
                throw CTrsExp(ERR_UPDATE_USER_LOGIN,"系统繁忙");
	}		

	string strSessionKey = pReqData->GetCookie("xskey");
	//清空session数据
    string server_name = pReqData->GetEnv("ServerName") ;
    char domain[50];
    Tools::ServernameToDomain((char *)server_name.c_str(),domain);
    DeleteLoginSession(strSessionKey,domain);

	set_cookie("utp", "",  "Thu, 01 Jan 1970 00:00:00 GMT", "/", domain,  0);

	return 0;
}
