#include "CQuerySvnFilesList.h"
#include "CIdentRelayApi.h"

#include <iostream>
#include <cstdio>
#include <memory>


int CQuerySvnFilesList::IdentCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map sessMap;
	CStr2Map inMap, outMap;
	pReqData->GetStrMap(inMap);

	this->CheckLogin(sessMap);

	int limit = atol(inMap["limit"].c_str());
	int offset = atol(inMap["offset"].c_str());

	offset = (offset -1)*limit;
    inMap["limit"] = Tools::IntToStr(limit);
	inMap["offset"] = Tools::IntToStr(offset);

	inMap["usertype"] = sessMap["usertype"];
    int usertype = atoi(sessMap["usertype"].c_str());
    //不做权限校验

    if(inMap["svn_path"].empty())
    {
        ErrorLog("svn_path is null");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"svn_path is null");
    }

    //先转义
    inMap["svn_path"] = CIdentPub::escape_svn_path(inMap["svn_path"]);

    //utf-8转GBK执行 ---SVN仓库的目录是GBK
    char buff[1024] = {0} ;
    int  iDestLen = sizeof(buff)-1;

    //Tools::ConvertCharSet((char*)inMap["svn_path"].c_str(),buff,iDestLen,"utf-8","GBK");

	vector<CStr2Map> vectmapArray;

    int nTotal = listSVNDirectory(inMap["svn_path"],vectmapArray);

    pResData->SetPara("ret_num",to_string(nTotal));
    pResData->SetPara("total",to_string(nTotal));

    for(size_t i = 0;i < vectmapArray.size();++i)
    {
        //文件名转UTF-8;???实测看看
        pResData->SetArray(vectmapArray[i]);
    }

	return 0;
}


int CQuerySvnFilesList::listSVNDirectory(const std::string& repoUrl,vector<CStr2Map>& vectmapArray) 
{
    std::string command = "svn list --verbose " + repoUrl;
    std::string output = exec(command.c_str());
    DebugLog("command[%s]",command.c_str());
    
    char buff[1024*8] = {0} ;
    int  iDestLen = sizeof(buff)-1;

    //Tools::ConvertCharSet((char*)output.c_str(),buff,iDestLen,"GBK","utf-8");
    //output = buff;
    
    DebugLog("output[%s]",output.c_str());
    auto items = parseSVNOutput1(output);
    
    int nSize = items.size();
    //处理返回
    for(int i = 0;i < nSize;i++)
    {
        CStr2Map mapEle;
        if(items[i].type == "directory")
        {
            mapEle["type"]  = "D";
            mapEle["size"]  = "0";
        }
        else
        {
            mapEle["type"]  = "F";
            mapEle["size"]  = items[i].size;
        }
        mapEle["file"]      = items[i].name;
        mapEle["datetime"]  = items[i].date;

        if(mapEle.size()>0)
            vectmapArray.push_back(mapEle);
    }
    return nSize;
    
}

std::vector<SVNItem> CQuerySvnFilesList::parseSVNOutput1(const std::string& output) 
{
    std::vector<SVNItem> items;
    std::istringstream stream(output);
    std::string line;

    while (std::getline(stream, line)) 
    {
        SVNItem item;

        // 去掉末尾的斜杠
        if (line.size() >= 2 && line.substr(line.size() - 2) == "./")
            continue;

        // 从头开始解析
        std::istringstream lineStream(line);
        std::string version, owner, size;

        // 解析版本号
        lineStream >> version;

        // 解析作者
        lineStream >> owner;

        std::string remainingLine;
        std::getline(lineStream, remainingLine); // 读取剩余的行

        // 查找第一个非空格字符的位置
        size_t firstNonSpace = remainingLine.find_first_not_of(" ");
        std::string trimmedLine = remainingLine.substr(0, firstNonSpace);

        string extractedString = remainingLine.substr(firstNonSpace);
        //DebugLog("extractedString[%s]",extractedString.c_str());
        
        if(trimmedLine.length() > 15)//没有大小，是目录
        {
            item.type = "directory";
            item.size = ""; // 目录没有大小
        }
        else
        {
            item.type = "file";
            //截取大小
            size_t firstSpace = extractedString.find_first_of(" ");
            if (firstSpace != std::string::npos)
                item.size = extractedString.substr(0, firstSpace);

            extractedString = extractedString.substr(firstSpace);
        }
        
        //去空格
        string dataAndName = extractedString.substr(extractedString.find_first_not_of(" "));
        
        string datePart,spaceName;

        if( dataAndName.length() >= 4 && std::all_of(dataAndName.begin(), dataAndName.begin() + 4, ::isdigit))//是"2019-07-23"格式
        {
            datePart = dataAndName.substr(0,10);
            spaceName = dataAndName.substr(10);
        }
        else// 处理格式 "11月 07 19:45"
        {
            //没办法，只能按照空格来
            size_t nPosSpace = dataAndName.find_first_of(" ");
            datePart = dataAndName.substr(0,nPosSpace);//11月
            datePart += " ";

            dataAndName = dataAndName.substr(nPosSpace);// 07 19:45"
           
            //去空格
            dataAndName = dataAndName.substr(dataAndName.find_first_not_of(" "));//07 19:45"
            DebugLog("dataAndName[%s]",dataAndName.c_str());

            nPosSpace = dataAndName.find_first_of(" ");
            datePart += dataAndName.substr(0,nPosSpace);//07
            datePart += " ";
            dataAndName = dataAndName.substr(nPosSpace);// 19:45"
            

            //去空格
            dataAndName = dataAndName.substr(dataAndName.find_first_not_of(" "));//19:45"
            nPosSpace = dataAndName.find_first_of(" ");
            datePart += dataAndName.substr(0,nPosSpace);//19:45

            spaceName = dataAndName.substr(nPosSpace); 
        }
        item.date = datePart;
        DebugLog("spaceName[%s]",spaceName.c_str());

        //去空格
        string filename = spaceName.substr(spaceName.find_first_not_of(" "));//07 19:45"
        item.name = filename;


        items.push_back(item);
    }

    return items;
}

vector<SVNItem> CQuerySvnFilesList::parseSVNOutput(const std::string& output) 
{
    std::vector<SVNItem> items;
    std::istringstream stream(output);
    std::string line;

    while (std::getline(stream, line)) 
    {
        // 替换多个空格为一个逗号
        std::string processedLine;
        bool inSpaces = false;

        for (char c : line) 
        {
            if (c == ' ') {
                if (!inSpaces) {
                    processedLine += ','; // 只在第一个空格之后添加逗号
                    inSpaces = true; // 进入空格状态
                }
            } else {
                processedLine += c; // 复制非空格字符
                inSpaces = false; // 退出空格状态
            }
        }
        
        // 如果处理后的行以逗号开头，去掉第一个逗号
        if (!processedLine.empty() && processedLine[0] == ',') 
        {
            processedLine.erase(0, 1);
        }
        
        std::vector<std::string> fields;
        std::istringstream lineStream(processedLine);
        std::string field;

        while (std::getline(lineStream, field, ',')) 
        {
            // 去掉字段前后的空格
            field.erase(0, field.find_first_not_of(' ')); // 去掉前面的空格
            field.erase(field.find_last_not_of(' ') + 1); // 去掉后面的空格

            if (!field.empty()) {
                fields.push_back(field); // 只添加非空字段
            }
        }
        int nTotal = fields.size();

        SVNItem item;
        item.name = fields[nTotal -1];//最后一个必是文件名称
        //判断是否是目录
        if(item.name.back() == '/')//是
        {
            item.type="directory";
            item.size="0";
            item.date=fields[2];
            if(nTotal > 4)//说明是 1月 06 13:50格式
            {
                item.date += " ";
                item.date += fields[3];
                item.date += " ";
                item.date += fields[4];
            }
            //格式是：2024-06-28 不用处理
        }
        else
        {
            item.type="file";
            //大小只能在第三个字段
            item.size=fields[2];
            item.date=fields[3];
            if(nTotal > 6)//格式是 1月 06 13:50格式
            {
                item.date += " ";
                item.date += fields[4];
                item.date += " ";
                item.date += fields[5];
            }
        }

        items.push_back(item); // 添加到结果容器
    }

    return items;
}


std::string CQuerySvnFilesList::exec(const char* cmd) 
{
    std::array<char, 128> buffer;
    std::string result;

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) 
    {
        ErrorLog("popen() failed!");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"popen() failed!");
    }
    
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) 
    {
        result += buffer.data();
    }
    
    return result;
}