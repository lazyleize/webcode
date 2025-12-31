#include "CQueryIPList.h"
#include "CAuthRelayApi.h"


int CQueryIPList::AuthCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map sessMap;
	CStr2Map inMap, outMap;
	//取得所有的请求参数
	pReqData->GetStrMap(inMap);

	int limit = atol(inMap["limit"].c_str());
	 limit += 1;
	 inMap["limit"] = Tools::IntToStr(limit);


	vector<CStr2Map> vectmapArray;
    CAuthRelayApi::QueryIPList(inMap,outMap,vectmapArray,true);
    pResData->SetPara("ret_num",outMap["ret_num"]);
    pResData->SetPara("total",outMap["total"]);


    for(size_t i = 0;i < vectmapArray.size();++i)
    {
        CStr2Map returnMap;
        CAuthComm::DelMapF(vectmapArray[i],returnMap);
        returnMap["loading_ip"] = returnMap["loading_ip"]; 
        pResData->SetArray(returnMap);
    }

	return 0;
}
