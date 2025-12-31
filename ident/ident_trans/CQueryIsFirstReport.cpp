#include "CQueryIsFirstReport.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"
#include "xml/unicode.h"

#include <base/datetime.hpp>
using namespace aps;
int CQueryIsFirstReport::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,outMap,tempOutMap,sessionMap;
	string uid;

	// 取得请求参数
    pReqData->GetStrMap(inMap);

	this->CheckLogin(sessionMap);

    //查询用户信息，查看是否首次进入这个页面
    CIdentRelayApi::UpdateUserIsFirst(sessionMap,tempOutMap,true);
    
    pResData->SetPara("is_first",tempOutMap["isFirst"]);

	return 0;
}
