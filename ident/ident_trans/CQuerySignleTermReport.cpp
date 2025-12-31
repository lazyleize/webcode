#include "CQuerySignleTermReport.h"
#include "CIdentRelayApi.h"

#include <base/datetime.hpp>

// 取消宏定义，避免冲突
#undef max
#undef min
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson; 
using namespace aps;

//
int CQuerySignleTermReport::IdentCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map sessMap,sessOutMap,tempInMap;
	CStr2Map inMap, outMap;
	pReqData->GetStrMap(inMap);

	this->CheckLogin(sessMap);
    CheckParameter(inMap);
    if(inMap["date"].empty())
    {
        inMap["date"] = Date::now().format("Y-M-D");
        tempInMap["year"]=inMap["date"].substr(0,4);
        tempInMap["month"]=inMap["date"].substr(5,2);
        tempInMap["lastmonth"]=to_string(Date::now().addMonths(-1).getMonth());
        // 设置时间范围为今年的1月1号到12月31号
        int currentYear = Date::now().getYear();
        Date yearStart(currentYear, 1, 1);
        Date yearEnd(currentYear, 12, 31);
        tempInMap["create_beg"] = yearStart.format("Y-M-D");
        tempInMap["create_end"] = yearEnd.format("Y-M-D");
    }
    else
    {
        tempInMap["year"]=inMap["date"].substr(0,4);
        tempInMap["month"]=inMap["date"].substr(5,2);
        tempInMap["lastmonth"]=to_string(atoi(tempInMap["month"].c_str())-1);

        int backYear = atoi(tempInMap["year"].c_str());
        Date yearStart(backYear, 1, 1);
        Date yearEnd(backYear, 12, 31);
        tempInMap["create_beg"] = yearStart.format("Y-M-D");
        tempInMap["create_end"] = yearEnd.format("Y-M-D");
    }
        
    tempInMap["pos_name"]=inMap["pos_name"];
    CIdentRelayApi::GetTermProducInfo(tempInMap,outMap,true);


    ErrorLog("total:%s",outMap["total"].c_str());

    
    pResData->SetPara("total",outMap["total"]);
    pResData->SetPara("successTotal",outMap["success"]);
    pResData->SetPara("totalFail",outMap["totalFail"]);
    pResData->SetPara("noSeriFail",outMap["noSeriFail"]);
    pResData->SetPara("SeriFail",outMap["SeriFail"]);
    pResData->SetPara("difference",outMap["difference"]);
    
    return 0;
    
}
// 输入判断
void CQuerySignleTermReport::CheckParameter( CStr2Map& inMap)
{
	if(inMap["pos_name"].empty())
    {
        ErrorLog("关键字段不能为空-pos_name");
		CIdentPub::SendAlarm2("关键字段不能为空-pos_name is empty[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段index_name为空");
	}
}

