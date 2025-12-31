#include "CCheckUserAnswer.h"
#include "CAuthRelayApi.h"

int CCheckUserAnswer::AuthCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap;
	//取得所有的请求参数
	pReqData->GetStrMap(inMap);
	CAuthRelayApi::CheckUserAnswer(inMap,true);
	return 0;
}
