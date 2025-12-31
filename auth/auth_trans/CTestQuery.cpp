#include "CTestQuery.h"
#include "CAuthRelayApi.h"

int CTestQuery::AuthCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map inMap;
    CStr2Map outMap;
    //取得所有的请求参数
    pReqData->GetStrMap(inMap);
    CAuthRelayApi::TestQuery(inMap,outMap,true);

    ErrorLog("kkkk  name:[%s]",outMap["name"].c_str());
    pResData->SetPara("name",outMap["name"]);
    pResData->SetPara("address",outMap["address"]);

	/*

    if(!outMap["uin"].empty())
    {
            ErrorLog("该用户名已被注册 uin:[%s]",inMap["uin"].c_str());
            throw CTrsExp(ERR_USER_HAS_REGISTER,"该用户名已被注册");
    }
  
    if(strstr(inMap["uin"].c_str(), "@"))
    {
            string uid = CAuthRelayApi::QueryUidByUin(inMap["uin"]);
            if(!uid.empty())
            {
                    ErrorLog("该邮箱已被注册 uin:[%s]",inMap["uin"].c_str());
                    throw CTrsExp(ERR_USER_HAS_REGISTER,"该邮箱已被注册");
            }
    }
    else if(Tools::IsDigit(inMap["uin"].c_str()))
    {
            string uid = CAuthRelayApi::QueryUidByUin(inMap["uin"]);
            if(!uid.empty())
            {
                    ErrorLog("该手机号已被注册 uin:[%s]",inMap["uin"].c_str());
                    throw CTrsExp(ERR_USER_HAS_REGISTER,"该手机号已被注册");
            }
    }
    else
    {
            string uid = CAuthRelayApi::QueryUidByUin(inMap["uin"]);
            if(!uid.empty())
            {
                    ErrorLog("该用户名已被注册 uin:[%s]",inMap["uin"].c_str());
                    throw CTrsExp(ERR_USER_HAS_REGISTER,"该用户名已被注册");
            }
    }
    */
    return 0;
}
