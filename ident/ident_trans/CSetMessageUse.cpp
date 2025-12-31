#include "CSetMessageUse.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"
#include <cstdlib> // for system
#include <cstdio>  // for popen and pclose

int CSetMessageUse::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,outMap,sessionMap;

	//取得所有的请求参数
	string strPostdata=pReqData->GetPostData();
	CIdentPub::parsePubRespJson(strPostdata,inMap);

    //调试屏蔽
	this->CheckLogin(sessionMap);

    /*inMap["usertype"] = sessionMap["usertype"];
    int usertype = atoi(sessionMap["usertype"].c_str());
    if(usertype != 2  && usertype != 1)
    {
        ErrorLog("没有权限: usertype=[%s]" ,inMap["usertype"].c_str());
        throw CTrsExp(ERR_NO_LOWER_LEVER,"没有权限");
    }*/
	
	CheckParameter(inMap);

    //根据包ID去获取记录，并更新记录状态
    if(!CIdentRelayApi::SetMessageBindUser(inMap,outMap,true))
    {
        ErrorLog("更新消息类型绑定表user_name[%s]" ,inMap["user_name"].c_str());
        throw CTrsExp(ERR_NO_LOWER_LEVER,"更新消息类型绑定表失败");
    }

	return 0;
}

// 输入判断
void CSetMessageUse::CheckParameter( CStr2Map& inMap)
{
	if(inMap["user_name"].empty())
    {
        ErrorLog("关键字段不能为空-user_name");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段user_name为空");
    }
    if(inMap["method"].empty())
    {
        ErrorLog("关键字段不能为空-method");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段method为空");
    }
    if(inMap["method"]=="email" || inMap["method"]=="mix")
    {
        if(inMap["address"].empty())
        {
            ErrorLog("关键字段不能为空-address");
            throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段address为空");
        }
    }
}
