#include "CQueryActiveDataList.h"
#include "CIdentRelayApi.h"

#include <base/datetime.hpp>



int CQueryActiveDataList::IdentCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map sessMap;
	CStr2Map inMap, outMap;
	pReqData->GetStrMap(inMap);

	this->CheckLogin(sessMap);

    CheckParameter(inMap);

	int limit = atol(inMap["limit"].c_str());
	int offset = atol(inMap["offset"].c_str());

	offset = (offset -1)*limit;
    inMap["limit"] = Tools::IntToStr(limit);
	inMap["offset"] = Tools::IntToStr(offset);
	
    vector<CStr2Map> vectmapArray;
    
    CIdentRelayApi::QueryOrderChangeList(inMap,outMap,vectmapArray,true);
    
    
    pResData->SetPara("ret_num",outMap["ret_num"]);
    pResData->SetPara("total",outMap["total"]);


    for(size_t i = 0;i < vectmapArray.size();++i)
    {
        CStr2Map returnMap;
        CIdentComm::DelMapF(vectmapArray[i],returnMap);
        returnMap["factory"]=returnMap["actory"];
        pResData->SetArray(returnMap);
    }

	return 0;
}

// 输入判断
void CQueryActiveDataList::CheckParameter( CStr2Map& inMap)
{
	if(inMap["order_id"].empty()&&inMap["pos_name"].empty())
    {
        //轮询查询，添加上当前日期
        inMap["create_time"] = aps::Date::now().format("Y-M-D");
	}
}