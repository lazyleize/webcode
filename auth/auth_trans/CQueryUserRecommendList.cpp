#include "CQueryUserRecommendList.h"
#include "CAuthRelayApi.h"

int CQueryUserRecommendList::AuthCommit(CReqData *pReqData, CResData *pResData)
{
        /*---add---by---zhangwuyu---20151104---begin---*/
        CStr2Map sessMap;
        /*---add---by---zhangwuyu---20151104---end---*/
	CStr2Map inMap, outMap;
	//取得所有的请求参数
	pReqData->GetStrMap(inMap);
	
	CheckLogin(sessMap);
        int limit = atol(inMap["limit"].c_str());
        limit += 1;
        inMap["limit"] = Tools::IntToStr(limit);

	vector<CStr2Map> vectmapArray;
        CAuthRelayApi::QueryUserRecommendList(inMap,outMap,vectmapArray,true);
        pResData->SetPara("ret_num",outMap["ret_num"]);
        pResData->SetPara("total",outMap["total"]);
        int offset = atoi(outMap["ret_num"].c_str()) - 1 + atoi(inMap["offset"].c_str());
	outMap["offset"] = Tools::IntToStr(offset);
        pResData->SetPara("offset",outMap["offset"]);
        for(size_t i = 0;i < vectmapArray.size();++i)
        {
                CStr2Map returnMap;
                CAuthComm::DelMapF(vectmapArray[i],returnMap);
                returnMap["coupon_money"] = Tools::ConvertFenToYan(returnMap["coupon_money"]);
                returnMap["create_time"] = returnMap["create_time"].substr(0,10); 
                pResData->SetArray(returnMap);
        }

	return 0;
}
