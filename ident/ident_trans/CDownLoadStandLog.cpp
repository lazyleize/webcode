#include "CDownLoadStandLog.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"
#include "xml/unicode.h"

int CDownLoadStandLog::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,tmpMap,outMap,sessionMap;
	string uid;

	// 取得请求参数
    pReqData->GetStrMap(inMap);

	this->CheckLogin(sessionMap);
	
	//检查输入参数
	CheckParameter(inMap);

	vector<string> VecId =  CIdentPub::split(inMap["id"],",");
	if(VecId.size()>0)
	{
		char szSavePath[125]={0};
		memcpy(szSavePath,g_mTransactions[GetTid()].m_mVars["pack_path"].c_str(),
                      g_mTransactions[GetTid()].m_mVars["pack_path"].length());

		// 根据当前时间生成压缩包的名称
		char cmd[2048]={0};
		char szResult[256]={0};
		string cmdstr;
		string ZipFileName = CIdentPub::GetCurrentDateTime();
		ZipFileName += ".zip";
		snprintf(cmd,sizeof(cmd),"zip -jq3 %s%s",szSavePath,ZipFileName.c_str());
		cmdstr = cmd;
		for(size_t i = 0;i < VecId.size();i++)
		{
			string log_path;
			string strMinIOPath;
			CIdentRelayApi::GetLogPathBath(VecId[i],log_path,strMinIOPath,true);
			if(log_path.length() == 0)
			{
				ErrorLog("日志路径不存在 [%s]",VecId[i].c_str());
				CIdentPub::SendAlarm2("日志路径不存在[%s]",VecId[i].c_str());
				continue;
			}
			memset(cmd,0x00,sizeof(cmd));
			//从MinIO去拉取日志
			string strLocalPath= log_path.substr(0, log_path.find_last_of('/'));
			snprintf(cmd,sizeof(cmd),"mkdir -p %s && chmod 777 %s",strLocalPath.c_str(),strLocalPath.c_str());
    		system(cmd);
			DebugLog("cmd=[%s]",cmd);
			if(0 != CIdentPub::DownloadFileFromMinIO(g_S3Cfg,strMinIOPath.c_str(),(char*)log_path.c_str()))
			{
				ErrorLog("获取不到文件");
				CIdentPub::SendAlarm2("获取不到文件[%s]",strMinIOPath.c_str());
        		throw(CTrsExp(ERR_UPFILE_COUNT, "文件上传失败"));
				return 0;
			}
			cmdstr += " ";
			cmdstr += log_path;
		}
		DebugLog("cmdstr [%s]",cmdstr.c_str());
		system(cmdstr.c_str());
		snprintf(szResult,sizeof(szResult),"%s%s",szSavePath,ZipFileName.c_str());

		size_t nPos;
		string  downLoadPath = szResult;
		nPos = downLoadPath.find("download");
		if(nPos != string::npos)
			downLoadPath = downLoadPath.substr(nPos);

		pResData->SetPara("file_path",downLoadPath);
	}
	
	return 0;
}

// 输入判断
void CDownLoadStandLog::CheckParameter( CStr2Map& inMap)
{
	if(inMap["id"].empty())
    {
        ErrorLog("关键字段不能为空-id");
		CIdentPub::SendAlarm2("Key Fields-id is empty[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段id为空");
	}
}
