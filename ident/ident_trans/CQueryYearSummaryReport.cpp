#include "CQueryYearSummaryReport.h"
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
int CQueryYearSummaryReport::IdentCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map sessMap,sessOutMap,tempInMap;
	CStr2Map inMap, outMap,lastYearMap;
	pReqData->GetStrMap(inMap);

	this->CheckLogin(sessMap);
    CheckParameter(inMap);
    
    // 查询年度数据
    CIdentRelayApi::QueryYearSummary(inMap,outMap,true);

    //查上一年的数据
    tempInMap["year"] = inMap["prev_year"];
    CIdentRelayApi::QueryYearSummary(tempInMap,lastYearMap,true);


    CStr2Map qualiMap;//生产质量数据
    qualiMap["passRate"]          = outMap["Fonepass"];
    qualiMap["passRateTrend"]     = lastYearMap["Fonepass"];
    qualiMap["reworkRate"]        = outMap["Frework"];
    qualiMap["reworkRateTrend"]   = lastYearMap["Frework"];
    qualiMap["avgEfficiency"]     = outMap["Favgefficiency"];
    qualiMap["efficiencyTrend"]   = lastYearMap["Favgefficiency"];
    //最晚加班时间
    qualiMap["latestOvertime"]    = outMap["FlastOvertime"];
    qualiMap["inspectionCount"]   = outMap["Finspection"];
    qualiMap["inspectionTrend"] = to_string( ((atoll(outMap["FlastOvertime"].c_str()) - atoll(lastYearMap["FlastOvertime"].c_str()))/ atoll(lastYearMap["FlastOvertime"].c_str()))*100);

    pResData->SetArray("qualityData",qualiMap);


    
    CStr2Map errorMap;//错误类型

    errorMap.clear();
    errorMap["error_type"]     = "串口错误";
    errorMap["count"]          = outMap["Ferror_comm"];
    pResData->SetArray("errorTypes",errorMap);

    errorMap.clear();
    errorMap["error_type"]     = "流程节点错误";
    errorMap["count"]          = outMap["Ferror_work"];
    pResData->SetArray("errorTypes",errorMap);

    errorMap.clear();
    errorMap["error_type"]     = "调用接口错误";
    errorMap["count"]          = outMap["Ferror_api"];
    pResData->SetArray("errorTypes",errorMap);

    errorMap.clear();
    errorMap["error_type"]     = "数据错误";
    errorMap["count"]          = outMap["Ferror_data"];
    pResData->SetArray("errorTypes",errorMap);

    errorMap.clear();
    errorMap["error_type"]     = "客户定制错误";
    errorMap["count"]          = outMap["Ferror_custom"];
    pResData->SetArray("errorTypes",errorMap);

    errorMap.clear();
    errorMap["error_type"]     = "下载流程错误";
    errorMap["count"]          = outMap["Ferror_down"];
    pResData->SetArray("errorTypes",errorMap);


    // ========== 季度数据 ==========
    CStr2Map quarterMap;

    quarterMap.clear();
    quarterMap["quarter"]    = "1";
    quarterMap["name"]       = "第一季度";
    quarterMap["value"]      = outMap["FfirstQuarter"];
    quarterMap["growth"]     = to_string(((atof(outMap["FfirstQuarter"].c_str()) - atof(lastYearMap["FfirstQuarter"].c_str()))/atof(lastYearMap["FfirstQuarter"].c_str()))*100);
    quarterMap["passRate"]   = outMap["FfirstpassRate"];
    quarterMap["efficiency"] = outMap["Ffirstefficiency"];
    pResData->SetArray("quarters",quarterMap);


    quarterMap.clear();
    quarterMap["quarter"]    = "2";
    quarterMap["name"]       = "第二季度";
    quarterMap["value"]      = outMap["FsecondQuarter"];
    quarterMap["growth"]     = to_string(((atof(outMap["FsecondQuarter"].c_str()) - atof(lastYearMap["FsecondQuarter"].c_str()))/atof(lastYearMap["FsecondQuarter"].c_str()))*100);
    quarterMap["passRate"]   = outMap["FsecondpassRate"];
    quarterMap["efficiency"] = outMap["Fsecondefficiency"];
    pResData->SetArray("quarters",quarterMap);


    quarterMap.clear();
    quarterMap["quarter"]    = "3";
    quarterMap["name"]       = "第三季度";
    quarterMap["value"]      = outMap["FthirdQuarter"];
    quarterMap["growth"]     = to_string(((atof(outMap["FthirdQuarter"].c_str()) - atof(lastYearMap["FthirdQuarter"].c_str()))/atof(lastYearMap["FthirdQuarter"].c_str()))*100);
    quarterMap["passRate"]   = outMap["FthirdpassRate"];
    quarterMap["efficiency"] = outMap["Fthirdefficiency"];
    pResData->SetArray("quarters",quarterMap);


    quarterMap.clear();
    quarterMap["quarter"]    = "4";
    quarterMap["name"]       = "第四季度";
    quarterMap["value"]      = outMap["FfourthQuarter"];
    quarterMap["growth"]     = to_string(((atof(outMap["FfourthQuarter"].c_str()) - atof(lastYearMap["FfourthQuarter"].c_str()))/atof(lastYearMap["FfourthQuarter"].c_str()))*100);
    quarterMap["passRate"]   = outMap["FfourthpassRate"];
    quarterMap["efficiency"] = outMap["Ffourthefficiency"];
    pResData->SetArray("quarters",quarterMap);


    // ========== 月度统计数据 ==========
    
    return 0;
    
}
// 输入判断
void CQueryYearSummaryReport::CheckParameter( CStr2Map& inMap)
{
	if(inMap["year"].empty()||inMap["prev_year"].empty())
    {
        ErrorLog("关键字段不能为空-year");
		CIdentPub::SendAlarm2("关键字段不能为空-year is empty[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段year为空");
	}
}

