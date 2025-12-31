#include "CTermCloseGo.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"
#include <cstdlib> // for system
#include <cstdio>  // for popen and pclose

int CTermCloseGo::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,outMap,sessionMap;

	//取得所有的请求参数
	pReqData->GetStrMap(inMap);
	
	CheckParameter(inMap);

    if(!CIdentRelayApi::CloseTermGo(inMap,outMap,true))
    {
        ErrorLog("关闭GO热点失败");
        throw(CTrsExp(outMap["errorcode"],outMap["errormessage"])) ;
    }

	return 0;
}

// 输入判断
void CTermCloseGo::CheckParameter( CStr2Map& inMap)
{
	if(inMap["go_name"].empty())
    {
        ErrorLog("关键字段不能为空-go_name");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段go_name为空");
    }
    if(inMap["term_type"].empty())
    {
        ErrorLog("关键字段不能为空-term_type");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段term_type为空");
    }
}
