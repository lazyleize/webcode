#include "CUpLoadLog.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"

#include <base/charset.hpp>

using namespace aps;

int CUpLoadLog::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,outMap,sessionMap;
	string uid;
	// 取得请求参数
    pReqData->GetStrMap(inMap);

	char localSavePath[256]={0};
	char szDebugFullPath[512]={0};
	char szStandFullPath[512]={0};
	char szFileName[51]={0};
	
	memcpy(localSavePath,g_mTransactions[GetTid()].m_mVars["upload_path"].c_str(),
                      g_mTransactions[GetTid()].m_mVars["upload_path"].length());


	memcpy(szFileName,inMap["debug_path"].c_str(),inMap["debug_path"].length()+1);  
	DebugLog("szFileName=[%s]",szFileName);
	inMap["szFileName"] = szFileName;

	int iCount = Tools::StrToLong(g_mTransactions[GetTid()].m_mVars["file_count"].c_str());
	DebugLog("iCount=[%d]",iCount);
	CIdentComm::UploadFile(pReqData,inMap,iCount,false);

	if ( iCount > 0 )//保存用户上传的原文件
        CIdentComm::UploadFile(pReqData,inMap,iCount,false);
	else
	{
		ErrorLog("文件数量不对");
        throw(CTrsExp(ERR_UPFILE_COUNT, "文件数量不对"));
		return 0;
	}

	if(inMap["deg_file_content"].empty()||inMap["std_file_content"].empty())
	{
		ErrorLog("找到文件内容字段");
        throw(CTrsExp(ERR_UPFILE_COUNT, "找到文件内容字段"));
		return 0;
	}

	DebugLog("deg_file_content=[%s]",inMap["deg_file_content"].c_str());
	DebugLog("std_file_content=[%s]",inMap["std_file_content"].c_str());
	
	//转UTF-8
	inMap["stand_minio_path"] = _G2U(inMap["stand_minio_path"]);
	inMap["debug_minio_path"] = _G2U(inMap["debug_minio_path"]);

	DebugLog("stand_minio_path=[%s]",inMap["stand_minio_path"].c_str());
	DebugLog("debug_minio_path=[%s]",inMap["debug_minio_path"].c_str());
	

	snprintf(szDebugFullPath,sizeof(szDebugFullPath),"%s%s",localSavePath,inMap["deg_file_content"].c_str());
	snprintf(szStandFullPath,sizeof(szStandFullPath),"%s%s",localSavePath,inMap["std_file_content"].c_str());

	DebugLog("szDebugFullPath=[%s]",szDebugFullPath);
	DebugLog("szStandFullPath=[%s]",szStandFullPath);


	//开始上传
	if(0 != CIdentPub::UploadFile2MinIO(g_S3Cfg,szStandFullPath,inMap["stand_minio_path"].c_str()))
	{
		ErrorLog("标准日志上传失败");
		CIdentPub::SendAlarm2("标准日志上传失败 [%s]",inMap["stand_minio_path"].c_str());
		throw(CTrsExp(ERR_UPFILE_COUNT, "标准日志上传失败"));
		return 0;
	}

	if(0 != CIdentPub::UploadFile2MinIO(g_S3Cfg,szDebugFullPath,inMap["debug_minio_path"].c_str()))
	{
		ErrorLog("Debug日志上传失败");
		CIdentPub::SendAlarm2("Debug日志上传失败 [%s]",inMap["debug_minio_path"].c_str());
		throw(CTrsExp(ERR_UPFILE_COUNT, "Debug日志上传失败"));
		return 0;
	}

	DebugLog("done!");
	::unlink(szDebugFullPath);
	::unlink(szStandFullPath);

	return 0;

}

