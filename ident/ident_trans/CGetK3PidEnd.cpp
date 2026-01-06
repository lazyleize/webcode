#include "CGetK3PidEnd.h"
#include "CIdentRelayApi.h"

#undef max
#undef min
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include "rapidjson/stringbuffer.h"

int CGetK3PidEnd::IdentCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map inMap,outMap,sessionMap;
	string uid;

	// 取得请求参数
    pReqData->GetStrMap(inMap);
	
    this->CheckLogin(sessionMap);

	//检查输入参数
	CheckParameter(inMap);
    

    //去查询这个订单号的组装PID最小值
    CIdentRelayApi::GetAssMinPid(inMap,outMap,false);
    if(outMap["ass_pid"].length()!=0)
        pResData->SetPara("pid_start",outMap["ass_pid"]);

    //返回
    pResData->SetPara("order",inMap["order"]);
    pResData->SetPara("model_name",inMap["model_name"]);
    pResData->SetPara("count",inMap["count"]);
    pResData->SetPara("materi_63",inMap["materi_63"]);
    
    pResData->SetPara("svn_path",inMap["svn_path"]);
    //pResData->SetPara("file_name",getFileNameFromUrl(strSVNpath));

}

void CGetK3PidEnd::CheckParameter( CStr2Map& inMap)
{
	if(inMap["order"].empty())
    {
        ErrorLog("关键字段不能为空-order");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段order为空");
    }
    if(inMap["svn_path"].empty())
    {
        ErrorLog("关键字段不能为空-svn_path");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段svn_path为空");
    }
    if(inMap["count"].empty())
    {
        ErrorLog("关键字段不能为空-count");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段count为空");
    }
}