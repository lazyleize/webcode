#include "CSetPwdQuestion.h"
#include "CAuthRelayApi.h"

int CSetPwdQuestion::AuthCommit(CReqData* pReqData,CResData* pResData)
{
    /*---add---by---zhangwuyu---20151104---begin---*/
    CStr2Map sessMap;
    /*---add---by---zhangwuyu---20151104---end---*/
    CStr2Map inMap,outMap;
    //取得所有的请求参数
    pReqData->GetStrMap(inMap);
    //检查用户登录态
    CheckLogin(sessMap);

    //检查用户是否是合法用户
    /*paramap["uid"] = CAuthRelayApi::QueryUidByUin(this->GetCookieUin());
    if(paramap["uid"].empty())
    {
        ErrorLog("该用户非法登录,uin:[%s] 用户登录ip:[%s]"
                ,this->GetCookieUin().c_str(),this->GetClientIp().c_str());
        throw CTrsExp(ERR_USER_NOT_REGISTER,"该用户未注册");
    }
    */
    inMap["name"] = sessMap["uin"];
    //paramap["uin"] = this->m_stSessionData.szUin;
    
    char buff[1024] = {0} ;
    int  iDestLen = sizeof(buff)-1;
    //Tools::ConvertCharSet((char*)inMap["answer"].c_str(),buff,iDestLen,"utf-8","GBK") ;
    //ErrorLog("答案:[%s]--[%s]" ,inMap["answer"].c_str(),buff);

    //CAuthRelayApi::SetPwdQuestion(inMap,outMap,true);
    return 0;
}
