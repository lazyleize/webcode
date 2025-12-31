#include "CModifyPwd.h"
#include "CAuthRelayApi.h"
#define OP_TYPE_PWD  "1"  //修改登录密码
#define OP_TYPE_PWD_TRAN  "2" //修改交易密码

int CModifyPwd::AuthCommit(CReqData* pReqData,CResData* pResData)
{
    CStr2Map inMap,outMap;
    CStr2Map sessMap;

    //取得所有的请求参数
	string strPostdata=pReqData->GetPostData();
	CAuthPub::parsePubRespJson(strPostdata,inMap);

    //检查用户登录态
    CheckLogin(sessMap);

	if(inMap["op_type"] == OP_TYPE_PWD) //登录密码
    {
        inMap["pwd_tran"] = CAuthPub::WebrsaToAdsdes(1,inMap["old_pwd"],0,TIMEOUT_RSA);
        inMap["pwd_tran_repeat"] = CAuthPub::WebrsaToAdsdes(1,inMap["new_pwd"],0,TIMEOUT_RSA);
    }
    else if(inMap["op_type"] == OP_TYPE_PWD_TRAN) //修改交易密码
    {
     	inMap["old_pwd"] = CAuthPub::WebrsaToAdsdes(2,inMap["old_pwd"],1,TIMEOUT_RSA);
       	inMap["new_pwd"] = CAuthPub::WebrsaToAdsdes(2,inMap["new_pwd"],1,TIMEOUT_RSA);
    }
	else
	{
		inMap["pwd_tran"] = CAuthPub::WebrsaToAdsdes(1,inMap["pwd_tran"],0,TIMEOUT_RSA);
        inMap["new_pwd"] = CAuthPub::WebrsaToAdsdes(1,inMap["pwd_tran_repeat"],0,TIMEOUT_RSA);
	}

    //更新用户密码
    CAuthRelayApi::ModifyPassword(inMap,outMap,true);
    return 0;
}
