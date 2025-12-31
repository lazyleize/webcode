#include "CQueryAnswerInfo.h"
#include "CAuthRelayApi.h"

int CQueryAnswerInfo::AuthCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap, outMap;
        /*---add---by---zhangwuyu---20151104---begin---*/
        CStr2Map sessMap;
        /*---add---by---zhangwuyu---20151104---end---*/
	//取得所有的请求参数
	pReqData->GetStrMap(inMap);
	
	CheckLogin(sessMap);
	//用户状态默认为1 有效
	//inMap["state"]="1";
	//调用relay接口
        inMap["name"] = sessMap["uin"];
        inMap["uid"] = sessMap["uid"];
	outMap["question_no"] = CAuthRelayApi::QueryAnswerByUid(inMap["uid"],true);
	pResData->SetPara("question_no",outMap["question_no"]);

	return 0;
}
