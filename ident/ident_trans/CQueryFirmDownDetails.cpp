#include "CQueryFirmDownDetails.h"
#include "CIdentRelayApi.h"
#include "xml/unicode.h"

#include <base/strHelper.hpp>

using namespace aps;

int CQueryFirmDownDetails::IdentCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map sessMap;
	CStr2Map inMap, outMap,tmpMap,tmpOutMap;
	pReqData->GetStrMap(inMap);

	this->CheckLogin(sessMap);

	CIdentRelayApi::QueryFirmDownDetail(inMap,outMap,true);

    CStr2Map returnMap;
    CIdentComm::DelMapF(outMap,returnMap);

    tmpMap["pack_id"] = returnMap["pack_id"];
    //查版本
    CIdentRelayApi::QueryFirmDownRule(tmpMap,tmpOutMap,true);
    
    vector<string> verVec = StrHelper::split(tmpOutMap["Fversion"],"|");

    pResData->SetPara("trans_id",returnMap["task_id"]);
    pResData->SetPara("term_type",returnMap["term_type"]);
    pResData->SetPara("order_id",returnMap["order_id"]);
    pResData->SetPara("pid",returnMap["pid"]);
    pResData->SetPara("down_type",returnMap["down_type"]);
    pResData->SetPara("mater_num",returnMap["mater_num"]);
    pResData->SetPara("result",returnMap["state_node"]);
    pResData->SetPara("begin_time",returnMap["begin_time"]);


    const std::string versionKeys[] = { "app_version", "boot_version", "core_version", "recover_version" };
    for (size_t i = 0; i < min(verVec.size(), sizeof(versionKeys)/sizeof(versionKeys[0])); ++i)
    {
        pResData->SetPara(versionKeys[i], verVec[i]);
    }
    return 0;
}
