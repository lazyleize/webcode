#include "CWeeklyDataCollectCount.h"
#include "CIdentRelayApi.h"

#define WEEKS 7
int CWeeklyDataCollectCount::IdentCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map sessMap;
	CStr2Map inMap, outMap;
	string begDate,endDate;
	string DataNow;
	//取得所有的请求参数
	pReqData->GetStrMap(inMap);
	this->CheckLogin(sessMap);

	/*inMap["usertype"] = sessMap["usertype"];
    int usertype = atoi(sessMap["usertype"].c_str());
    if(usertype == 2 || usertype == 1)
        inMap["factory_id"].clear();
	else
	    inMap["factory_id"] = sessMap["factoryid"];*/

	inMap["limit"] = "7";
	inMap["offset"] = "0";

	if(!inMap["create_time_end"].empty())
	{
		endDate = inMap["create_time_end"];
		begDate = CIdentPub::GetXyyDay(endDate.c_str(),-107);

		DebugLog("begDate=[%s] endDate=[%s]",begDate.c_str(),endDate.c_str());
		
	}
	else
	{
		//取当前时间
		DataNow = CIdentPub::GetDateNow();
		inMap["create_time_end"] = DataNow;
		endDate = inMap["create_time_end"];
		begDate = CIdentPub::GetXyyDay(endDate.c_str(),-107);

	}
	inMap["date"] = begDate;

	vector<CStr2Map> vectmapArray;
	CIdentRelayApi::QueryCollectSevenList(inMap,outMap,vectmapArray,true);
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
