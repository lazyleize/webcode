#include "CGetFirmVersion.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"
#include "xml/unicode.h"

#include <base/datetime.hpp>

using namespace aps;
int CGetFirmVersion::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,tmpMap,outMap,sessionMap;
	string uid;

	// 取得请求参数
    pReqData->GetStrMap(inMap);
	
	//检查输入参数
	CheckParameter(inMap);

    //查询顺序：PID>订单号>物料号>机型
    if(!inMap["pid"].empty())
    {
        inMap["order_id"]="";
        inMap["term_type"]="";
        inMap["mater_num"]="";
    }
    else if(!inMap["order_id"].empty())
    {
        inMap["term_type"]="";
        inMap["mater_num"]="";
    }
    else if(!inMap["mater_num"].empty())
    {
        inMap["term_type"]="";
    }

    CIdentRelayApi::QueryFirmRulesPath(inMap,outMap,true);
    if(outMap["ret_num"]=="0")//找不到下载规则记录
    {
        ErrorLog("找不到下载规则");
        CIdentPub::SendAlarm2("找不到下载规则,上送的PID超出范围或者没有web建单");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"找不到下载规则,上送的PID超出范围或者没有web建单");
    }
    
	CStr2Map returnMap;
    CIdentComm::DelMapF(outMap,returnMap);

    int updateFlag = compareVersions(returnMap["version"],inMap["version"]); 
    /*if(updateFlag < 0)//报错，说明上送的版本号不对
    {
        ErrorLog("APP上送或系统录入的版本号不对APP[%s],SYS[%s]",inMap["version"].c_str(),returnMap["version"].c_str());
        CIdentPub::SendAlarm2("APP上送或系统录入的版本号不对APP[%s],SYS[%s]",inMap["version"].c_str(),returnMap["version"].c_str());
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"APP上送或系统录入的版本号不对");
    }*/
    InfoLog("比较结果[%d]",updateFlag);
    if(updateFlag <= 0)//跟服务器版本相同或者比服务器版本大，不需要更新
    {
        pResData->SetPara("pre_version",returnMap["version"]);
        pResData->SetPara("update_flag","0");
        return 0;
    }
    pResData->SetPara("pre_version",returnMap["version"]);
    pResData->SetPara("update_flag","1");
    pResData->SetPara("pack_id",returnMap["pack_id"]);
    pResData->SetPara("count",returnMap["down_count"]);
    pResData->SetPara("pack_size",returnMap["pack_size"]);
    
	return 0;
}

// 输入判断
void CGetFirmVersion::CheckParameter( CStr2Map& inMap)
{
	if(inMap["order_id"].empty()&&inMap["term_type"].empty()&&inMap["mater_num"].empty()&&inMap["pid"].empty())
    {
        ErrorLog("必须填写一个元素");
		CIdentPub::SendAlarm2("APP获取固件版本信息必须填写一个元素[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"必须填写一个元素");
    }
    if(inMap["version"].empty())
    {
        ErrorLog("version为空");
		CIdentPub::SendAlarm2("关键字段版本号为空-version[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段版本号为空-version");
    }
}

//sysVer是系统上的版本号，appVer是上送的版本号
//结果 0表示相同 1表示需要更新 -1报错
int CGetFirmVersion::compareVersions(const std::string &sysVer, const std::string &appVer)
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

int CGetFirmVersion::compareSpecialParts(const std::string &v1, const std::string &v2) 
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

static string cleanVersion(const string &version) 
{
    // 找到下划线的位置
    size_t underscore_pos = version.find('_');
    // 如果找到下划线，保留下划线前的部分；否则保留整个字符串
    return (underscore_pos != string::npos) ? version.substr(0, underscore_pos) : version;
}

int CGetFirmVersion::compareVersionParts(const std::string &v1, const std::string &v2)
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



vector<string> CGetFirmVersion::split(const std::string &s, char delimiter)
{
    std::vector<std::string> tokens;
    std::stringstream ss(s);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

bool CGetFirmVersion::mergeStrings(std::string& strOnlineGo, const std::string& strTermUpdate,vector<std::string>& onlyInOnline) 
{
    // 清空返回的vector
    onlyInOnline.clear();
    
    // 如果strOnlineGo为空，直接赋值
    if (strOnlineGo.empty()) {
        strOnlineGo = strTermUpdate;
        return false;
    }
    
    // 分割strOnlineGo
    std::set<std::string> onlineSet;
    std::vector<std::string> onlineList;
    std::stringstream ss1(strOnlineGo);
    std::string item;
    
    while (std::getline(ss1, item, ',')) {
        item.erase(0, item.find_first_not_of(" \t"));
        item.erase(item.find_last_not_of(" \t") + 1);
        if (!item.empty()) {
            onlineSet.insert(item);
            onlineList.push_back(item);
        }
    }
    
    // 分割strTermUpdate
    std::set<std::string> termSet;
    std::vector<std::string> termList;
    std::stringstream ss2(strTermUpdate);
    while (std::getline(ss2, item, ',')) {
        item.erase(0, item.find_first_not_of(" \t"));
        item.erase(item.find_last_not_of(" \t") + 1);
        if (!item.empty()) {
            termSet.insert(item);
            termList.push_back(item);
        }
    }
    
    // 找出strOnlineGo中有但strTermUpdate中没有的项目
    for (const auto& onlineItem : onlineList) {
        if (termSet.find(onlineItem) == termSet.end()) {
            onlyInOnline.push_back(onlineItem);
        }
    }
    
    // 检查是否有新项目需要添加
    bool hasNewItems = false;
    for (const auto& termItem : termList) {
        if (onlineSet.find(termItem) == onlineSet.end()) {
            hasNewItems = true;
            onlineList.push_back(termItem);
        }
    }
    
    // 如果没有新项目，返回false
    if (!hasNewItems) {
        return false;
    }
    
    // 重新构建strOnlineGo
    strOnlineGo.clear();
    for (size_t i = 0; i < onlineList.size(); ++i) {
        if (i > 0) {
            strOnlineGo += ",";
        }
        strOnlineGo += onlineList[i];
    }
    
    return true;
}