#include "CAppUserReg.h"
#include "CAuthRelayApi.h"

int CAppUserReg::AuthCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map inMap;
    CStr2Map outMap;

    //取得所有的请求参数
    pReqData->GetStrMap(inMap);
    /*
    char des_open[512],des_open2[512];
    char des_enc[512],des_enc2[512];
    string pwd_open = CAuthPub::undes3(inMap["pwd"]);
    memset(des_open,0x00,sizeof(des_open));
    memset(des_enc,0x00,sizeof(des_enc));
    strcpy(des_open,pwd_open.c_str());
    CAuthPub::ads_des_idstr_see("E",0,des_open,strlen(des_open),des_enc);
    inMap["pwd"] = des_enc;

    string pwd_open2 = CAuthPub::undes3(inMap["pwd_repeat"]);
    memset(des_open2,0x00,sizeof(des_open2));
    memset(des_enc2,0x00,sizeof(des_enc2));
    strcpy(des_open2,pwd_open2.c_str());
    CAuthPub::ads_des_idstr_see("E",0,des_open2,strlen(des_open2),des_enc2);
    inMap["pwd_repeat"] = des_enc2;
    */

    inMap["pwd"] = CAuthPub::WebrsaToAdsdes(1,inMap["pwd"],0,TIMEOUT_RSA);
    inMap["pwd_repeat"] = CAuthPub::WebrsaToAdsdes(1,inMap["pwd_repeat"],0,TIMEOUT_RSA);

    if(inMap["pwd"] != inMap["pwd_repeat"])
    {   
         ErrorLog("请输入相同的登录密码 pwd=[%s] pwd_repeat=[%s]",
                    inMap["pwd"].c_str(),inMap["pwd_repeat"].c_str());
         throw CTrsExp(ERR_USER_PASSWORD,"请输入相同的登录密码"); 
    }   
   
    CAuthRelayApi::AppUserReg(inMap,outMap,true);
    
    return 0;
}
