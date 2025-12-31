#include "CQueryWeekDetailList.h"
#include "CIdentRelayApi.h"


int CQueryWeekDetailList::IdentCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map sessMap;
	CStr2Map inMap, outMap;
	pReqData->GetStrMap(inMap);

	this->CheckLogin(sessMap);

    if(inMap["day"].empty())
    {
        ErrorLog("关键字段不能为空-day");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段day为空");
    }
    inMap["limit"] = "100";
	inMap["offset"] = "0";

	vector<CStr2Map> vectmapArray;
    CIdentRelayApi::QueryWeekCheckDetailList(inMap,outMap,vectmapArray,true);
    pResData->SetPara("ret_num",outMap["ret_num"]);
    pResData->SetPara("total",outMap["total"]);

    for(size_t i = 0;i < vectmapArray.size();++i)
    {
        CStr2Map returnMap;
        CIdentComm::DelMapF(vectmapArray[i],returnMap);
        pResData->SetArray(returnMap);
    }
	return 0;
}
