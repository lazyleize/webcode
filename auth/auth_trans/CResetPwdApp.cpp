#include "CResetPwdApp.h"
#include "CAuthRelayApi.h"

int CRestPwdApp::AuthCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map inMap,sessMap;
    CStr2Map outMap;

    //取得所有的请求参数
	string strPostdata=pReqData->GetPostData();
	CAuthPub::parsePubRespJson(strPostdata,inMap);

	this->CheckLogin(sessMap);

	//权限校验
	inMap["usertype"] = sessMap["usertype"];
    int usertype = atoi(sessMap["usertype"].c_str());
    if(usertype != 2 && usertype != 1 )
    {
        ErrorLog("没有权限: usertype=[%s]" ,inMap["usertype"].c_str());
        throw CTrsExp(ERR_NO_LOWER_LEVER,"没有权限");
    }
    inMap["pwd_new"] = CAuthPub::WebrsaToAdsdes(2,inMap["pwd_new"],0,TIMEOUT_RSA);
    CAuthRelayApi::ResetAppPwd(inMap,outMap,true);
    
    return 0;
}
