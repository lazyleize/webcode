#include "CGetDocumentDescribeReport.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"
#include <cstdlib> // for system
#include <cstdio>  // for popen and pclose

// 取消宏定义，避免冲突
#undef max
#undef min
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

int CGetDocumentDescribeReport::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,outMap,sessionMap;
    vector<CStr2Map> vectmapArray;

    // 取得请求参数
    pReqData->GetStrMap(inMap);
    //调试屏蔽
	//this->CheckLogin(sessionMap);

    /*inMap["usertype"] = sessionMap["usertype"];
    int usertype = atoi(sessionMap["usertype"].c_str());
    if(usertype != 2  && usertype != 1)
    {
        ErrorLog("没有权限: usertype=[%s]" ,inMap["usertype"].c_str());
        throw CTrsExp(ERR_NO_LOWER_LEVER,"没有权限");
    }*/
    
    //从配置获取操作文档的路径
    char pdfpath[256]={0};
    memcpy(pdfpath,g_mTransactions[GetTid()].m_mVars["pdf_down_path"].c_str(),
					   g_mTransactions[GetTid()].m_mVars["pdf_down_path"].length());


    //处理 strDonwPath
    string downPath = pdfpath;
    string downPdf;
    size_t nPos = downPath.find("download");
    if(nPos != string::npos)
        downPdf = downPath.substr(nPos);

    pResData->SetPara("pdf_url",downPdf);


	return 0;
}
