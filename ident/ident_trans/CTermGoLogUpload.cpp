#include "CTermGoLogUpload.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"
// 取消宏定义，避免冲突
#undef max
#undef min
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson; 


int CTermGoLogUpload::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,outMap,insertInMap;
	string uid;
	// 取得请求参数
    pReqData->GetStrMap(inMap);

	char localSavePath[256]={0};
	char szDebugFullPath[512]={0};
	char szStandFullPath[512]={0};
	char szSavePath[125]={0};
	
	memcpy(localSavePath,g_mTransactions[GetTid()].m_mVars["save_path"].c_str(),
                      g_mTransactions[GetTid()].m_mVars["save_path"].length());
                      
	DebugLog("localSavePath=[%s]",localSavePath);

	vector<cgicc::FormFile> vcFilelist;
    pReqData->GetUploadFileList(vcFilelist);

    int iCount = Tools::StrToLong(g_mTransactions[GetTid()].m_mVars["file_count"].c_str());
	DebugLog("iCount=[%d],size=%d",iCount,vcFilelist.size());

	if(iCount != vcFilelist.size())
	{
		ErrorLog("文件数量不对");
        throw(CTrsExp(ERR_UPFILE_COUNT, "文件数量不对"));
		return 0;
	}

	string strFullPath;
	string szFileName = saveUploadFile(vcFilelist[0],strFullPath);
	DebugLog("strFullPath=[%s]",strFullPath.c_str());

	//入库
	insertInMap["filepath"] = strFullPath;
    insertInMap["pid"] = inMap["pid"];
    insertInMap["go_name"] = inMap["go_name"];
	CIdentRelayApi::TermGoLogUpload(insertInMap,outMap,true);

	return 0;

}


// 输入判断
void CTermGoLogUpload::CheckParameter( CStr2Map& inMap)
{
	if(inMap["go_name"].empty())
    {
        ErrorLog("终端GO日志上传关键字段不能为空-go_name");
		CIdentPub::SendAlarm2("终端GO日志上传关键字段不能为空-go_name[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"终端GO日志上传关键字段不能为空");
	}
    if(inMap["pid"].empty())
    {
        ErrorLog("终端GO日志上传关键字段不能为空-pid");
		CIdentPub::SendAlarm2("终端GO日志上传关键字段不能为空-pid[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"终端GO日志上传关键字段不能为空");
    }
}

