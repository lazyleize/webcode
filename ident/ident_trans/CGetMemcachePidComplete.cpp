#include "CGetMemcachePidComplete.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"
#include "xml/unicode.h"

#include <base/datetime.hpp>

using namespace aps;
int CGetMemcachePidComplete::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,tmpMap,outMap,sessionMap;
	string uid;

	// 取得请求参数
    pReqData->GetStrMap(inMap);
	
	//检查输入参数
	CheckParameter(inMap);

    //去缓存查询，根据pack_id查询个数，并返回
    string cache_value;
    int ret = GetCache(inMap["pack_id"], cache_value);
    
    if (ret == 0 && !cache_value.empty())
    {
        // 查询成功，返回完成个数
        pResData->SetPara("complete_count", cache_value);
        InfoLog("查询缓存成功: pack_id=[%s], complete_count=[%s]",inMap["pack_id"].c_str(), cache_value.c_str());
    }
    else
    {
        // 缓存不存在或查询失败，返回0或空
        pResData->SetPara("complete_count", "0");
        DebugLog("缓存不存在或查询失败: pack_id=[%s]", inMap["pack_id"].c_str());
    }
    
	return 0;
}

// 输入判断
void CGetMemcachePidComplete::CheckParameter( CStr2Map& inMap)
{
    if(inMap["pack_id"].empty())
    {
        ErrorLog("pack_id为空");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段为空-pack_id");
    }
}
