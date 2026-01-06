#include "CUserLogin.h"
#include "CAuthRelayApi.h"
#include "cgicomm/adsmem_rules.h"
#include "enc/adsenc.h"
#include "tlib_all.h"

int CUserLogin::GetLoginType(const string & uin)
{
    if(strstr(uin.c_str(), "@"))
    {
        //邮箱登录
        return LOGIN_TYPE_EMAIL;
    }
    else if(Tools::IsDigit(uin.c_str()))
    {
        //手机号登录
        return LOGIN_TYPE_MOBILE;
    }
    else
    {
        //会员名登录
        return LOGIN_TYPE_NAME;
    }
}

int CUserLogin::AuthCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map inMap,outMap;
    LoginSessionData sessionData;

	//取得所有的请求参数
	string strPostdata=pReqData->GetPostData();
	CAuthPub::parsePubRespJson(strPostdata,inMap);

    if(inMap["login_type"].empty())
    {
        ErrorLog("缺少登陆类型字段");
        throw CTrsExp(ERR_WRITE_SESSION,"缺少登陆类型字段");
    }

    inMap["loading_ip"] = pReqData->GetEnv("ClientIp");
	//将原3des解密的改为RSA解密
    inMap["pwd"] = CAuthPub::WebrsaToAdsdes(1,inMap["pwd"],0,TIMEOUT_RSA);

    //inMap["uin"] = inMap["name"];
    //检查用户名密码是否正确, 不正确则返回
    CAuthRelayApi::UserLogin(inMap, outMap, true);
    
    if(outMap["errorcode"] != "0000")
    {
        WarnLog("账户登陆异常 uin:[%s]",inMap["uin"].c_str());
        return 0;
    }

    pResData->SetPara("name", outMap["name"]);
	pResData->SetPara("uin", outMap["uid"]);
    pResData->SetPara("last_ip", outMap["last_ip"]);
    pResData->SetPara("last_time", outMap["last_time"]);
    //pResData->SetPara("last_mark", outMap["last_mark"]);
    pResData->SetPara("fail_num", outMap["fail_num"]);
    pResData->SetPara("transpwd_status", outMap["transpwd_status"]);
    pResData->SetPara("truename_status", outMap["truename_status"]);
    pResData->SetPara("card_status", outMap["card_status"]);
    pResData->SetPara("answer_status", outMap["answer_status"]);
    pResData->SetPara("email_status", outMap["email_status"]);
	pResData->SetPara("factory", outMap["factory_name"]);
    pResData->SetPara("user_type", outMap["user_type"]);

    sessionData.iUinType = GetLoginType(inMap["uin"]);
    sessionData.uType = Tools::StrToLong(outMap["user_type"].c_str());
    memcpy(sessionData.szUin,outMap["name"].c_str(),outMap["name"].length()+1);
    memcpy(sessionData.szUid,outMap["uid"].c_str(),outMap["uid"].length()+1);  
    memcpy(sessionData.szLoginIp,pReqData->GetEnv("ClientIp").c_str(),pReqData->GetEnv("ClientIp").length()+1);
    memcpy(sessionData.szMac,outMap["syscode"].c_str(),outMap["syscode"].length()+1);  
	memcpy(sessionData.szHdd,outMap["factory_id"].c_str(),outMap["factory_id"].length()+1);
    memcpy(sessionData.szfactoyName,outMap["factory_name"].c_str(),outMap["factory_name"].length()+1);
	string sOutErrMsg;
    string strSessionKey = "";
    string server_name = pReqData->GetEnv("ServerName") ;
    char domain[125];
    Tools::ServernameToDomain((char *)server_name.c_str(),domain);
    
	DebugLog("domain=%s",server_name.c_str());
    if(0 != WriteContractSession(&sessionData, sOutErrMsg, strSessionKey,domain))
    {
        ErrorLog("系统繁忙写SESSION失败[%s]",sOutErrMsg.c_str());
        throw CTrsExp(ERR_WRITE_SESSION,"系统繁忙");
    }
    
    DebugLog("写Session: 返回消息=%s",strSessionKey.c_str());
    //更新数据库用户上次和本次登录ip和时间
    inMap["loading_ip"] = pReqData->GetEnv("ClientIp");
    if(!CAuthRelayApi::UpdateUserLogin(inMap))
    {
        DeleteLoginSession(strSessionKey,domain);
        ErrorLog("系统繁忙-更新用户登录信息失败:uin=[%s]",inMap["uin"].c_str());
        throw CTrsExp(ERR_UPDATE_USER_LOGIN,"系统繁忙");
    }
	
    set_cookie("xsuin", outMap["name"].c_str(), NULL, "/",domain,0);
    set_cookie("ltype", outMap["user_type"].c_str(), NULL, "/",domain,0);
    string cookie = this->GetCookieUin();
    DebugLog("Cookie=[%s]",cookie.c_str());

	pResData->SetPara("xstype", outMap["user_type"]);
	pResData->SetPara("xskey", strSessionKey);
    
    return 0;
}

unsigned char*  CUserLogin::Str2Hex(const char *str, unsigned char *hex, int sLen)
{
	if(str == NULL || hex == NULL)
    {
        return NULL;
    }
	if(sLen%2 == 1)
	{
        return NULL;
	}

    char buf[3];
	memset(buf, 0, 3);

	for (int i=0; i<sLen; i+=2)
	{
		memcpy(buf, &str[i], 2);
		hex[i/2]=(unsigned char)strtol(buf, NULL, 16);
	}
	return hex;
}