#include "CDownloadPointLog.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"
#include "xml/unicode.h"

#include <base/file.hpp>

using namespace aps;
int CDownloadPointLog::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,tmpMap,outMap,sessionMap;
	string uid;

	// 取得请求参数
    pReqData->GetStrMap(inMap);

	this->CheckLogin(sessionMap);
	
	//检查输入参数
	CheckParameter(inMap);

	char basePath[256]={0};
    memcpy(basePath,g_mTransactions[GetTid()].m_mVars["file_path_base"].c_str(),
                      g_mTransactions[GetTid()].m_mVars["file_path_base"].length());

	size_t nPos;
	CIdentRelayApi::QueryTermRun(inMap,tmpMap,true);

	CStr2Map returnMap;
	CIdentComm::DelMapF(tmpMap,returnMap);

    FileInfo m_fileInfo(returnMap["ile_path"]);
    if(!File::exists(returnMap["ile_path"]))
    {
        ErrorLog("文件不存在[%s]",returnMap["ile_path"].c_str());
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"文件不存在");
    }

    string downPath = basePath + m_fileInfo.name();
    //拷贝到对应的目录
    string command = "cp " + returnMap["ile_path"] + " " + downPath;
    int result = system(command.c_str());
    if (result != 0)
    {
        ErrorLog("拷贝错误[%s]",command.c_str());
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"拷贝错误");
    }

	string  downLoadPath = downPath;

	nPos = downLoadPath.find("download");
	if(nPos != string::npos)
		downLoadPath = downLoadPath.substr(nPos);

	pResData->SetPara("file_path",downLoadPath);
	
	return 0;
}

// 输入判断
void CDownloadPointLog::CheckParameter( CStr2Map& inMap)
{
	if(inMap["id"].empty())
    {
        ErrorLog("关键字段不能为空-id");
		CIdentPub::SendAlarm2("Key Fields-id is empty[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段id为空");
	}
}
