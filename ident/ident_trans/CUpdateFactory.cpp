#include "CUpdateFactory.h"
#include "CIdentRelayApi.h"


int CUpdateFactory::IdentCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map sessMap;
	CStr2Map inMap, outMap;
	//取得所有的请求参数
	pReqData->GetStrMap(inMap);

	CheckParameter(inMap);

	this->CheckLogin(sessMap);

	inMap["usertype"] = sessMap["usertype"];
    int usertype = atoi(sessMap["usertype"].c_str());
    if(usertype != 2 && usertype != 1)
    {
        ErrorLog("没有权限: usertype=[%s]" ,inMap["usertype"].c_str());
        throw CTrsExp(ERR_NO_LOWER_LEVER,"没有权限");
    }

	CIdentRelayApi::UpdateFactory(inMap,outMap,true);

	return 0;
}

// 输入判断
void CUpdateFactory::CheckParameter( CStr2Map& inMap)
{
	if(inMap["factory_id"].empty())
    {
        ErrorLog("关键字段不能为空-factory_id");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段factory_id为空");
    }
    if(inMap["state"].empty())
    {
        ErrorLog("关键字段不能为空-state");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段state为空");
    }
}
