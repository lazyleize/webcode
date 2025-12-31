#include "CUnbindBankCard.h"
#include "CAuthRelayApi.h"

int CUnbindBankCard::AuthCommit(CReqData* pReqData,CResData* pResData)
{
        /*---add---by---zhangwuyu---20151104---begin---*/
        CStr2Map sessMap;
        /*---add---by---zhangwuyu---20151104---end---*/
	CStr2Map inMap,outMap;
	char des_open[512];
    char des_enc[512];
	
	//取得所有的请求参数
	pReqData->GetStrMap(inMap);

	//检查用户登录态
	CheckLogin(sessMap);
	
	//校验验证码
	//CheckVerifyCode(pReqData->GetPara("verify_code"));

	inMap["name"] = sessMap["uin"];
        inMap["uid"] = sessMap["uid"];
	CAuthRelayApi::QueryUserBaseInfo(inMap["uid"],outMap);
	inMap["mobile"] = outMap["Fmobile"];
    if(inMap["mobile"].empty())
    {
			ErrorLog("[%s %d]用户尚未绑定手机号: name=[%s]",
							__FILE__, __LINE__,
							inMap["name"].c_str());
			throw CTrsExp(ERR_USER_UNBIND_MOBILE,"请绑定手机号！");
    }

    /*
	string tranpwd_open = CAuthPub::undes3(inMap["pwd_tran"]);
	ErrorLog("pwd_tran_open:[%s]",tranpwd_open.c_str());
    memset(des_open,0x00,sizeof(des_open));
    memset(des_enc,0x00,sizeof(des_enc));
    strcpy(des_open,tranpwd_open.c_str());
    CAuthPub::ads_des_idstr_see("E",1,des_open,strlen(des_open),des_enc);
    inMap["pwd_tran"] = des_enc;
    */
    inMap["pwd_tran"] = CAuthPub::WebrsaToAdsdes(1,inMap["pwd_tran"],1,TIMEOUT_RSA);

	CAuthRelayApi::UnbindBankCard(inMap,outMap,true);
	return 0;
}
