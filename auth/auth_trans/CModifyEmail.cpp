#include "CModifyEmail.h"
#include "CAuthRelayApi.h"

int CModifyEmail::AuthCommit(CReqData* pReqData,CResData* pResData)
{
    
    CStr2Map inMap,outMap,sessMap;
    pReqData->GetStrMap(inMap);
    
    /*--JiaYeHui --- 20151104 --- 从cess获取UID及UIN --*/
    CheckLogin(sessMap);
    inMap["uid"] = sessMap["uid"];
    inMap["name"] = sessMap["uin"];

    //校验验证码
    //CheckVerifyCode(pReqData->GetPara("verify_code"));
    //检查用户是否是合法用户
    //inMap["name"] = this->m_stSessionData.szUin;
    //inMap["name"] = this->GetCookieUin();
    
/*    不需要这个查询
    //查询是否有旧邮箱
    CStr2Map outMap;
    CAuthRelayApi::QueryLoginInfo(paramap["uid"],outMap,true);
    paramap["o_email"] = outMap["email"];
*/
    CAuthRelayApi::ModifyEmail(inMap,outMap,true);
    return 0;
}
