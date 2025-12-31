#include "CSetAuthMobile.h"
#include "CAuthRelayApi.h"

int CSetAuthMobile::AuthCommit(CReqData* pReqData,CResData* pResData)
{
        /*---add---by---zhangwuyu---20151104---begin---*/
        CStr2Map sessMap;
        /*---add---by---zhangwuyu---20151104---end---*/

	CStr2Map inMap,outMap;
	pReqData->GetStrMap(inMap);

	//检查用户登录态
    CheckLogin(sessMap);
	//检查用户是否是合法用户
	/*inMap["uid"] = CAuthRelayApi::QueryUidByUin(this->GetCookieUin());
	if(inMap["uid"].empty())
	{
		ErrorLog("该用户非法登录,uin:[%s] 用户登录ip:[%s]"
				,this->GetCookieUin().c_str(),this->GetClientIp().c_str());
		throw CTrsExp(ERR_USER_NOT_REGISTER,"该用户未注册");
	}*/
	inMap["name"] = sessMap["uin"];
	//调用解绑手机号码接口
	CAuthRelayApi::SetAuthMobile(inMap,outMap,true);
	
	if("1" == outMap["op_stat"])
	{
			string mobile = Tools::MobileMask(outMap["mobile"],1);
			pResData->SetPara("mobile",mobile);
	}
	pResData->SetPara("op_stat",outMap["op_stat"]);
	pResData->SetPara("op_type",outMap["op_type"]);
	return 0;
}
