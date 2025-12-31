#include "CSetNewPwd.h"
#include "CAuthRelayApi.h"
#define OP_TYPE_PWD  "1"  //找回登录密码
#define OP_TYPE_PWD_TRAN  "2" //找回交易密码

int CSetNewPwd::AuthCommit(CReqData* pReqData,CResData* pResData)
{
	CStr2Map inMap,outMap;
	char des_open[512];
    char des_enc[512];

    //取得所有的请求参数
    pReqData->GetStrMap(inMap);

    inMap["uid"] = CAuthRelayApi::QueryUidByUin(pReqData->GetPara("uin"));
    if(inMap["uid"].empty())
    {
        ErrorLog("[%s],[%d]行 该用户不存在 uin:[%s]",__FILE__,__LINE__,inMap["uin"].c_str());
        throw CTrsExp(ERR_UNKNOWN_USER,"不存在的用户");
    }
	//inMap["name"] = this->GetCookieUin();
	//检查新密码与确认密码是否一致
	/*
	if(inMap["new_pwd"] != inMap["repeat_pwd"])
	{
		ErrorLog("[%s],[%d]行 新密码与确认密码不一致 new_pwd:[%s],repeat_pwd:[%s]",
						__FILE__,__LINE__,inMap["new_pwd"].c_str(),inMap["repeat_pwd"].c_str());
        throw CTrsExp(ERR_DISACCORD_PWD,"新密码与确认密码不一致");
	}
	*/
	if(inMap["op_type"] == OP_TYPE_PWD) //找回登录密码
    {
            /*
            memset(des_open,0x00,sizeof(des_open));
            memset(des_enc,0x00,sizeof(des_enc));
            string newpwd_open = CAuthPub::undes3(inMap["new_pwd"]);
			ErrorLog("[%s],[%d]  new_pwd:[%s]",__FILE__,__LINE__,newpwd_open.c_str());
            strcpy(des_open,newpwd_open.c_str());
            CAuthPub::ads_des_idstr_see("E",0,des_open,strlen(des_open),des_enc);
            inMap["new_pwd"] = des_enc;
            */
        inMap["new_pwd"] = CAuthPub::WebrsaToAdsdes(1,inMap["new_pwd"],0,TIMEOUT_RSA);
    }
    else if(inMap["op_type"] == OP_TYPE_PWD_TRAN) //找回交易密码
    {
            /*
            memset(des_open,0x00,sizeof(des_open));
            memset(des_enc,0x00,sizeof(des_enc));

            string newpwd_open = CAuthPub::undes3(inMap["new_pwd"]);
			ErrorLog("[%s],[%d]  new_pwd:[%s]",__FILE__,__LINE__,newpwd_open.c_str());

            strcpy(des_open,newpwd_open.c_str());
            CAuthPub::ads_des_idstr_see("E",1,des_open,strlen(des_open),des_enc);
            inMap["new_pwd"] = des_enc;
            DebugLog("[%s:%d] des_enc=[%s]",__FILE__,__LINE__,des_enc);
            */
        inMap["new_pwd"] = CAuthPub::WebrsaToAdsdes(1,inMap["new_pwd"],1,TIMEOUT_RSA);
    }
	//更新用户密码
	CAuthRelayApi::SetNewPassword(inMap,outMap,true);
	return 0;
}
