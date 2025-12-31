#include "CCheckTranPwd.h"
#include "CAuthRelayApi.h"

int CCheckTranPwd::AuthCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map inMap,outMap,sessMap;
    
    //取得所有的请求参数
    pReqData->GetStrMap(inMap);

    /*--JiaYeHui --- 20151104 --- 从cess获取UID及UIN --*/
    CheckLogin(sessMap);
    inMap["uid"] = sessMap["uid"];
    inMap["uin"] = sessMap["uin"];

    /*
    char des_open[512];
    char des_enc[512];
    memset(des_open,0x00,sizeof(des_open));
    memset(des_enc,0x00,sizeof(des_enc));
    string pwdtran_open = CAuthPub::undes3(inMap["pwd_tran"]);
    //ErrorLog("[%s],[%d] 交易密码:[%s]",__FILE__,__LINE__,pwdtran_open.c_str());
    strcpy(des_open,pwdtran_open.c_str());
    CAuthPub::ads_des_idstr_see("E",1,des_open,strlen(des_open),des_enc);
    inMap["pwd_tran"] = des_enc;
    */
    inMap["pwd_tran"] = CAuthPub::WebrsaToAdsdes(1,inMap["pwd_tran"],1,TIMEOUT_RSA);

    CAuthRelayApi::CheckTranPwd(inMap,outMap,true);
    return 0;
}
