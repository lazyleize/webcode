#include "CModifyPwdApp.h"
#include "CAuthRelayApi.h"
#define OP_TYPE_PWD  "1"  //修改登录密码
#define OP_TYPE_PWD_TRAN  "2" //修改交易密码

int CModifyPwdApp::AuthCommit(CReqData* pReqData,CResData* pResData)
{
    CStr2Map inMap,outMap;
    CStr2Map sessMap;

    //取得所有的请求参数
	string strPostdata=pReqData->GetPostData();
	CAuthPub::parsePubRespJson(strPostdata,inMap);

    //检查用户登录态
    CheckLogin(sessMap);

	inMap["usertype"] = sessMap["usertype"];
    int usertype = atoi(sessMap["usertype"].c_str());
    if(usertype != 2 && usertype != 1)
    {
        ErrorLog("没有权限: usertype=[%s]" ,inMap["usertype"].c_str());
        throw CTrsExp(ERR_NO_LOWER_LEVER,"没有权限");
    }


	inMap["pwd_tran"] = CAuthPub::WebrsaToAdsdes(2,inMap["pwd_tran"],0,TIMEOUT_RSA);
	inMap["pwd_new"] = CAuthPub::WebrsaToAdsdes(2,inMap["pwd_new"],0,TIMEOUT_RSA);


    //更新用户密码
    CAuthRelayApi::ModifyAppPwd(inMap,outMap,true);
    return 0;
}
