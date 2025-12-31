#include "CQueryRegisterInfo.h"
#include "CAuthRelayApi.h"

int CQueryRegisterInfo::AuthCommit(CReqData* pReqData,CResData* pResData)
{
	CStr2Map outMap;

	CAuthRelayApi::QueryLoginInfo(pReqData->GetPara("uin"),outMap,true);

	pResData->SetPara("ret_num",outMap["ret_num"]);
	pResData->SetPara("name",this->GetCookieUin());
	pResData->SetPara("last_ip",outMap["last_ip"]);
	pResData->SetPara("last_time",outMap["last_time"]);
	pResData->SetPara("mobile",outMap["mobile"]);
	pResData->SetPara("email",outMap["email"]);
	pResData->SetPara("is_realname",outMap["is_realname"].empty()?"0":outMap["is_realname"]);
	pResData->SetPara("strength",outMap["strength"]);
	pResData->SetPara("set_question",outMap["set_question"]);
	pResData->SetPara("question",outMap["question"]);
	pResData->SetPara("answer",outMap["answer"]);
	
	return 0;
}
