#include "CCheckVerifyCode.h"
#include "CAuthRelayApi.h"
#include "tools/base64.h"
#include "tlib/tlib_all.h"

int CCheckVerifyCode::AuthCommit(CReqData* pReqData,CResData* pResData)
{
		CStr2Map inMap;
		
		//取得所有的请求参数
        pReqData->GetStrMap(inMap);

		inMap["uin"] = this->GetCookieUin();
		//获取cookie中的验证码
		string verify_code = m_pReqData->GetCookie(VERIFYSESSION);
		if(verify_code.empty())
		{
			ErrorLog("验证码过期 uin:[%s] verify_code:[%s]",inMap["uin"].c_str(),verify_code.c_str());
            throw CTrsExp(ERR_VERIFYCODE_TIMEOUT,"验证码过期，请从新获取验证码");
		}
		
		//验证码解密
        char verify_decode[32];
        memset(verify_decode, 0x00, sizeof(verify_decode));
        int length;
		//base64解密
		int result = Base64_Decode(verify_code.c_str(), verify_code.length(),
                    (unsigned char*)verify_decode,sizeof(verify_decode),&length);
		if(result != 0)
		{
			ErrorLog("[%s],[%d]行 验证码解密失败 verify_code:[%s]",__FILE__,__LINE__,verify_code.c_str());
            throw CTrsExp(ERR_DECODE_BASE,"系统繁忙,请稍后再试");
		}
		
		//删除cookie记录
		//set_cookie(VERIFYSESSION, "", "Thu, 01 Jan 1970 00:00:00 GMT", "/", "icandai.com",  0);
	
		//校验验证码是否正确
		if(string(verify_decode) != inMap["verify_code"])	
		{
			ErrorLog("[%s],[%d]行 验证码错误 uin:[%s]",__FILE__,__LINE__,inMap["verify_code"].c_str());
            throw CTrsExp(ERR_VERIFY_CODE,"验证码错误");
		}

	
		return 0;
}
