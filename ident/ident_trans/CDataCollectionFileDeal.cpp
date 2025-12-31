#include "CDataCollectionFileDeal.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"
// 取消宏定义，避免冲突
#undef max
#undef min
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson; 


int CDataCollectionFileDeal::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,outMap;
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

	//获取数据，然后解析入库
	ifstream inFile(strFullPath.c_str(), std::ios::in | std::ios::binary);
    if (!inFile)
	{
		ErrorLog("文件打开失败[%s]",strFullPath.c_str());
		throw(CTrsExp(ERR_UPFILE_COUNT, "文件打开失败"));
		return 0;
	}

	std::ostringstream oss;
    oss << inFile.rdbuf(); // 读取整个文件内容到ostringstream中

    // 关闭文件
    inFile.close();
	string filecontext = oss.str();

    sendToElasticsearch(filecontext,outMap);

	//开始解析JSON
	Document document;
	if(document.Parse(filecontext.c_str()).HasParseError())
	{
		ErrorLog("JSON解析失败",strFullPath.c_str());
		throw(CTrsExp(ERR_UPFILE_COUNT, "文件打开失败"));
		return 0;
	}

	if(document.HasMember("timestamp") && document["timestamp"].IsString())
		inMap["strTime"] = document["timestamp"].GetString();

	if(document.HasMember("type") && document["type"].IsString())
		inMap["strType"] = document["type"].GetString();

	if(document.HasMember("result") && document["result"].IsString())
		inMap["strResult"] = document["result"].GetString();
	

	//入库
	inMap["filepath"] = strFullPath;
	CIdentRelayApi::RecordDataAcquisi(inMap,outMap,true);

	return 0;

}

void CDataCollectionFileDeal::sendToElasticsearch(string& inStr,CStr2Map& outMap)
{
    string  strResponse;
    string strUrl =  "http://localhost:9200/collection/_doc";

    if(CIdentPub::HttpESPost(strUrl,inStr,strResponse))
    {
        InfoLog("http post fail %s", strUrl.c_str()); 
		return ;
    }
    InfoLog("ES. 返回报文[%s]",strResponse.c_str());
}


