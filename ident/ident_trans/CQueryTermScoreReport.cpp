#include "CQueryTermScoreReport.h"
#include "CIdentRelayApi.h"

#include <iostream>
#include <cstdio>
#include <memory>

#include <base/datetime.hpp>

using namespace aps;

int CQueryTermScoreReport::IdentCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map sessMap;
	CStr2Map inMap, outMap;
	pReqData->GetStrMap(inMap);

	this->CheckLogin(sessMap);

	int limit = atol(inMap["limit"].c_str());
	int offset = atol(inMap["offset"].c_str());

	offset = (offset -1)*limit;
    inMap["limit"] = Tools::IntToStr(limit);
	inMap["offset"] = Tools::IntToStr(offset);

    CheckParameter(inMap);

	vector<CStr2Map> vectmapArray;
    CIdentRelayApi::QueryTermScoreList(inMap,outMap,vectmapArray,false);
   
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

// 输入判断
void CQueryTermScoreReport::CheckParameter( CStr2Map& inMap)
{
	if(inMap["date"].empty())
    {
        inMap["date"] = Date::now().format("Y-M");
	}
}