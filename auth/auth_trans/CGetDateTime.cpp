#include "CGetDateTime.h"
#include "CAuthRelayApi.h"

int CGetDateTime::AuthCommit(CReqData* pReqData,CResData* pResData)
{
    pResData->SetPara("datetime",Tools::GetCurrentDateTime());
    return 0;
}
