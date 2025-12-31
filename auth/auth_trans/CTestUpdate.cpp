#include "CTestUpdate.h"
#include "CAuthRelayApi.h"

int CTestUpdate::AuthCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map inMap;
    CStr2Map outMap;
    //取得所有的请求参数
    pReqData->GetStrMap(inMap);
    CAuthRelayApi::TestUpdate(inMap,outMap,true);
    
    return 0;
}
