#include "CQryAtmLogList.h"
#include "CIdentRelayApi.h"


int CQryAtmLogList::IdentCommit(CReqData *pReqData, CResData *pResData)
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
	
    vector<CStr2Map> vectmapArray;
    
    CIdentRelayApi::QueryAtmLogList(inMap,outMap,vectmapArray,true);
    
    
    pResData->SetPara("ret_num",outMap["ret_num"]);
    pResData->SetPara("total",outMap["total"]);


    for(size_t i = 0;i < vectmapArray.size();++i)
    {
        CStr2Map returnMap;
        CIdentComm::DelMapF(vectmapArray[i],returnMap);
        returnMap["factory"]=returnMap["actory"];
        returnMap["filepath"]=returnMap["ile_path"];
        pResData->SetArray(returnMap);
    }

	return 0;
}
