#include "CQueryAuthSafeStatus.h"
#include "CAuthRelayApi.h"

int CQueryAuthSafeStatus::AuthCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,outMap,sessMap;
	//取得所有的请求参数
	pReqData->GetStrMap(inMap);
	
        /*--JiaYeHui --- 20151104 --- 从cess获取UID及UIN --*/
        CheckLogin(sessMap);
	inMap["uid"] = sessMap["uid"];
	inMap["uin"] = sessMap["uin"];

	//调用relay接口
	if(!CAuthRelayApi::QueryAuthSafeStatus(inMap["uid"],outMap,true))
	{
		 ErrorLog("查询用户安全状态失败 uin:[%s]",inMap["uin"].c_str());
         throw CTrsExp(ERR_SYS_ERROR,"系统繁忙，请稍后再试!");
	}
	CStr2Map returnMap;
	CAuthComm::DelMapF(outMap,returnMap);

	pResData->SetPara("transpwd_status",returnMap["transpwd_status"]);
        pResData->SetPara("truename_status",returnMap["truename_status"]);
        pResData->SetPara("card_status",returnMap["card_status"]);
        pResData->SetPara("answer_status",returnMap["answer_status"]);
        pResData->SetPara("email_status",returnMap["email_status"]);
        pResData->SetPara("mobile_status",returnMap["mobile_status"]);
	Tools::ShieldCard(returnMap["card_no"]);
	pResData->SetPara("card_no",returnMap["card_no"]);
	string mobile = Tools::MobileMask(returnMap["mobile"],1);
	pResData->SetPara("mobile",mobile);

	return 0;
}
