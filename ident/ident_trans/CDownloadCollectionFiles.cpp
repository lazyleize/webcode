#include "CDownloadCollectionFiles.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"
#include "xml/unicode.h"
#include <base/file.hpp>

using namespace aps;
int CDownloadCollectionFiles::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,outMap,sessionMap,tempMap;
	string uid;

	// 取得请求参数
    pReqData->GetStrMap(inMap);

	this->CheckLogin(sessionMap);
	
	//检查输入参数
	CheckParameter(inMap);

	string strDonwPath,command,downPath,filename;

	 //从配置文件获取报表文件下载存放的基本路径
	 char szFileDownPathbase[256]={0};

	 memcpy(szFileDownPathbase,g_mTransactions[GetTid()].m_mVars["file_down_path"].c_str(),
					   g_mTransactions[GetTid()].m_mVars["file_down_path"].length());

	//判断是不是多个
	vector<string> VecId =  CIdentPub::split(inMap["id"],",");
	if(VecId.size()==1)//单个文件，直接下载
	{
		CIdentRelayApi::QueryCollectPath(inMap,tempMap,true);
		if(!File::exists(tempMap["File_path"]))
        {
            ErrorLog("数据采集文件路径错误,报表不存在");
            throw CTrsExp(ERR_SIGNATURE_INCORRECT,"数据采集文件路径错误,报表不存在");
        }
		filename = File::getFileInfo(tempMap["File_path"]).name();

		//单个，直接拷贝
        strDonwPath = szFileDownPathbase;
        strDonwPath += "/";
        strDonwPath += File::getFileInfo(tempMap["File_path"]).name();
        if(File::exists(strDonwPath))
            File::unlink(strDonwPath);
        command = "cp -f " + tempMap["File_path"] + " " + strDonwPath;
        InfoLog("command = [%s]",command.c_str());
        system(command.c_str());
		size_t nPos = strDonwPath.find("download");
        if(nPos != string::npos)
            downPath = strDonwPath.substr(nPos);
	
		pResData->SetPara("file_name",filename);
		pResData->SetPara("file_path",downPath);

	}
	else //多个
	{
		string strMuilFilePath;
		//压缩一份 到基础路径
		for(int i = 0;i < VecId.size();i++)
		{
			CStr2Map tmpInMap,tmpOutMap;
			tmpInMap["id"]=VecId[i];
			CIdentRelayApi::QueryCollectPath(tmpInMap,tmpOutMap,true);
			if(!File::exists(tmpOutMap["File_path"]))
        	{
            	ErrorLog("数据采集文件路径错误,报表不存在[%s]",tmpInMap["id"].c_str());
            	throw CTrsExp(ERR_SIGNATURE_INCORRECT,"数据采集文件路径错误,报表不存在");
        	}
			
			strMuilFilePath += tmpOutMap["File_path"];
			strMuilFilePath += "  ";
		}
		//压缩后存放路径
		strDonwPath = szFileDownPathbase;
		strDonwPath += "/";
		string strZipName = Datetime::now().format("YMDHIS") + ".zip";
		strDonwPath += strZipName;

		command = "zip -jq3 " + strDonwPath + " " +strMuilFilePath;
    	InfoLog("command = [%s]",command.c_str());
    	system(command.c_str());

		size_t nPos = strDonwPath.find("download");
    	if(nPos != string::npos)
        	downPath = strDonwPath.substr(nPos);

		pResData->SetPara("file_name",strZipName);
		pResData->SetPara("file_path",downPath);
	}

	return 0;
}

// 输入判断
void CDownloadCollectionFiles::CheckParameter( CStr2Map& inMap)
{
	if(inMap["id"].empty())
    {
        ErrorLog("关键字段不能为空-id");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段id为空");
	}
}
