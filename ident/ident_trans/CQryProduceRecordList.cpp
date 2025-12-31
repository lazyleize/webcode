#include "CQryProduceRecordList.h"
#include "CIdentRelayApi.h"


int CQryProduceRecordList::IdentCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map sessMap;
	CStr2Map inMap, outMap;
	pReqData->GetStrMap(inMap);

	this->CheckLogin(sessMap);

	int limit = atol(inMap["limit"].c_str());
	int offset = atol(inMap["offset"].c_str());

	offset = (offset -1)*limit;
    inMap["limit"] = Tools::IntToStr(limit);
	inMap["offset"] = Tools::IntToStr(offset);

	inMap["usertype"] = sessMap["usertype"];
    int usertype = atoi(sessMap["usertype"].c_str());
    if(usertype == 2 || usertype == 1)
        inMap["factory_id"].clear();
	else if(usertype == 3 )
	    inMap["factory_id"] = sessMap["factoryid"];
    
    if(inMap["factory_id"] == "85000001")
        inMap["factory_id"].clear();
	
    vector<CStr2Map> vectmapArray;
    bool IsPID = false;
    //区分是SN号还是PID
    if(!inMap["pos_sn"].empty())
    {
        IsPID = CIdentPub::IsPIDNumber(inMap["pos_sn"]);
        if(IsPID)
            CIdentRelayApi::QueryProducePIDRecordList(inMap,outMap,vectmapArray,true);
        else
        {
            CIdentRelayApi::QueryProduceRecordList(inMap,outMap,vectmapArray,true); 
        }
            
    }
    else
    {
        CStr2Map tmpMap;
        //批量查询没有条件的时候，从数据库获取total,有条件的时候，直接获取。
        if(inMap["pos_sn"].empty()&&inMap["result"].empty()&&inMap["create_time_beg"].empty()&&inMap["create_time_end"].empty()&&inMap["factory_id"].empty()&&inMap["pos_name"].empty())
        {
            //不带total查询
            CIdentRelayApi::QueryProduceRecordListNoToal(inMap,outMap,vectmapArray,true);

            //单独查询总记录数
            CIdentRelayApi::QueryProduceRecordTotal(inMap,tmpMap,true);
            outMap["total"] = tmpMap["Ftotal"];
        }
        else
            CIdentRelayApi::QueryProduceRecordList(inMap,outMap,vectmapArray,true);
    }

    pResData->SetPara("ret_num",outMap["ret_num"]);
    pResData->SetPara("total",outMap["total"]);
    
    for(size_t i = 0;i < vectmapArray.size();++i)
    {
        CStr2Map returnMap;
        CIdentComm::DelMapF(vectmapArray[i],returnMap);
        /*DebugLog("pos_sn=[%s]",returnMap["pos_sn"].c_str());
        DebugLog("ass_pid=[%s]",returnMap["ass_pid"].c_str());*/
        if(returnMap["pos_sn"].empty())
        {   
            returnMap["pos_sn"] = returnMap["ass_pid"];
        }
		returnMap["factory"] = returnMap["actory"];
        pResData->SetArray(returnMap);
    }

	return 0;
}
