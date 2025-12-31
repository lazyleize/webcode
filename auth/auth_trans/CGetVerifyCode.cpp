#include "CGetVerifyCode.h"
#include "CAuthRelayApi.h"
#include "tlib/tlib_all.h"
#include "tools/base64.h"
#include "tools/sendmail.h"

int CGetVerifyCode::AuthCommit(CReqData* pReqData,CResData* pResData)
{
		CStr2Map inMap;
		
		//取得所有的请求参数
    	pReqData->GetStrMap(inMap);
		
		//生成激活码
		//int code = this->GetVerifyCode();
		inMap["verify_code"] = this->GetVerifyCode();

		//验证码加密
		char verify_encode[256];
		memset(verify_encode, 0x00, sizeof(verify_encode));
		int len;
		//base64加密
		int result = Base64_Encode((unsigned char*)inMap["verify_code"].c_str(),inMap["verify_code"].length(),
				(char*)verify_encode,sizeof(verify_encode), &len);
		if(result != 0)
		{
			ErrorLog("[%s],[%d]行 验证码加密失败 verify_code:[%s]",__FILE__,__LINE__,inMap["verify_code"].c_str());
            throw CTrsExp(ERR_ENCODE_BASE,"系统繁忙,请稍后再试");
		}
		//验证码存储到session服务器中
		//验证码存储到cookie中
		set_cookie(VERIFYSESSION, verify_encode, NULL, "/", "transt.cn",  0);

		//查询用户uid
		/*string uid = CAuthRelayApi::QueryUidByUin(pReqData->GetCookie(SX_LOGIN_SESSION_UIN_NAME));
    	if(uid.empty())
    	{
        	//ErrorLog("[%s],[%d]行 该用户不存在 uin:[%s]",__FILE__,__LINE__,pReqData->GetCookie(SX_LOGIN_SESSION_UIN_NAME).c_str());
        	//throw CTrsExp(ERR_UNKNOWN_USER,"不存在的用户");
    	}	

		//给用户发送验证码
		if(CHECK_TYPE_MOBILE == pReqData->GetPara("c_type"))
		{	
			if(inMap["mobile"].empty())
			{
				//查询用户手机号
				inMap["mobile"] = CAuthRelayApi::QueryMobileByUid(inMap["uid"],true);	
			}
			//发送手机短信
		}*/
		InfoLog("1111111111111");
		if(CHECK_TYPE_EMAIL == pReqData->GetPara("c_type"))
		{
		InfoLog("222222");
			if(inMap["email"].empty())
        	{
				ErrorLog("[%s],[%d]行 用户邮箱为空",__FILE__,__LINE__);
        	}
			//发送电子邮件
			string from = "13632850648@163.com";    //暂时使用该邮箱，后续需要从配置文件中读取
			string password = "272599dtq";         //同上
			string subject = "邮箱验证码";
			string message = "验证码: " + inMap["verify_code"];
			string errMsg;
		InfoLog("3333333");
			if(sendEmail(inMap["email"], from,password,subject,message,"smtp.163.com",errMsg) != 0)
			{
				ErrorLog("[%s],[%d]行 邮件发送失败,错误信息[%s]",__FILE__,__LINE__,errMsg.c_str());
			}
		InfoLog("44444444");
		}
		
		pResData->SetPara("verify_code", inMap["verify_code"]);
	
		return 0;
}
