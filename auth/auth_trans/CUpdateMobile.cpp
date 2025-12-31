#include "CUpdateMobile.h"
#include "CAuthRelayApi.h"

int CUpdateMobile::AuthCommit(CReqData* pReqData,CResData* pResData)
{
		CStr2Map inMap,outMap;
		inMap["mobile"] = pReqData->GetPara("contact_mobile");
		//检查用户是否是合法用户
		/*inMap["uid"] = CAuthRelayApi::QueryUidByUin(this->GetCookieUin());
		if(inMap["uid"].empty())
		{
				ErrorLog("该用户非法登录,uin:[%s] 用户登录ip:[%s]"
								,this->GetCookieUin().c_str(),this->GetClientIp().c_str());
				throw CTrsExp(ERR_USER_NOT_REGISTER,"该用户未注册");
		}*/
		inMap["uin"] = this->GetCookieUin();

		//调用修改手机号码接口
		CAuthRelayApi::UpdateMobile(inMap,outMap,true);
		return 0;
}
