#include "CSetAuthEmail.h"
#include "CAuthRelayApi.h"

int CSetAuthEmail::AuthCommit(CReqData* pReqData,CResData* pResData)
{
        /*---add---by---zhangwuyu---20151104---begin---*/
        CStr2Map sessMap;
        /*---add---by---zhangwuyu---20151104---end---*/
	CStr2Map inMap,outMap;

	//取得所有的请求参数
	pReqData->GetStrMap(inMap);

	//检查用户登录态
	CheckLogin(sessMap);
	
	//校验验证码
	//CheckVerifyCode(pReqData->GetPara("verify_code"));

	//检查用户是否是合法用户
	/*paramap["uid"] = CAuthRelayApi::QueryUidByUin(this->GetCookieUin().empty()? paramap["name"] : this->GetCookieUin());
	if(paramap["uid"].empty())
	{
		ErrorLog("该用户非法登录,uin:[%s] 用户登录ip:[%s]"
				,this->GetCookieUin().c_str(),this->GetClientIp().c_str());
		throw CTrsExp(ERR_USER_NOT_REGISTER,"该用户未注册");
	}*/
	//paramap["uin"] = this->m_stSessionData.szUin;
	inMap["name"] = sessMap["uin"];
/*	不需要这个查询
	//查询是否有旧邮箱
	CStr2Map outMap;
	CAuthRelayApi::QueryLoginInfo(paramap["uid"],outMap,true);
	paramap["o_email"] = outMap["email"];
*/
	CAuthRelayApi::SetAuthEmail(inMap,outMap,true);
	return 0;
}
