#include "CAppLogin.h"
#include "CAuthRelayApi.h"
#include "cgicomm/adsmem_rules.h"
#include "enc/adsenc.h"
#include "tlib_all.h"

int CAppLogin::GetLoginType(const string & uin)
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

int CAppLogin::AuthCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map inMap,tmpMap,outMap;
    LoginSessionData sessionData;

    //取得所有的请求参数
	string strPostdata=pReqData->GetPostData();
	CAuthPub::parsePubRespJson(strPostdata,inMap);

	//RSA解密
    inMap["pwd"] = CAuthPub::WebrsaToAdsdes(1,inMap["pwd"],0,TIMEOUT_RSA);
    
	//查询最新可用版本
	CAuthRelayApi::QueryVerison(inMap,tmpMap,true);

	//判断版本号
	int nRet = compare_version(inMap["version"].c_str(),tmpMap["Fversion"].c_str(),3);
	if(nRet != 0)
	{
		ErrorLog("版本号校验不过:V[%s]，tmpMap V:[%s]",inMap["version"].c_str(),tmpMap["Fversion"].c_str());
        throw CTrsExp(ERR_VERSION_ERROR,"版本号校验不过");
	}

	//校验文件MD5值
	if(inMap["md5"].compare(tmpMap["File_md5"])!=0)
	{
		ErrorLog("文件MD5校验不过:V[%s]，tmpMap V:[%s]",inMap["md5"].c_str(),tmpMap["File_md5"].c_str());
        throw CTrsExp(ERR_MD5FILE_ERROR,"MD5校验不过,请核对最新可以版本号");
	}

    //检查用户名密码是否正确, 不正确则返回
    CAuthRelayApi::AppUserLogin(inMap, outMap, true);
    if(outMap["errorcode"] != "0000")
    {
        WarnLog("APP登陆异常 uid:[%s]",inMap["uin"].c_str());
        return 0;
    }

    pResData->SetPara("status", outMap["state"]);
   
    sessionData.iUinType = GetLoginType(inMap["name"]);
	sessionData.iStatus = atoi(outMap["state"].c_str());
    memcpy(sessionData.szUin,outMap["name"].c_str(),outMap["name"].length()+1);
    memcpy(sessionData.szUid,outMap["uid"].c_str(),outMap["uid"].length()+1);
    memcpy(sessionData.szLoginIp,pReqData->GetEnv("ClientIp").c_str(),pReqData->GetEnv("ClientIp").length()+1);
    string sOutErrMsg;
    string strSessionKey = "";
    string server_name = pReqData->GetEnv("ServerName") ;
    char domain[50];
    Tools::ServernameToDomain((char *)server_name.c_str(),domain);

	DebugLog("domain=%s",domain);
    if(0 != WriteContractSessionApp(&sessionData, sOutErrMsg, strSessionKey,domain))
    {
        ErrorLog("系统繁忙写SESSION失败[%s]",sOutErrMsg.c_str());
        throw CTrsExp(ERR_WRITE_SESSION,"系统繁忙");
    }
    
    DebugLog("写Session: 返回消息=%s",strSessionKey.c_str());
    
	pResData->SetPara("token", strSessionKey.c_str());
	pResData->SetPara("uid", outMap["uid"]);

    return 0;
}

unsigned char*  CAppLogin::Str2Hex(const char *str, unsigned char *hex, int sLen)
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

// 输入判断
void CAppLogin::CheckParameter( CStr2Map& inMap)
{
	if(inMap["name"].empty())
    {
        ErrorLog("关键字段不能为空-name");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段name为空");
    }
    if(inMap["pwd"].empty())
    {
        ErrorLog("关键字段不能为空-pwd");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段pwd为空");
    }
	if(inMap["version"].empty())
    {
        ErrorLog("关键字段不能为空-version");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段version为空");
    }
	if(inMap["md5"].empty())
    {
        ErrorLog("关键字段不能为空-md5");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段md5为空");
    }
}

//版本号格式以.分割
//dest为目标版本
//str为当前版本
//返回结果:-1不需要升级  0-不需要升级   >=1-需要升级
int CAppLogin::compare_version(const char *dest, const char *str, int number)
{
    int dv = 0, sv = 0;
    const char *p_dv = dest, *p_sv = str;
    int i = 0;

    if((dest == NULL) || (str == NULL))
    {
        ErrorLog("error, parameter is null\n");
        return -1;
    }

    ErrorLog("version: dest=%s,str=%s\n", dest, str);

    for(i = 0; i < number; i++)
    {
        dv = atoi(p_dv);
        sv = atoi(p_sv);
        if(dv != sv)
        {
            break;
        }

        p_dv = strstr(p_dv, ".");
        p_sv = strstr(p_sv, ".");

        if((p_dv == NULL) || (p_sv == NULL))
        {
            break;
        }
        p_dv++;
        p_sv++;
    }

    if(dv != sv)
    {
        return (dv - sv);
    }

    //dv == sv
    if((i == number - 1) || (i == number))
    {
        return 0;
    }

    //默认格式以number为准，格式不一致的都认为错误
    return -1; //版本号的格式不固定
}
