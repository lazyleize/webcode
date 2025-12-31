#include "CQueryAuthRealName.h"
#include "CAuthRelayApi.h"

int CQueryAuthRealName::AuthCommit(CReqData *pReqData, CResData *pResData)
{
        /*---add---by---zhangwuyu---20151104---begin---*/
        CStr2Map sessMap;
        /*---add---by---zhangwuyu---20151104---end---*/
	CStr2Map inMap, outMap;
	//取得所有的请求参数
	pReqData->GetStrMap(inMap);
	
	CheckLogin(sessMap);
	//用户状态默认为1 有效
	//inMap["state"]="1";
	//调用relay接口
        inMap["uid"] = sessMap["uid"];
	CAuthRelayApi::QueryUserBaseInfo(inMap["uid"],outMap,true);
	CStr2Map returnMap;
        CAuthComm::DelMapF(outMap,returnMap);

        Tools::ShieldCard(returnMap["card_no"]);
        pResData->SetPara("card_no",returnMap["card_no"]);
        string mobile = Tools::MobileMask(returnMap["mobile"],1);
        pResData->SetPara("mobile",mobile);
        string email = Tools::EmailMask(returnMap["email"],1);
        pResData->SetPara("email",email);

	return 0;
}
