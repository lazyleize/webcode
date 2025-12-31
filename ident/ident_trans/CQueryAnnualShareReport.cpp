#include "CQueryAnnualShareReport.h"
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

//机型出货占比（年）
int CQueryAnnualShareReport::IdentCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map sessMap,sessOutMap;
	CStr2Map inMap, outMap,qryInMap,qryOutMap;
	pReqData->GetStrMap(inMap);

	//this->CheckLogin(sessMap);
    CheckParameter(inMap);

    inMap["limit"] = inMap["term_num"];
	inMap["offset"] = "0";
    vector<CStr2Map> vectmapArray;

    //去查询全年出货总数
    qryInMap["year"] = inMap["year"];
    CIdentRelayApi::QueryOfYearQuantity(qryInMap,qryOutMap,true);

    long long datellTermSuccess = 0;
    long long datellTermFail = 0;
    long long datellTermCount = 0;
    datellTermSuccess = atoll(qryOutMap["Ftotal_quantity"].c_str());
    datellTermFail    = atoll(qryOutMap["Ftotal_down_fail"].c_str());
    datellTermCount   = atoll(qryOutMap["Ftotal_down_count"].c_str());

    //批量查询
    CIdentRelayApi::QueryYearQuantityList(inMap,outMap,vectmapArray,false);
    pResData->SetPara("ret_num",outMap["ret_num"]);
    pResData->SetPara("total",qryOutMap["Ftotal_quantity"]);
    

    long long llTermSuccess = 0;
    long long llTermFail = 0;
    long long llTermCount = 0;
    CStr2Map returnMap;

    for(size_t i = 0;i < vectmapArray.size();++i)
    {
        returnMap.clear();
        returnMap["term_name"]    = vectmapArray[i]["Fterm_name"];
        returnMap["quantity"]     = vectmapArray[i]["Ftotal_quantity"];
        returnMap["down_count"]   = vectmapArray[i]["Ftotal_down_count"];
        returnMap["down_fail"]    = vectmapArray[i]["Ftotal_down_fail"];
        returnMap["down_success"] = vectmapArray[i]["Ftotal_quantity"];

        //统计数量
        llTermSuccess += atoll(returnMap["down_success"].c_str());
        llTermFail    += atoll(returnMap["down_fail"].c_str());
        llTermCount   += atoll(returnMap["down_count"].c_str());

        pResData->SetArray(returnMap);
    }
    if(inMap["term_num"] != outMap["total"])//传入的数量不等于查到的总数，需要返回其他
    {
        returnMap.clear();
        returnMap["term_name"]    = "其它";
        returnMap["quantity"]     = toString(datellTermSuccess - llTermSuccess);
        returnMap["down_count"]   = toString(datellTermCount - llTermCount);
        returnMap["down_fail"]    = toString(datellTermFail - llTermFail);
        returnMap["down_success"] = toString(datellTermSuccess - llTermSuccess);
        pResData->SetArray(returnMap);
    }

    return 0;

}
// 输入判断
void CQueryAnnualShareReport::CheckParameter( CStr2Map& inMap)
{
	if(inMap["term_num"].empty())
    {
        inMap["term_num"] = "8";
	}
    if(inMap["year"].empty())
    {
        inMap["year"]= to_string(Date::now().getYear());
    }
}

void CQueryAnnualShareReport::sendToElasticsearch(const string& strUrl,string& inStr,string& strOutJson)
{
    string  strResponse;
    if(CIdentPub::HttpESPost(strUrl,inStr,strOutJson))
    {
        InfoLog("http post fail %s", strUrl.c_str()); 
		return;
    }
	return ;
}
