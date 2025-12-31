#include "CDataAcquisition.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"
// 取消宏定义，避免冲突
#undef max
#undef min
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <base/datetime.hpp>

using namespace rapidjson; 


int CDataAcquisition::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,outMap;
	string uid;

	//取得所有的请求参数
	string strPostdata=pReqData->GetPostData();
    //InfoLog(": strPostdata=[%s]" ,strPostdata.c_str());

	// 取得请求参数
    pReqData->GetStrMap(inMap);

	//检查输入参数
	CheckParameter(inMap);
	if(inMap["pid"].length()<6)//明显PID是乱填的不记录--龙龙沟通过
		return 0;
	GetType(inMap);

	char localSavePath[256]={0};
	char szESPath[256]={0};//ES存储路径
	
	memcpy(localSavePath,g_mTransactions[GetTid()].m_mVars["save_path"].c_str(),
                      g_mTransactions[GetTid()].m_mVars["save_path"].length());
	memcpy(szESPath,g_mTransactions[GetTid()].m_mVars["es_save_url"].c_str(),
                      g_mTransactions[GetTid()].m_mVars["es_save_url"].length());
                      
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
	string filecontext = oss.str();

	//开始解析JSON
	Document document;
	if(document.Parse(filecontext.c_str()).HasParseError())
	{
		ErrorLog("JSON解析失败",strFullPath.c_str());
		CIdentPub::SendAlarm2("JSON解析失败[%s]",strFullPath.c_str());
		//入库
		inMap["filepath"] = strFullPath;
		inMap["esState"] = "F";
		CIdentRelayApi::RecordDataAcquisi(inMap,outMap,true);
		throw(CTrsExp(ERR_UPFILE_COUNT, "文件打开失败"));
	}

	//修改timestamp字段,换成当前时间，格式为YYYY-MM-DDTHH:MM:SS
	// 先保存原始时间到inMap["strTime"]
	if(document.HasMember("timestamp") && document["timestamp"].IsString())
	{
		inMap["strTime"] = document["timestamp"].GetString(); // 保存原始时间到数据库
	}
	
	string currentTimestamp = aps::Datetime::now().format("Y-M-DTH:I:S");
	if(document.HasMember("timestamp"))
	{
		// 如果字段已存在，更新它
		document["timestamp"].SetString(currentTimestamp.c_str(), currentTimestamp.length(), document.GetAllocator());
	}
	else
	{
		// 如果字段不存在，添加它
		document.AddMember("timestamp", Value().SetString(currentTimestamp.c_str(), document.GetAllocator()), document.GetAllocator());
	}

	//还需要修改类型type，字段名称是type，改成使用inMap["type"]
	if(!inMap["log_type"].empty())
	{
		if(document.HasMember("type"))
		{
			// 如果字段已存在，更新它
			document["type"].SetString(inMap["log_type"].c_str(), inMap["log_type"].length(), document.GetAllocator());
		}
	}

	if(document.HasMember("type") && document["type"].IsString())
		inMap["strType"] = document["type"].GetString();

	if(document.HasMember("result") && document["result"].IsString())
		inMap["strResult"] = document["result"].GetString();

	if(document.HasMember("orderNumber") && document["orderNumber"].IsString())
		inMap["order"] = document["orderNumber"].GetString();

	//添加新的字段 机型,便于统计,posname
	document.AddMember("posname", Value().SetString(inMap["posname"].c_str(), document.GetAllocator()), document.GetAllocator());
	
	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	document.Accept(writer);
	std::string modifiedJson = buffer.GetString();

	// 不覆盖源文件，只在写入ES时使用修改后的JSON
	inMap["filepath"] = strFullPath;

	//去索引表获取对应的索引名称
	CStr2Map ESmap;
	CIdentRelayApi::QueryESIndex(inMap,ESmap,true);
	string strUrl = szESPath+ESmap["Findex_name"]+"/_doc/";

	//去申请流水号
	CStr2Map SeriInMap,SeriOutMap;
	SeriInMap["code"] = "1";
	CIdentRelayApi::GetSerialNum(SeriInMap,SeriOutMap,true);

	SeriOutMap["id"] = inMap["strType"]+SeriOutMap["id"];
	strUrl += SeriOutMap["id"];//加上自定义的ID号
	sendToElasticsearch(strUrl,modifiedJson,inMap);
	
	inMap["id"] = SeriOutMap["id"];
	CIdentRelayApi::RecordDataAcquisi(inMap,outMap,true);


	if(inMap["order"].empty())
	{
		if(!inMap["ass_pid"].empty())
		{
			inMap["order"] = inMap["pid"].substr(0,5) + "-1";
		}
	}
	//去更新工厂生产概况表
	inMap["produc_type"] = "1";//表示数据采集类
	inMap["log_type"] = inMap["strType"];
	inMap["pos_name"] = inMap["posname"];
	inMap["ass_pid"] = inMap["pid"];
	if(inMap["strResult"]=="Success")
		inMap["result"] = "0";
	else
		inMap["result"] = "1";//失败
	CIdentRelayApi::UpdateTermRecord(inMap,outMap,true);

	return 0;

}

// 输入判断
void CDataAcquisition::CheckParameter( CStr2Map& inMap)
{
	if(inMap["pid"].empty())
    {
        ErrorLog("数据采集关键字段不能为空-pid");
		CIdentPub::SendAlarm2("Key pid is empty[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"数据采集关键字段pid为空");
	}
	if(inMap["type"].empty())
    {
        ErrorLog("数据采集关键字段不能为空-type");
		CIdentPub::SendAlarm2("Key Fields-type is empty[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"数据采集关键字段type为空");
    }
	if(inMap["type"]=="3")
	{
		if(inMap["sn"].empty())
		{
			ErrorLog("数据采集包装阶段sn不能为空");
			CIdentPub::SendAlarm2("Key Fields-sn is empty[%s]",ERR_SIGNATURE_INCORRECT);
        	throw CTrsExp(ERR_SIGNATURE_INCORRECT,"数据采集包装阶段sn为空");
		}
	}
	if(inMap["posname"].empty())
	{
		CStr2Map qryInMap,qryOutMap;
		qryInMap["log_type"] = "2";//找组装记录
		qryInMap["pid"] = inMap["pid"];
		//去日志表查机型
		if(!CIdentRelayApi::QueryTermName(qryInMap,qryOutMap,true))
		{
			ErrorLog("机型匹配失败");
			CIdentPub::SendAlarm2("查询机型失败[%s]",ERR_SIGNATURE_INCORRECT);
        	throw CTrsExp(ERR_SIGNATURE_INCORRECT,"机型匹配失败");
		}
		//CIdentPub::SendAlarm2("已匹配到机型[%s]",qryOutMap["Fpos_name"].c_str());
		inMap["posname"] = qryOutMap["Fpos_name"];
	}
}


void CDataAcquisition::GetType(CStr2Map& inMap)
{
	if(inMap["type"]=="1")
	{
		inMap["log_type"] = "SMT";
	}
	else if(inMap["type"]=="2")
	{
		inMap["log_type"] = "FAC";
	}
	else if(inMap["type"]=="3")
	{
		inMap["log_type"] = "PACK";
	}
	else
	{
		ErrorLog("类型获取失败");
		CIdentPub::SendAlarm2("类型获取失败[%s]",ERR_SIGNATURE_INCORRECT);
		throw CTrsExp(ERR_SIGNATURE_INCORRECT,"类型获取失败");
	}
}

void CDataAcquisition::sendToElasticsearch(const string& strUrl,string& inStr,CStr2Map& outMap)
{
    string  strResponse;

	InfoLog("ES. 请求报文[%s]",inStr.c_str());
    if(CIdentPub::HttpESPost(strUrl,inStr,strResponse))
    {
        InfoLog("http post fail %s", strUrl.c_str()); 
		outMap["es_state"] = "F";
		return;
    }
    InfoLog("ES. 返回报文[%s]",strResponse.c_str());
	//解析返回
	Document responseDoc;
	if(responseDoc.Parse(strResponse.c_str()).HasParseError())
	{
		ErrorLog("ES返回JSON解析失败");
		outMap["es_state"] = "F";
		return;
	}
	
	// 判断ES是否插入成功
	if(responseDoc.HasMember("result") && responseDoc["result"].IsString())
	{
		string result = responseDoc["result"].GetString();
		if(result == "created" || result == "updated")
		{
			outMap["es_state"] = "S";
		}
		else
		{
			ErrorLog("ES插入失败, result=[%s]", result.c_str());
			outMap["es_state"] = "F";
		}
	}
	else if(responseDoc.HasMember("error"))
	{
		ErrorLog("ES插入失败, 存在error字段");
		outMap["es_state"] = "F";
	}
	else
	{
		// 如果没有result字段也没有error字段，可能是其他情况，默认认为成功
		outMap["es_state"] = "S";
	}
	
	return ;

}