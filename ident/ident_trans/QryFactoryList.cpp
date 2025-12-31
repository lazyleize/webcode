#include "CQueryDllList.h"
#include "CIdentRelayApi.h"


int CQryDllList::IdentCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map sessMap;
	CStr2Map inMap, outMap;
	//取得所有的请求参数
	pReqData->GetStrMap(inMap);

	this->CheckLogin(sessMap);
	inMap["usertype"] = sessMap["usertype"];
    int usertype = atoi(sessMap["usertype"].c_str());
    if(usertype != 2 && usertype != 1)
    {
        ErrorLog("没有权限: usertype=[%s]" ,inMap["usertype"].c_str());
        throw CTrsExp(ERR_NO_LOWER_LEVER,"没有权限");
    }

	int limit = atol(inMap["limit"].c_str());
	int offset = atol(inMap["offset"].c_str());

	offset = (offset -1)*limit;
    inMap["limit"] = Tools::IntToStr(limit);
	inMap["offset"] = Tools::IntToStr(offset);

	/*if(!inMap["factory_name"].empty())
	    inMap["factory_name"] = Tools::StrToHex(inMap["factory_name"]);*/

	vector<CStr2Map> vectmapArray;
    CIdentRelayApi::QueryFactoryList(inMap,outMap,vectmapArray,true);
    pResData->SetPara("ret_num",outMap["ret_num"]);
    pResData->SetPara("total",outMap["total"]);

    for(size_t i = 0;i < vectmapArray.size();++i)
    {
        CStr2Map returnMap,resMap;
        CIdentComm::DelMapF(vectmapArray[i],resMap);
		
		returnMap["factory_id"]        = resMap["actory_id"];
		returnMap["state"]	           = resMap["status"];
		returnMap["user"]              = resMap["user"];
		returnMap["create_time"]	   = resMap["create_time"];
		returnMap["modify_time"]	   = resMap["modify_time"];
		returnMap["factory_name"]      = resMap["actory_name"];

        pResData->SetArray(returnMap);
    }

	//增加一个3去查询
	inMap["state"] = "3";
	vectmapArray.clear();
	CIdentRelayApi::QueryFactoryList(inMap,outMap,vectmapArray,true);
    pResData->SetPara("ret_num",outMap["ret_num"]);
    pResData->SetPara("total",outMap["total"]);
	for(size_t i = 0;i < vectmapArray.size();++i)
    {
        CStr2Map returnMap,resMap;
        CIdentComm::DelMapF(vectmapArray[i],resMap);
		
		returnMap["factory_id"]        = resMap["actory_id"];
		returnMap["state"]	           = resMap["status"];
		returnMap["user"]              = resMap["user"];
		returnMap["create_time"]	   = resMap["create_time"];
		returnMap["modify_time"]	   = resMap["modify_time"];
		returnMap["factory_name"]      = resMap["actory_name"];

        pResData->SetArray(returnMap);
    }


	return 0;
}
