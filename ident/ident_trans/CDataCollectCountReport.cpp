#include "CDataCollectCountReport.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"
#include "xml/unicode.h"

#include <base/datetime.hpp>
using namespace aps;
int CDataCollectCountReport::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,outMap,tempInMap,tempOutMap,sessionMap;
	string uid;

	// 取得请求参数
    pReqData->GetStrMap(inMap);

	this->CheckLogin(sessionMap);
    


    if(inMap["date"].empty())
        inMap["date"] = Date::now().format("Y-M-D");

    tempInMap["year"]=inMap["date"].substr(0,4);
    tempInMap["month"]=inMap["date"].substr(5,2);
    CIdentRelayApi::QueryMonthSum(tempInMap,tempOutMap,true);
    pResData->SetPara("ship_count",tempOutMap["monthly_total"]);//本月出货数量

    //上月出货数量
    tempOutMap.clear();
    tempInMap["month"]=to_string(Date::now().addMonths(-1).getMonth());
    tempInMap["year"] =to_string(Date::now().addMonths(-1).getYear());
    CIdentRelayApi::QueryMonthSum(tempInMap,tempOutMap,true);
    pResData->SetPara("last_month_count",tempOutMap["monthly_total"]);//上月出货数量


	CIdentRelayApi::QueryShipSum(inMap,outMap,true);

	CStr2Map returnMap;
    CIdentComm::DelMapF(outMap,returnMap);

    //计算出较昨日新增百分比
    /*int todayAdded = stoi(returnMap["taday_count"]);
    int yesterdayAdded = stoi(returnMap["last_count"]);
    double percentageChange = ((todayAdded - yesterdayAdded) / static_cast<double>(yesterdayAdded)) * 100;
    string percentageStr = std::to_string(percentageChange);
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << percentageChange;*/
	
	pResData->SetPara("date",returnMap["date"]);
	
    pResData->SetPara("today_count",returnMap["taday_count"]);
    pResData->SetPara("last_day_counte",returnMap["last_count"]);
    //pResData->SetPara("previous_day_perce",oss.str());

	return 0;
}
