#include "CCreatePwdTran.h"
#include "CAuthRelayApi.h"

int CCreatePwdTran::AuthCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map inMap,outMap,sessMap;

    //取得所有的请求参数
    pReqData->GetStrMap(inMap);
    
    /*--JiaYeHui --- 20151104 --- 从cess获取UID及UIN --*/
    CheckLogin(sessMap);
    inMap["uid"] = sessMap["uid"];
    inMap["name"] = sessMap["uin"];

    /*
    char des_open[512];
    char des_enc[512];
    string tranpwd_open = CAuthPub::undes3(inMap["pwd_tran"]);
    ErrorLog("[%s %d] 交易密码: tranpwd_open=[%s]", __FILE__, __LINE__,tranpwd_open.c_str());

    memset(des_open,0x00,sizeof(des_open));
    memset(des_enc,0x00,sizeof(des_enc));
    strcpy(des_open,tranpwd_open.c_str());
    CAuthPub::ads_des_idstr_see("E",1,des_open,strlen(des_open),des_enc);
    inMap["pwd_tran"] = des_enc;
    inMap["pwd_tran_repeat"] = des_enc;
    ErrorLog("[%s %d] 交易密码: pwd_tran=[%s]", __FILE__, __LINE__,inMap["pwd_tran"].c_str());
    */

    inMap["pwd_tran"] = CAuthPub::WebrsaToAdsdes(1,inMap["pwd_tran"],1,TIMEOUT_RSA);
    inMap["pwd_tran_repeat"] = CAuthPub::WebrsaToAdsdes(1,inMap["pwd_tran_repeat"],1,TIMEOUT_RSA);
    if(inMap["pwd_tran"] != inMap["pwd_tran_repeat"])
    {
         ErrorLog("请输入相同的交易密码 pwd_tran=[%s] pwd_tran_repeat=[%s]",
                    inMap["pwd_tran"].c_str(),inMap["pwd_tran_repeat"].c_str());
         throw CTrsExp(ERR_USER_PASSWORD,"请输入相同的交易密码");
    }

    //调用relay接口
    CAuthRelayApi::CreatePwdTran(inMap,outMap,true);
    
    return 0;
}
