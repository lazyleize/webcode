#include "CQueryDataExportCount.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"
#include "xml/unicode.h"

#include <base/datetime.hpp>
using namespace aps;
int CQueryDataExportCount::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,outMap,quryInMap, quryOutMap,sessionMap;
	string uid;

	// 取得请求参数
    vector<CStr2Map> vectmapArray;
    pReqData->GetStrMap(inMap);
    string strdata=pReqData->GetPostData();
	InfoLog("strdata [%s]", strdata.c_str());
    CIdentPub::parsePubRespJsonList(strdata,inMap,vectmapArray);

	//this->CheckLogin(sessionMap);

    //查询用户信息，查看是否首次进入这个页面
    //初始化查询参数
	quryInMap["limit"] = "300";
	quryInMap["offset"] = "0";
	quryInMap["pid_beg"] = inMap["pid_beg"];
	quryInMap["pid_end"] = inMap["pid_end"];

    if(inMap["date_type"] != "ALL")
			quryInMap["type"] = inMap["date_type"];

    int nCount = vectmapArray.size();
    int nTotal=0 ;
    ErrorLog("nCount=%d",nCount);
    for(int i = 0;i < nCount;i++)
    {
        quryInMap["pid_beg"] = vectmapArray[i]["pid_beg"]; 
        quryInMap["pid_end"] = vectmapArray[i]["pid_end"];
        CIdentRelayApi::QueryPidRangCollectList(quryInMap, quryOutMap, vectmapArray, true);
        nTotal += atoi(quryOutMap["total"].c_str());
    }

    pResData->SetPara("total",to_string(nTotal));

	return 0;
}
