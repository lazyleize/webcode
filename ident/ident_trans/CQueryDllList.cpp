#include "CQueryDllList.h"
#include "CIdentRelayApi.h"


int CQryDllList::IdentCommit(CReqData *pReqData, CResData *pResData)
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
    CIdentRelayApi::QueryDllList(inMap,outMap,vectmapArray,true);
    pResData->SetPara("ret_num",outMap["ret_num"]);
    pResData->SetPara("total",outMap["total"]);

    for(size_t i = 0;i < vectmapArray.size();++i)
    {
        CStr2Map returnMap,resMap;
        CIdentComm::DelMapF(vectmapArray[i],resMap);

		returnMap["dll_id"]        = resMap["record_id"];
		returnMap["file_name"]     = resMap["ile_name"];
		returnMap["version"]	   = resMap["version"];
		returnMap["upload_time"]   = resMap["upload_time"];
		returnMap["state"]		   = resMap["state"];
		returnMap["size"]		   = resMap["size"];
		returnMap["download_num"]  = resMap["download_num"];
		returnMap["remark"]        = resMap["rmark"];

        pResData->SetArray(returnMap);
    }


	return 0;
}
