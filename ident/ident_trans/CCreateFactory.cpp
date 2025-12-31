#include "CCreateFactory.h"
#include "CIdentRelayApi.h"
#include "xml/unicode.h"

int CCreateFactory::IdentCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map sessMap;
	CStr2Map inMap, outMap;
	//取得所有的请求参数
	string strPostdata=pReqData->GetPostData();
	CIdentPub::parsePubRespJson(strPostdata,inMap);

	this->CheckLogin(sessMap);

    inMap["usertype"] = sessMap["usertype"];
    int usertype = atoi(sessMap["usertype"].c_str());
    if(usertype != 2  && usertype != 1)
    {
        ErrorLog("没有权限: usertype=[%s]" ,inMap["usertype"].c_str());
        throw CTrsExp(ERR_NO_LOWER_LEVER,"没有权限");
    }

	inMap["user"] = sessMap["uin"];
	CIdentRelayApi::CreateFactory(inMap,outMap,true);

	return 0;
}

// 输入判断
void CCreateFactory::CheckParameter( CStr2Map& inMap)
{
	if(inMap["factory_name"].empty())
    {
        ErrorLog("关键字段不能为空-factory_name");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段工厂名称为空");
	}
	if(inMap["factory_name"].length() > 20)
    {
        ErrorLog("关键字段factory_name超长");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"工厂名称超出长度限制");
	}
}