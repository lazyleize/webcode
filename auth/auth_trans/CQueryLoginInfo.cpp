#include "CQueryLoginInfo.h"
#include "CAuthRelayApi.h"

int CQueryLoginInfo::AuthCommit(CReqData* pReqData,CResData* pResData)
{
        /*---add---by---zhangwuyu---20151104---begin---*/
        CStr2Map sessMap;
        /*---add---by---zhangwuyu---20151104---end---*/
	CStr2Map inMap,outMap;

	//检查用户登录态
	CheckLogin(sessMap);
	//检查用户是否是合法用户
	string uin  = sessMap["uin"];
	//调用查询用户登陆信息接口
	CAuthRelayApi::QueryLoginInfo(uin,outMap,true);
	if("0" != outMap["ret_num"])
	{
		//ErrorLog("该用户未实名认证,uin:[%s]",this->GetCookieUin().c_str());
		//throw CTrsExp(ERR_NOT_REAL_USER,"该用户未实名认证");
		pResData->SetPara("last_time",outMap["last_time"]);
		pResData->SetPara("last_ip",outMap["last_ip"]);
		string mobile = Tools::MobileMask(outMap["mobile"],1);
		pResData->SetPara("mobile",mobile);
		string email = Tools::EmailMask(outMap["email"]);
		pResData->SetPara("email",email);
		Tools::ShieldCard(outMap["id_no"]);
		pResData->SetPara("id_no",outMap["id_no"].c_str());
		Tools::ShieldCard(outMap["card_no"]);
		pResData->SetPara("card_no",outMap["card_no"]);
		pResData->SetPara("transpwd_status",outMap["transpwd_status"]);
		pResData->SetPara("truename_status",outMap["truename_status"]);
		pResData->SetPara("card_status",outMap["card_status"]);
		pResData->SetPara("answer_status",outMap["answer_status"]);
		pResData->SetPara("cmer_status",outMap["cmer_status"]);
		pResData->SetPara("email_status",outMap["email_status"]);
		pResData->SetPara("mobile_status",outMap["mobile_status"]);
	//	pResData->SetPara("real_name",outMap["real_name"]);
	//	pResData->SetPara("name",outMap["name"]);
	}
	pResData->SetPara("ret_num",outMap["ret_num"]);
	return 0;
}
