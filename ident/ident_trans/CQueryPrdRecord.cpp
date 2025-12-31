#include "CQueryPrdRecord.h"
#include "CIdentRelayApi.h"
#include "xml/unicode.h"

int CQryPrdRecord::IdentCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map sessMap;
	CStr2Map inMap, outMap,tmpMap;
	pReqData->GetStrMap(inMap);

	this->CheckLogin(sessMap);

	CIdentRelayApi::QueryPrdRecord(inMap,outMap,true);

	char utf8_buf[1024] = {0};

	//改
	if(atoi(outMap["Fresult"].c_str()) != 0)
	{
		CIdentRelayApi::QueryErrorSolve(outMap,tmpMap,true);
		DebugLog("solve=[%s]",tmpMap["Fsolve"].c_str());
		if(!tmpMap["Fsolve"].empty())
		{
			//ads_gb2312toutf8((char *)tmpMap["Fsolve"].c_str(),tmpMap["Fsolve"].length(),utf8_buf);
			pResData->SetPara("solve",tmpMap["Fsolve"].c_str());
		}
	}


    CStr2Map returnMap;
    CIdentComm::DelMapF(outMap,returnMap);
  
    // 优先使用pos_sn，如果为空则使用ass_pid，最后使用pack_pid
    if(returnMap["pos_sn"].empty())
    {
        if(!returnMap["ass_pid"].empty())
        {
            returnMap["pos_sn"] = returnMap["ass_pid"];
        }
        else if(!returnMap["pack_pid"].empty())
        {
            returnMap["pos_sn"] = returnMap["pack_pid"];
        }
    }

    pResData->SetPara("pos_sn",returnMap["pos_sn"]);
    pResData->SetPara("cpu_id",returnMap["cpu_id"]);
    pResData->SetPara("falsh_id",returnMap["falsh_id"]);
    pResData->SetPara("wifi_macid",returnMap["wifi_macid"]);
    pResData->SetPara("blue_macid",returnMap["blue_macid"]);
    pResData->SetPara("eth_macid",returnMap["eth_macid"]);
    pResData->SetPara("imei",returnMap["imei"]);
    pResData->SetPara("fatory",returnMap["atory"]);
    pResData->SetPara("pos_name",returnMap["pos_name"]);
    pResData->SetPara("ass_pid",returnMap["ass_pid"]);
    pResData->SetPara("pack_pid",returnMap["pack_pid"]);
    pResData->SetPara("message",returnMap["message"]);
    pResData->SetPara("result",returnMap["result"]);
    return 0;
}
