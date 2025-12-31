#include "CTermFirmUpdateNotice.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"

int CTermFirmUpdateNotice::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,outMap,sessionMap;
	string uid;

	//取得所有的请求参数
	pReqData->GetStrMap(inMap);

	//检查输入参数
	CheckParameter(inMap);

	if(!CIdentRelayApi::TermFirmUpdateNofity(inMap,outMap,false))
	{
		ErrorLog("更新固件信息失败,给终端回复成功");
        //不报错
        pResData->SetPara("go_flag","2");
        return 0;
		//throw(CTrsExp(outMap["errorcode"],outMap["errormessage"])) ;
	}

    //去写缓存，将trans_id、完成个数pid_complete_num+1后更新到缓存
    if (!outMap["pack_id"].empty() && !outMap["pid_complete_num"].empty())
    {
         // 获取当前完成个数并+1
        int complete_num = atoi(outMap["pid_complete_num"].c_str()) + 1;
        string value = toString(complete_num);
    
        // 写入/更新缓存
        if (CIdentComm::SetOrUpdateCache(outMap["pack_id"], value, 86400) == 0)
        {
            InfoLog("缓存更新成功: trans_id=[%s], pid_complete_num=[%d]", outMap["pack_id"].c_str(), complete_num);
        }
        else
        {
            ErrorLog("缓存更新失败: pack_id=[%s]", outMap["pack_id"].c_str());
        }
    }

	CStr2Map returnMap;
    CIdentComm::DelMapF(outMap,returnMap);

    //判断版本号是否更新
    if(!inMap["version"].empty())
    {
        int updateFlag = compareVersions(returnMap["version"],inMap["version"]); 
        if(updateFlag > 0)//需要更新
        {
            //去查包的ID

            pResData->SetPara("pack_id",returnMap["pack_id"]);
            pResData->SetPara("go_flag","0");
            pResData->SetPara("is_update","1");//需要更新
            pResData->SetPara("pre_version",returnMap["version"]);
            return 0;
        }
        else
            pResData->SetPara("is_update","0");
    }
  
    pResData->SetPara("go_flag",returnMap["go_flag"]);
    pResData->SetPara("go_name",returnMap["go_name"]);
    pResData->SetPara("go_passwd",returnMap["go_passwd"]);
    pResData->SetPara("channel",returnMap["channel"]);

	return 0;
}

// 输入判断
void CTermFirmUpdateNotice::CheckParameter( CStr2Map& inMap)
{
	if(inMap["trans_id"].empty())
    {
        ErrorLog("关键字段不能为空-trans_id");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"trans_id");
    }
    if(inMap["result"].empty())
    {
        ErrorLog("关键字段不能为空-result");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段result为空");
    }
  
}

int CTermFirmUpdateNotice::compareSpecialParts(const std::string &v1, const std::string &v2) 
{
    // 格式为 V6DCR000 + 时间 + 3 位编号
    std::string version1 = v1.substr(8, 8); // 提取时间部分
    std::string version2 = v2.substr(8, 8); // 提取时间部分

    // 比较版本号部分
    int result = version1.compare(version2);
    if (result != 0) return -1; // 如果版本号不同，直接报错，后面不需要比较

    // 提取时间部分
    std::string time1 = v1.substr(8, 8); // 提取时间部分
    std::string time2 = v2.substr(8, 8); // 提取时间部分

    // 比较时间部分
    int timeResult = time1.compare(time2);
    if (timeResult != 0) {
        return (timeResult > 0) ? 1 : -1; // 返回 1 或 -1
    }

    // 比较后面的 3 位编号
    std::string id1 = v1.substr(16, 3);
    std::string id2 = v2.substr(16, 3);
    // 比较编号部分
    int idResult = id1.compare(id2);
    if (idResult != 0) {
        return (idResult > 0) ? 1 : -1; // 返回 1 或 -1 取决于编号的比较结果
    }

    return 0;
}


//sysVer是系统上的版本号，appVer是上送的版本号
//结果 0表示相同 1表示需要更新 -1报错
int CTermFirmUpdateNotice::compareVersions(const std::string &sysVer, const std::string &appVer)
{
    //判断固件版本是否小于等于服务器的，如果是，就返回更新，不是就报错
    std::vector<std::string> v1Parts = split(sysVer, '|');
    std::vector<std::string> v2Parts = split(appVer, '|');
    if(v1Parts.size() != v2Parts.size())
    {
        return -1;
    }
    for (size_t i = 0; i < v1Parts.size(); ++i)
    {
        string part1 = v1Parts[i];
        string part2 = v2Parts[i];

        if(i ==0)//比较软件部分
        {
            InfoLog("软件部分比较[%s]跟[%s]",part1.c_str(),part2.c_str());
            int result = compareVersionParts(part1, part2);
            InfoLog("软件部分比较结果[%d]",result);
            if(result != 0)
                return result;
        }
        else
        {
            int result = compareSpecialParts(part1, part2);
            if(result != 0)
                return result; // 返回比较结果
        }
    }
    return 0; // 两个版本完全相同
}

static string cleanVersion(const string &version) 
{
    // 找到下划线的位置
    size_t underscore_pos = version.find('_');
    // 如果找到下划线，保留下划线前的部分；否则保留整个字符串
    return (underscore_pos != string::npos) ? version.substr(0, underscore_pos) : version;
}

int CTermFirmUpdateNotice::compareVersionParts(const std::string &v1, const std::string &v2)
{
    // 去掉版本号中的非数字字符
    string v1_clean = cleanVersion(v1);
    string v2_clean = cleanVersion(v2);

    v1_clean.erase(remove_if(v1_clean.begin(), v1_clean.end(), [](unsigned char c) { return !isdigit(c) && c != '.'; }), v1_clean.end());
    v2_clean.erase(remove_if(v2_clean.begin(), v2_clean.end(), [](unsigned char c) { return !isdigit(c) && c != '.'; }), v2_clean.end());

    std::vector<std::string> parts1 = split(v1_clean, '.');
    std::vector<std::string> parts2 = split(v2_clean, '.');

    if(parts1.size() != parts2.size())//长度不对，直接报错，说明格式不对
        return -1;


    for (size_t i = 0; i < parts1.size(); ++i) {
        int num1 = (i < parts1.size()) ? std::stoi(parts1[i]) : 0;
        int num2 = (i < parts2.size()) ? std::stoi(parts2[i]) : 0;
        InfoLog("比较[%d]跟[%d]",num1,num2);
        if (num1 < num2) return -1; // v1 < v2
        if (num1 > num2) return 1;  // v1 > v2
    }
    return 0; // v1 == v2
}

vector<string> CTermFirmUpdateNotice::split(const std::string &s, char delimiter)
{
    std::vector<std::string> tokens;
    std::stringstream ss(s);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}