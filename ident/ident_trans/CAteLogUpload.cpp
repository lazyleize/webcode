#include "CAteLogUpload.h"
#include "CIdentRelayApi.h"

int CAteLogUpload::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,outMap;
	string uid;
	// 取得请求参数
    pReqData->GetStrMap(inMap);

	char localSavePath[256]={0};
	char szDebugFullPath[512]={0};
	char szStandFullPath[512]={0};
	char szSavePath[125]={0};
    CheckParameter(inMap);
	
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
	inMap["filepath"] = strFullPath;
	CIdentRelayApi::ATELogUpload(inMap,outMap,true);
	return 0;

}


// 输入判断
void CAteLogUpload::CheckParameter( CStr2Map& inMap)
{
	if(inMap["sn"].empty())
    {
        ErrorLog("射频日志上传,SN号为空-sn");
		CIdentPub::SendAlarm2("射频日志上传,SN号为空 is empty[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"射频日志上传关键字段为空");
	}
    if(inMap["result"].empty())
    {
        ErrorLog("射频日志上传,结果为空-result");
		CIdentPub::SendAlarm2("射频日志上传,result为空 is empty[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"射频日志上传关键字段为空");
	}
	if(inMap["factory"].empty())
    {
        ErrorLog("射频日志上传,工厂类型为空-factory");
		CIdentPub::SendAlarm2("射频日志上传,factory为空 is empty[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"射频日志上传关键字段为空");
	}
	if(inMap["role"].empty())
    {
        ErrorLog("射频日志上传,角色类型为空-role");
		CIdentPub::SendAlarm2("射频日志上传,角色为空 is empty[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"射频日志上传关键字段为空");
	}
}

