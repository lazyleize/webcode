#include "CQueryAppUserList.h"
#include "CAuthRelayApi.h"


int CQueryAppUserList::AuthCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map sessMap;
	CStr2Map inMap, outMap;
	//取得所有的请求参数
	pReqData->GetStrMap(inMap);

	//获取用户工厂信息
	this->CheckLogin(sessMap);
	int limit = atol(inMap["limit"].c_str());
	int offset = atol(inMap["offset"].c_str());
	

    inMap["usertype"] = sessMap["usertype"];
    int usertype = atoi(sessMap["usertype"].c_str());
    if(usertype == 2 || usertype == 1)
        inMap["factory_id"].clear();
	else
	    inMap["factory_id"] = sessMap["factoryid"];

	

	offset = (offset -1)*limit;
    inMap["limit"] = Tools::IntToStr(limit);
	inMap["offset"] = Tools::IntToStr(offset);

	vector<CStr2Map> vectmapArray;
    CAuthRelayApi::QueryAppUserList(inMap,outMap,vectmapArray,true);
    pResData->SetPara("ret_num",outMap["ret_num"]);
    pResData->SetPara("total",outMap["total"]);

    for(size_t i = 0;i < vectmapArray.size();++i)
    {
        CStr2Map returnMap,resMap;
        CAuthComm::DelMapF(vectmapArray[i],resMap);

		returnMap["app_uid"]               = resMap["uid"];
		returnMap["name"]                  = resMap["name"];
		returnMap["factory"]	           = resMap["actory"];
		returnMap["state"]                 = resMap["state"];
		returnMap["create_time"]		   = resMap["create_time"];
        pResData->SetArray(returnMap);
    }
	return 0;
}
