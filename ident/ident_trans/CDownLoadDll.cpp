#include "CDownLoadDll.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"
#include "xml/unicode.h"

int CDownLoadDll::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,outMap,sessionMap;
	string uid;

	// 取得请求参数
    pReqData->GetStrMap(inMap);

	this->CheckLogin(sessionMap);
	
	//检查输入参数
	CheckParameter(inMap);

	//去数据库获取路径
	CIdentRelayApi::GetDllPath(inMap,outMap,true);

	size_t nPos;
	string  downLoadPath = outMap["file_down"];

	nPos = downLoadPath.find("download");
	if(nPos != string::npos)
		downLoadPath = downLoadPath.substr(nPos);
	
	pResData->SetPara("file_name",outMap["file_name"]);
	pResData->SetPara("file_path",downLoadPath);

	return 0;
}

// 输入判断
void CDownLoadDll::CheckParameter( CStr2Map& inMap)
{
	if(inMap["dll_id"].empty())
    {
        ErrorLog("关键字段不能为空-id");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段id为空");
	}
}
