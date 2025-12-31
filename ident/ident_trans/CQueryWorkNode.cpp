#include "CReportWeekType.h"
#include "CIdentRelayApi.h"

#include <base/strHelper.hpp>
using namespace aps;

int CReportWeekType::IdentCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map sessMap;
	CStr2Map inMap, outMap,tmpOutMap;
	//取得所有的请求参数
	pReqData->GetStrMap(inMap);
	this->CheckLogin(sessMap);

    //先根据ID查询机型
    CIdentRelayApi::QueryWorkNode(inMap,tmpOutMap,true);

    inMap["term_name"] = tmpOutMap["Fterm_name"];
	CIdentRelayApi::QueryTermWorkNode(inMap,outMap,true);

    vector<string> tmpVec;

    //计算个数
    if(!outMap["Fsmt_node"].empty())
    {
        tmpVec.clear();
        StrHelper::trim(outMap["Fsmt_node"]);
        StrHelper::split(outMap["Fsmt_node"],',',tmpVec);
        pResData->SetPara("smt_total",toString(tmpVec.size()));
    }
    else
        pResData->SetPara("smt_total","0");

    if(!outMap["Fpack_node"].empty())
    {
        tmpVec.clear();
        StrHelper::trim(outMap["Fpack_node"]);
        StrHelper::split(outMap["Fpack_node"],',',tmpVec);
        pResData->SetPara("pack_total",toString(tmpVec.size()));
    }
    else
        pResData->SetPara("pack_total","0");


    if(!outMap["Fac_node"].empty())
    {
        tmpVec.clear();
        StrHelper::trim(outMap["Fac_node"]);
        StrHelper::split(outMap["Fac_node"],',',tmpVec);
        pResData->SetPara("fac_node",toString(tmpVec.size()));
    }
    else
        pResData->SetPara("fac_total","0");



    pResData->SetPara("smt_array",outMap["Fsmt_node"]);
    pResData->SetPara("fac_array",outMap["Fac_node"]);
    pResData->SetPara("pack_array",outMap["Fpack_node"]);

	
	return 0;
}
