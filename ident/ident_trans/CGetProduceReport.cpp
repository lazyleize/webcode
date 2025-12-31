#include "CGetProduceReport.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"
#include "xml/unicode.h"

#include <base/datetime.hpp>
#include <base/file.hpp>

using namespace aps;

int CGetProduceReport::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,tmpMap,outMap,sessMap;
	string uid,downPath,strDonwPath,command;
    vector<CStr2Map> vectmapArray;
    size_t nPos;

	// 取得请求参数
    pReqData->GetStrMap(inMap);

	this->CheckLogin(sessMap);

	/*inMap["usertype"] = sessMap["usertype"];
    int usertype = atoi(sessMap["usertype"].c_str());
    if(usertype != 2  && usertype != 1)
    {
        ErrorLog("普通用户没有生产报表导出权限: usertype=[%s]" ,inMap["usertype"].c_str());
        throw CTrsExp(ERR_NO_LOWER_LEVER,"没有权限");
    }*/
	
	//检查输入参数
	CheckParameter(inMap);

    //从配置文件获取报表文件下载存放的基本路径
    char szFileDownPathbase[256]={0};

	memcpy(szFileDownPathbase,g_mTransactions[GetTid()].m_mVars["report_down_path"].c_str(),
                      g_mTransactions[GetTid()].m_mVars["report_down_path"].length());
    if(inMap["type"] != "A")
    {
        if(!CIdentRelayApi::GetReportDownloadPath(inMap,tmpMap,true))
        {
            ErrorLog("获取报表路径失败");
            throw CTrsExp(ERR_SIGNATURE_INCORRECT,"获取报表路径失败");
        }
        if(!File::exists(tmpMap["Fpath"]))
        {
            ErrorLog("报表路径错误,报表不存在");
            throw CTrsExp(ERR_SIGNATURE_INCORRECT,"报表路径错误,报表不存在");
        }

        //单个，直接拷贝
        strDonwPath = szFileDownPathbase;
        strDonwPath += "/";
        strDonwPath += File::getFileInfo(tmpMap["Fpath"]).name();
        if(File::exists(strDonwPath))
            File::unlink(strDonwPath);
        command = "cp -f " + tmpMap["Fpath"] + " " + strDonwPath;
        InfoLog("command = [%s]",command.c_str());
        system(command.c_str());

        //处理 strDonwPath
        
        nPos = strDonwPath.find("download");
        if(nPos != string::npos)
            downPath = strDonwPath.substr(nPos);

        pResData->SetPara("url",downPath);
        pResData->SetPara("date",inMap["date"]);
        return 0;
    }

    //压缩一份 到基础路径
    inMap["offset"] = "0";
    inMap["limit"]  = "10"; //其实最多就2笔
    string srcFilePath;
    CIdentRelayApi::QueryReportDownList(inMap,outMap,vectmapArray,true);
    for(size_t i = 0;i < vectmapArray.size();++i)
    {
        CStr2Map returnMap;
        CIdentComm::DelMapF(vectmapArray[i],returnMap);
        // 拼出压缩
        srcFilePath += returnMap["path"];
        srcFilePath += " ";
    }

    //压缩后存放路径
    strDonwPath = szFileDownPathbase;
    strDonwPath += "/";
    strDonwPath += Datetime::now().format("YMDHIS");
    strDonwPath += ".zip";

    command = "zip -jq3 " + strDonwPath + " " +srcFilePath;
    InfoLog("command = [%s]",command.c_str());
    system(command.c_str());

    //处理 strDonwPath
    nPos = strDonwPath.find("download");
    if(nPos != string::npos)
        downPath = strDonwPath.substr(nPos);

    pResData->SetPara("url",downPath);
    pResData->SetPara("date",inMap["date"]);
    
	return 0;
}

// 输入判断
void CGetProduceReport::CheckParameter( CStr2Map& inMap)
{
	if(inMap["date"].empty())
    {
        ErrorLog("date缺少");
		CIdentPub::SendAlarm2("date缺少[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"date缺少");
    }
    if(inMap["type"].empty())
    {
        ErrorLog("type缺少");
		CIdentPub::SendAlarm2("type缺少[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"type缺少");
    }
}

  