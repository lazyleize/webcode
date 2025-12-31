#include "CSvnRetryFirm.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"
#include <cstdlib> // for system
#include <cstdio>  // for popen and pclose

int CSvnRetryFirm::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,outMap,sessionMap;

	//取得所有的请求参数
	string strPostdata=pReqData->GetPostData();
	CIdentPub::parsePubRespJson(strPostdata,inMap);

    //调试屏蔽
	this->CheckLogin(sessionMap);

    inMap["usertype"] = sessionMap["usertype"];
    int usertype = atoi(sessionMap["usertype"].c_str());
    if(usertype != 2  && usertype != 1)
    {
        ErrorLog("没有权限: usertype=[%s]" ,inMap["usertype"].c_str());
        throw CTrsExp(ERR_NO_LOWER_LEVER,"没有权限");
    }
	
	CheckParameter(inMap);

    //根据包ID去获取记录，并更新记录状态
    inMap["state"]="0";
    if(!CIdentRelayApi::FirmDownloadRuleUpdate(inMap,outMap,true))
    {
        ErrorLog("更新规则表失败rules_id[%s]" ,inMap["rules_id"].c_str());
        throw CTrsExp(ERR_NO_LOWER_LEVER,"更新规则表失败");
    }

    //触发程序去SVN拉取固件包
    char cmd[256];
    memset(cmd,0x00,sizeof(cmd));
    snprintf(cmd,sizeof(cmd),"/home/httpd/web/web_pay5/tools/bin/start_app %s > /dev/null 2>&1 &",inMap["rules_id"].c_str());
    InfoLog(": cmd=[%s]" ,cmd);
    system(cmd);
    
	return 0;
}

// 输入判断
void CSvnRetryFirm::CheckParameter( CStr2Map& inMap)
{
	if(inMap["rules_id"].empty())
    {
        ErrorLog("关键字段不能为空-rules_id");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段rules_id为空");
    }
}
