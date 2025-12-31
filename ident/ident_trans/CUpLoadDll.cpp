#include "CUpLoadDll.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"
#include <sys/types.h>
#include <dirent.h>

int getdirs(string path,vector<string>& vecFileNames)
{
	DIR* dir;
	struct dirent* ptr;

	if((dir = opendir(path.c_str())) == NULL)
	{
		DebugLog("opendir error");
		return -1;
	}

	while((ptr = readdir(dir))!= NULL)
	{
		if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)
			continue;
		else if(ptr->d_type == 8)
			vecFileNames.push_back(path + ptr->d_name);
		else if(ptr->d_type == 10)
			continue;
		else if(ptr->d_type == 4)
			vecFileNames.push_back(path + ptr->d_name);
	}
	closedir(dir);
	return 0;
}

int CUpLoadDll::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,outMap,sessionMap;
	// 取得请求参数
    pReqData->GetStrMap(inMap);

	this->CheckLogin(sessionMap);

	inMap["usertype"] = sessionMap["usertype"];
    int usertype = atoi(sessionMap["usertype"].c_str());
    if(usertype != 2 && usertype != 1)
    {
        ErrorLog("没有权限: usertype=[%s]" ,inMap["usertype"].c_str());
        throw CTrsExp(ERR_NO_LOWER_LEVER,"没有权限");
    }

	char szFilePath[125]={0};
	char szFileName[51]={0};
	char szSavePath[125]={0};
	char szSavetmp[125]={0};
	char cmd[256]={0};
	
	
	memcpy(szFilePath,g_mTransactions[GetTid()].m_mVars["upload_path"].c_str(),
                      g_mTransactions[GetTid()].m_mVars["upload_path"].length());

	memcpy(szSavePath,g_mTransactions[GetTid()].m_mVars["save_path"].c_str(),
                      g_mTransactions[GetTid()].m_mVars["save_path"].length());

	memcpy(szFileName,inMap["file_name"].c_str(),inMap["file_name"].length()+1);  
	DebugLog("szFileName=[%s]",szFileName);
	inMap["szFileName"] = szFileName;

	int iCount = Tools::StrToLong(g_mTransactions[GetTid()].m_mVars["file_count"].c_str());
	DebugLog("iCount=[%d]",iCount);

	if ( iCount > 0 )//保存用户上传的文件
        CIdentComm::UploadFile(pReqData,inMap,iCount,true);
	else
		return 0;

	if(checkVersion(inMap["version"].c_str()) != 0)
	{
		ErrorLog("版本号错误 %s", inMap["version"].c_str());
        throw CTrsExp(ERR_UPFILE_COUNT, "版本号错误");
	}

	DebugLog("version=[%s]",inMap["version"].c_str());

	snprintf(cmd,sizeof(cmd),"cd %s && rm -fr tmpfile ",szFilePath);
	system(cmd);
    DebugLog("cmd=[%s]",cmd);

	vector<cgicc::FormFile> vcFilelist;
    m_pReqData->GetUploadFileList(vcFilelist);
	int nSize = vcFilelist[0].getDataLength();
	
	memset(cmd,0x00,sizeof(cmd));
	snprintf(cmd,sizeof(cmd),"cd %s && mkdir %s",szSavePath,inMap["version"].c_str());
    system(cmd);
	DebugLog("cmd=[%s]",cmd);

	strcpy(szSavetmp,szSavePath);
	strcat(szSavePath,inMap["version"].c_str());
	strcat(szSavePath,"/");
	strcat(szSavePath,szFileName);
	DebugLog("saveFullPath=[%s]",szSavePath);

	inMap["size"]= toString(nSize/1000);
	inMap["file_donwn_path"]= szSavePath;

	
	//算MD5值
	memset(cmd,0x00,sizeof(cmd));
	snprintf(cmd,sizeof(cmd),"cd %s && unzip -q %s -d tmpfile ",szFilePath,inMap["file"].c_str());
	system(cmd);
    DebugLog("cmd=[%s]",cmd);
	//加延时
	usleep(10000);

	string MD5Dir = szFilePath ;
	MD5Dir += "tmpfile/";
	vector<string> vec;
	getdirs(MD5Dir,vec);
	if(vec.size() != 1)
	{
		ErrorLog("dir count error=[%d]",vec.size());
        throw CTrsExp(ERR_UPFILE_COUNT, "目录数量不对");
	}
	string fullpathMD5 =  vec[0] + "/AutoDownloadTLS.dll";

	DebugLog("fullpathMD5=[%s]",fullpathMD5.c_str());
	string mdValue = Tools::MD5file(fullpathMD5);
	DebugLog("mdValue=[%s]",mdValue.c_str());
	inMap["md5value"]= mdValue;

	//入库
	CIdentRelayApi::SaveDllFile(inMap,outMap,true);

	memset(cmd,0x00,sizeof(cmd));
	snprintf(cmd,sizeof(cmd),"mv %s%s %s",szFilePath,inMap["file"].c_str(),szSavePath);
	system(cmd);
    DebugLog("cmd=[%s]",cmd);

	return 0;
}


int CUpLoadDll::checkVersion(const char* version)
{
	int tmpRet = 0;
    char DotCount = 0;
    int sLen = strlen(version);
    if (sLen == 0)
    {
        return false;
    }
    while (sLen--)
    {
        if (version[sLen] == 0x2e)
        {
            DotCount++;
            if (DotCount > 2)
            {
                tmpRet = -1;
                break;
            }
            continue;
        }
        if (version[sLen] > 0x39 || version[sLen] < 0x30)
        {
            tmpRet = -1;
            break;
        }
    }
	return tmpRet;
}