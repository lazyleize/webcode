#include "CQueryStandLog.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"
#include "xml/unicode.h"

int CQueryStandLog::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,tmpMap,outMap,sessionMap;
	string uid;

	// 取得请求参数
    pReqData->GetStrMap(inMap);

	this->CheckLogin(sessionMap);
	
	//检查输入参数
	CheckParameter(inMap);

	size_t nPos;
	tmpMap["log_id"] = inMap["id"];
	CIdentRelayApi::GetStandLogPath(tmpMap,outMap,true);

	CStr2Map returnMap;
	CIdentComm::DelMapF(outMap,returnMap);
	
	string  downLoadPath = returnMap["log_path"];
	char cmd[256]={0};

	//核对文件是否存在，不存在就重新从MINIO拉取
	if(!CIdentPub::exists(downLoadPath))
	{
		//从MinIO去拉取日志
		string strLocalPath= downLoadPath.substr(0, downLoadPath.find_last_of('/'));
		snprintf(cmd,sizeof(cmd),"mkdir -p %s && chmod 777 %s",strLocalPath.c_str(),strLocalPath.c_str());
    	system(cmd);
		DebugLog("cmd=[%s]",cmd);
		if(0 != CIdentPub::DownloadFileFromMinIO(g_S3Cfg,returnMap["log_minio_path"].c_str(),(char*)downLoadPath.c_str()))
		{
			ErrorLog("获取不到文件");
			CIdentPub::SendAlarm2("Get file error[%s]",inMap["minio_path"].c_str());
        	throw(CTrsExp(ERR_UPFILE_COUNT, "没有找到文件，注意：没有2024年3月1号之前的生产日志"));
			return 0;
		}
	}

	nPos = downLoadPath.find("download");
	if(nPos != string::npos)
		downLoadPath = downLoadPath.substr(nPos);

	pResData->SetPara("file_path",downLoadPath);
	return 0;
}

// 输入判断
void CQueryStandLog::CheckParameter( CStr2Map& inMap)
{
	if(inMap["id"].empty())
    {
        ErrorLog("关键字段不能为空-id");
		CIdentPub::SendAlarm2("Key Fields-id is empty[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段id为空");
	}
}
