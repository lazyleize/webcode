#include "CDisableApp.h"
#include "CAuthRelayApi.h"

int CDisableApp::AuthCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map inMap;
    CStr2Map outMap,sessionMap;

    //取得所有的请求参数
    pReqData->GetStrMap(inMap);
   
	CheckParameter(inMap);

	//检查用户登录态
    //CheckLogin(sessionMap);

	/*inMap["usertype"] = sessionMap["usertype"];
    int usertype = atoi(sessionMap["usertype"].c_str());
    if(usertype != 2 && usertype != 1)
    {
        ErrorLog("没有权限: usertype=[%s]" ,inMap["usertype"].c_str());
        throw CTrsExp(ERR_NO_LOWER_LEVER,"没有权限");
    }*/

    CAuthRelayApi::DisableAppState(inMap,outMap,true);
    
    return 0;
}

void CDisableApp::CheckParameter(CStr2Map& inMap)
{
	if(inMap["app_uid"].empty())
    {
        ErrorLog("关键字段不能为空-app_uid");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段app_uid为空");
    }
    if(inMap["update_state"].empty())
    {
        ErrorLog("关键字段不能为空-update_state");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段update_state为空");
    }
}