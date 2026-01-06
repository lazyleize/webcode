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
        try
        {
            // 跳过空行
            if (line.empty())
                continue;
                
            SVNItem item;

            // 从头开始解析
            std::istringstream lineStream(line);
            std::string version, owner, size;

            // 解析版本号
            lineStream >> version;
            if (version.empty())
                continue;

            // 解析作者
            lineStream >> owner;
            if (owner.empty())
                continue;

            std::string remainingLine;
            std::getline(lineStream, remainingLine); // 读取剩余的行

            // 查找第一个非空格字符的位置
            size_t firstNonSpace = remainingLine.find_first_not_of(" ");
            if (firstNonSpace == std::string::npos)
                continue;
                
            std::string trimmedLine = remainingLine.substr(0, firstNonSpace);
            if (firstNonSpace >= remainingLine.length())
                continue;
            string extractedString = remainingLine.substr(firstNonSpace);
        
        // 判断是目录还是文件（目录没有大小字段，所以空格会更多）
        bool isDirectory = false;
        if(trimmedLine.length() > 15)//没有大小，是目录
        {
            isDirectory = true;
            item.type = "directory";
            item.size = "0"; // 目录大小设为0
        }
        else
        {
            item.type = "file";
            //截取大小
            size_t firstSpace = extractedString.find_first_of(" ");
            if (firstSpace != std::string::npos)
            {
                item.size = extractedString.substr(0, firstSpace);
                extractedString = extractedString.substr(firstSpace);
            }
            else
            {
                item.size = "0";
            }
        }
        
        //去空格
        size_t dataStart = extractedString.find_first_not_of(" ");
        if (dataStart == std::string::npos)
            continue;
        string dataAndName = extractedString.substr(dataStart);
        if (dataAndName.empty())
            continue;
        
        string datePart,spaceName;

        // 判断日期格式：如果是"2019-07-23"格式（前4个字符都是数字）
        if( dataAndName.length() >= 4 && std::all_of(dataAndName.begin(), dataAndName.begin() + 4, ::isdigit))
        {
            // 格式：2019-07-23 或 2019-07-23 18:01
            size_t dateEnd = dataAndName.find_first_of(" ");
            if (dateEnd != std::string::npos && dateEnd >= 10)
            {
                datePart = dataAndName.substr(0, dateEnd);
                spaceName = dataAndName.substr(dateEnd);
            }
            else if (dataAndName.length() >= 10)
            {
                datePart = dataAndName.substr(0, 10);
                spaceName = dataAndName.substr(10);
            }
            else
            {
                continue; // 格式错误，跳过
            }
        }
        else// 处理格式 "Nov 11 18:01" 或 "11月 07 19:45"
        {
            // 按照空格分割
            size_t nPosSpace = dataAndName.find_first_of(" ");
            if (nPosSpace == std::string::npos || nPosSpace == 0)
                continue;
                
            datePart = dataAndName.substr(0,nPosSpace);//Nov 或 11月
            datePart += " ";

            dataAndName = dataAndName.substr(nPosSpace);
            //去空格
            size_t nextStart = dataAndName.find_first_not_of(" ");
            if (nextStart == std::string::npos)
                continue;
            dataAndName = dataAndName.substr(nextStart);
            if (dataAndName.empty())
                continue;

            nPosSpace = dataAndName.find_first_of(" ");
            if (nPosSpace == std::string::npos || nPosSpace == 0)
                continue;
                
            datePart += dataAndName.substr(0,nPosSpace);//11 或 07
            datePart += " ";
            dataAndName = dataAndName.substr(nPosSpace);

            //去空格
            nextStart = dataAndName.find_first_not_of(" ");
            if (nextStart == std::string::npos)
            {
                // 没有时间部分，只有日期
                spaceName = "";
            }
            else
            {
                dataAndName = dataAndName.substr(nextStart);
                if (dataAndName.empty())
                {
                    spaceName = "";
                }
                else
                {
                    nPosSpace = dataAndName.find_first_of(" ");
                    if (nPosSpace != std::string::npos && nPosSpace > 0)
                    {
                        datePart += dataAndName.substr(0,nPosSpace);//18:01
                        spaceName = dataAndName.substr(nPosSpace);
                    }
                    else
                    {
                        // 没有找到空格，可能时间后面直接是文件名
                        // 尝试查找时间格式（包含冒号）
                        size_t colonPos = dataAndName.find_first_of(":");
                        if (colonPos != std::string::npos && colonPos > 0)
                        {
                            // 假设时间格式是 HH:MM，取前5个字符
                            if (dataAndName.length() >= 5)
                            {
                                datePart += dataAndName.substr(0, 5);
                                spaceName = dataAndName.substr(5);
                            }
                            else
                            {
                                datePart += dataAndName;
                                spaceName = "";
                            }
                        }
                        else
                        {
                            // 没有时间格式，整个作为文件名
                            spaceName = dataAndName;
                        }
                    }
                }
            }
        }
        
        item.date = datePart;

        //去空格，提取文件名
        string filename;
        if (!spaceName.empty())
        {
            size_t nameStart = spaceName.find_first_not_of(" ");
            if (nameStart != std::string::npos)
            {
                filename = spaceName.substr(nameStart);
            }
        }
        
        // 去除末尾的换行符和回车符
        while (!filename.empty() && (filename.back() == '\n' || filename.back() == '\r'))
        {
            filename.pop_back();
        }
        
        // 跳过特殊目录名：./ 和 ../
        if (filename == "./" || filename == "../" || filename == "." || filename == "..")
        {
            continue;
        }
        
        // 如果文件名仍然为空，跳过
        if (filename.empty())
        {
            continue;
        }
        
        item.name = filename;
        
        // 如果文件名以 / 结尾，确保类型是目录
        if (!filename.empty() && filename.back() == '/')
        {
            item.type = "directory";
            item.size = "0";
            // 去掉末尾的斜杠（可选，根据需求决定）
            // item.name = filename.substr(0, filename.length() - 1);
        }

        items.push_back(item);
        }
        catch (const std::exception& e)
        {
            ErrorLog("解析SVN输出行时发生异常: %s, line=[%s]", e.what(), line.c_str());
            continue;
        }
        catch (...)
        {
            ErrorLog("解析SVN输出行时发生未知异常, line=[%s]", line.c_str());
            continue;
        }
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