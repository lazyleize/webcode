#include "CExportData.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"
#include "xml/unicode.h"
#include <base/file.hpp>
#include <map>
#include <fstream>
#include <sstream>

int CExportData::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap;
    vector<CStr2Map> vectmapArray;
	
	// 取得请求参数
	pReqData->GetStrMap(inMap);
    string strdata=pReqData->GetPostData();
	InfoLog("strdata [%s]", strdata.c_str());
    CIdentPub::parsePubRespJsonList(strdata,inMap,vectmapArray);
	
	//检查输入参数
	CheckParameter(inMap);
	
	char szSavePath[125] = {0};
    char szPythonPath[125] = {0};
	memcpy(szSavePath, g_mTransactions[GetTid()].m_mVars["file_path_base"].c_str(),
		g_mTransactions[GetTid()].m_mVars["file_path_base"].length());
    memcpy(szPythonPath, g_mTransactions[GetTid()].m_mVars["python_path"].c_str(),
		g_mTransactions[GetTid()].m_mVars["python_path"].length());

	CStr2Map quryInMap, quryOutMap;
	vector<string> filePathList;
	map<string, string> filePossnMap;  // 存储文件路径与Fpossn的映射关系
	
	//初始化查询参数
	quryInMap["limit"] = "300";
	quryInMap["offset"] = "0";
	quryInMap["pid_beg"] = inMap["pid_beg"];
	quryInMap["pid_end"] = inMap["pid_end"];
	
	if(inMap["index"] == "1")  //结果采集数据
	{
		if(inMap["date_type"] != "ALL")
			quryInMap["type"] = inMap["date_type"];
        
        //先压缩全部文件
        int nCount = vectmapArray.size();
        ErrorLog("nCount=%d",nCount);
        for(int i = 0;i < nCount;i++)
        {
            quryInMap["pid_beg"] = vectmapArray[i]["pid_beg"]; 
            quryInMap["pid_end"] = vectmapArray[i]["pid_end"];
            CollectFilePaths(quryInMap, quryOutMap, filePathList, filePossnMap, true);
        }
		//返回压缩包完整路径
		string zipFullPath = CompressFiles(filePathList, szSavePath, pResData);
		DebugLog("zip full path = [%s]", zipFullPath.c_str());
        
		
        if(inMap["export_type"] == "SRC" )//只有源文件就可以返回了
            return 0;

		//需要去运行Python脚本
		if(inMap["export_type"] == "ANA" )//只需要excel就行
		{
			if(zipFullPath.empty())
				return 0;
			// 生成Fpossn映射JSON文件
			string possnJsonPath = GeneratePossnJsonFile(filePossnMap, szSavePath);
			// 输出文件名：合并Excel
			string excelName = CIdentPub::GetCurrentDateTime();
			excelName += "_merged.xlsx";
			char outPath[512] = {0};
			snprintf(outPath, sizeof(outPath), "%s%s", szSavePath, excelName.c_str());
			char cmd[2048] = {0};
			if(!possnJsonPath.empty())
				snprintf(cmd, sizeof(cmd), "python3 %s --zip %s --output %s --possn-json %s", 
					szPythonPath, zipFullPath.c_str(), outPath, possnJsonPath.c_str());
			else
				snprintf(cmd, sizeof(cmd), "python3 %s --zip %s --output %s", 
					szPythonPath, zipFullPath.c_str(), outPath);
			InfoLog("run python excel cmd: [%s]", cmd);
			system(cmd);
			// 返回Excel下载路径
			string downLoadPath = outPath;
			size_t nPos = downLoadPath.find("download");
			if(nPos != string::npos) downLoadPath = downLoadPath.substr(nPos);
			pResData->SetPara("url", downLoadPath);
			return 0;
		}
		else if(inMap["export_type"] == "ALL" )//需要源文件和excel
		{
			if(zipFullPath.empty())
				return 0;
			// 生成Fpossn映射JSON文件
			string possnJsonPath = GeneratePossnJsonFile(filePossnMap, szSavePath);
			// 输出文件名：最终zip，包含源zip与合并Excel
			string finalZipName = CIdentPub::GetCurrentDateTime();
			finalZipName += "_final.zip";
			char outPath[512] = {0};
			snprintf(outPath, sizeof(outPath), "%s%s", szSavePath, finalZipName.c_str());
			char cmd[2048] = {0};
			if(!possnJsonPath.empty())
				snprintf(cmd, sizeof(cmd), "python3 %s --zip %s --output %s --possn-json %s", 
					szPythonPath, zipFullPath.c_str(), outPath, possnJsonPath.c_str());
			else
				snprintf(cmd, sizeof(cmd), "python3 %s --zip %s --output %s", 
					szPythonPath, zipFullPath.c_str(), outPath);
			InfoLog("run python final zip cmd: [%s]", cmd);
			system(cmd);
			// 返回最终zip下载路径
			string downLoadPath = outPath;
			size_t nPos = downLoadPath.find("download");
			if(nPos != string::npos) downLoadPath = downLoadPath.substr(nPos);
			pResData->SetPara("url", downLoadPath);
			return 0;
		}

		return 0;
	}
	else if(inMap["index"] == "2")  //终端运行数据
	{
		//先压缩全部文件
        int nCount = vectmapArray.size();
        ErrorLog("nCount=%d",nCount);
        for(int i = 0;i < nCount;i++)
        {
            quryInMap["pid_beg"] = vectmapArray[i]["pid_beg"]; 
            quryInMap["pid_end"] = vectmapArray[i]["pid_end"];
            CollectFilePaths(quryInMap, quryOutMap, filePathList, filePossnMap, false);
        }
		//返回压缩包完整路径
		string zipFullPath = CompressFiles(filePathList, szSavePath, pResData);
		DebugLog("zip full path = [%s]", zipFullPath.c_str());
		return 0;
	}
	else if(inMap["index"] == "3")  //终端埋点数据
	{
		// TODO: 实现终端埋点数据的导出逻辑
		ErrorLog("终端埋点数据导出功能暂未实现");
		pResData->SetPara("total", "0");
		return 0;
	}
	else
	{
		ErrorLog("未知类型: [%s]", inMap["index"].c_str());
		throw(CTrsExp(ERR_UPFILE_COUNT, "未知类型"));
	}
	
	return 0;
}

// 收集文件路径
void CExportData::CollectFilePaths(CStr2Map& quryInMap, CStr2Map& quryOutMap, 
	vector<string>& filePathList, map<string, string>& filePossnMap, bool isCollectData)
{
	vector<CStr2Map> vectmapArray;
	int current_offset = 0;
	int deal_num = 0;
	int ret_num = 0;
	
	while(1)
	{
		vectmapArray.clear();
		
		//批量查询文件路径
		if(isCollectData)
		{
			CIdentRelayApi::QueryPidRangCollectList(quryInMap, quryOutMap, vectmapArray, true);
		}
		else
		{
			CIdentRelayApi::QueryPidRangTermRunList(quryInMap, quryOutMap, vectmapArray, true);
		}

		ret_num = atoi(quryOutMap["ret_num"].c_str());
		
		//获取每个记录的路径
		for(size_t i = 0; i < vectmapArray.size(); i++)
		{
			if(vectmapArray[i].find("File_path") != vectmapArray[i].end() && 
				!vectmapArray[i]["File_path"].empty())
			{
				string filePath = vectmapArray[i]["File_path"];
				filePathList.push_back(filePath);
				
				//收集Fpossn数据（仅对结果采集数据）
				if(isCollectData && vectmapArray[i].find("Fpossn") != vectmapArray[i].end())
				{
					// 使用文件名作为key（去掉路径，只保留文件名）
					size_t pos = filePath.find_last_of("/");
					string fileName = (pos != string::npos) ? filePath.substr(pos + 1) : filePath;
					filePossnMap[fileName] = vectmapArray[i]["Fpossn"];
					DebugLog("File: %s, Fpossn: %s", fileName.c_str(), vectmapArray[i]["Fpossn"].c_str());
				}
				
				deal_num++;
			}
		}
		
		//判断是否处理完成
		if(atoi(quryOutMap["total"].c_str()) == deal_num)
		{
			InfoLog("已处理完,总共[%d]个记录", deal_num);
			break;
		}
		
		//累加当前查询到的记录数到偏移量
		current_offset += ret_num;
		quryInMap["offset"] = to_string(current_offset);
		quryInMap["limit"] = "300";

		sleep(1);//释放一下资源
	}
}

// 压缩文件
string CExportData::CompressFiles(const vector<string>& filePathList, const char* szSavePath, CResData* pResData)
{
	if(filePathList.size() == 0)
	{
		ErrorLog("没有找到文件");
		pResData->SetPara("total", "0");
		return string("");
	}
	
	// 根据当前时间生成压缩包的名称
	char cmd[2048] = {0};
	char szResult[256] = {0};
	string cmdstr;
	string ZipFileName = CIdentPub::GetCurrentDateTime();
	ZipFileName += ".zip";
	
	snprintf(cmd, sizeof(cmd), "zip -jq3 %s%s", szSavePath, ZipFileName.c_str());
	cmdstr = cmd;
	
	//添加所有存在的文件到压缩命令
	for(size_t i = 0; i < filePathList.size(); i++)
	{
		//检查文件是否存在
		if(!aps::File::exists(filePathList[i]))
		{
			ErrorLog("文件不存在，跳过 [%s]", filePathList[i].c_str());
			continue;
		}
		cmdstr += " ";
		cmdstr += filePathList[i];
	}
	
	//执行压缩命令
	system(cmdstr.c_str());
	snprintf(szResult, sizeof(szResult), "%s%s", szSavePath, ZipFileName.c_str());
	
	//提取相对路径用于返回
	size_t nPos;
	string downLoadPath = szResult;
	nPos = downLoadPath.find("download");
	if(nPos != string::npos)
		downLoadPath = downLoadPath.substr(nPos);
	
	InfoLog("压缩完成，文件路径[%s]，共压缩[%lu]个文件", downLoadPath.c_str(), filePathList.size());
	pResData->SetPara("url", downLoadPath);
	pResData->SetPara("total", to_string(filePathList.size()));

	// 返回压缩包的完整路径（绝对路径）
	return string(szResult);
}

// 生成Fpossn映射JSON文件
string CExportData::GeneratePossnJsonFile(const map<string, string>& filePossnMap, const char* szSavePath)
{
	if(filePossnMap.empty())
	{
		DebugLog("Fpossn映射数据为空，不生成JSON文件");
		return string("");
	}
	
	// 生成JSON文件名
	string jsonFileName = CIdentPub::GetCurrentDateTime();
	jsonFileName += "_possn.json";
	char jsonFilePath[512] = {0};
	snprintf(jsonFilePath, sizeof(jsonFilePath), "%s%s", szSavePath, jsonFileName.c_str());
	
	// 创建JSON文件
	ofstream jsonFile(jsonFilePath);
	if(!jsonFile.is_open())
	{
		ErrorLog("无法创建Fpossn JSON文件: [%s]", jsonFilePath);
		return string("");
	}
	
	// 写入JSON内容
	jsonFile << "{" << endl;
	map<string, string>::const_iterator it = filePossnMap.begin();
	bool first = true;
	for(; it != filePossnMap.end(); ++it)
	{
		if(!first)
			jsonFile << "," << endl;
		first = false;
		// 转义文件名中的特殊字符（简单处理，只转义双引号）
		string fileName = it->first;
		size_t pos = 0;
		while((pos = fileName.find("\"", pos)) != string::npos)
		{
			fileName.replace(pos, 1, "\\\"");
			pos += 2;
		}
		// 转义Fpossn值中的特殊字符
		string possnValue = it->second;
		pos = 0;
		while((pos = possnValue.find("\"", pos)) != string::npos)
		{
			possnValue.replace(pos, 1, "\\\"");
			pos += 2;
		}
		jsonFile << "  \"" << fileName << "\": \"" << possnValue << "\"";
	}
	jsonFile << endl << "}" << endl;
	jsonFile.close();
	
	InfoLog("生成Fpossn映射JSON文件: [%s], 共[%lu]条记录", jsonFilePath, filePossnMap.size());
	return string(jsonFilePath);
}

// 输入判断
void CExportData::CheckParameter(CStr2Map& inMap)
{
	if(inMap["index"].empty())
	{
		ErrorLog("关键字段不能为空-index");
		throw CTrsExp(ERR_SIGNATURE_INCORRECT, "关键字段index为空");
	}
	
	if(inMap["index"] == "1" && inMap["date_type"].empty())  //结果采集数据
	{
		ErrorLog("关键字段不能为空-date_type");
		throw CTrsExp(ERR_SIGNATURE_INCORRECT, "关键字段date_type为空");
	}
}
