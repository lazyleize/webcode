#include "CFirmDownloadSet.h"
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

int CFirmDownloadSet::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,outMap,sessionMap;
    vector<CStr2Map> vectmapArray;

    //取得所有的请求参数
	string strPostdata=pReqData->GetPostData();
    //ErrorLog("strPostdata=%s",strPostdata.c_str());
    CIdentPub::parsePubReqJsonList1(strPostdata,inMap,vectmapArray);
    if(vectmapArray.size() != atoi(inMap["total"].c_str()))
    {
        ErrorLog("错误vectmapArray[%d] total=%d" ,vectmapArray.size(),atoi(inMap["total"].c_str()));
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"total错误");
    }

    //调试屏蔽
	this->CheckLogin(sessionMap);

    inMap["usertype"] = sessionMap["usertype"];
    int usertype = atoi(sessionMap["usertype"].c_str());
    if(usertype != 2  && usertype != 1)
    {
        ErrorLog("没有权限: usertype=[%s]" ,inMap["usertype"].c_str());
        throw CTrsExp(ERR_NO_LOWER_LEVER,"没有权限");
    }
    
	CheckParameter(inMap);

    //从配置拿SVN用户名，密码
    char svnName[125]={0};
    char svnPasswd[125]={0};
    memcpy(svnName,g_mTransactions[GetTid()].m_mVars["SvnName"].c_str(),g_mTransactions[GetTid()].m_mVars["SvnName"].length());
    memcpy(svnPasswd,g_mTransactions[GetTid()].m_mVars["SvnPasswd"].c_str(),g_mTransactions[GetTid()].m_mVars["SvnPasswd"].length());
    string strSvnName=svnName;
    string strSvnPasswd=svnPasswd;

    int nCount = vectmapArray.size();
    ErrorLog("nCount=%d",nCount);
    
    for(int i = 0;i < nCount;i++)
    {
        //utf-8转GBK执行  一个一个验证SVN地址的有效性
        char buff[1024] = {0} ;
        int  iDestLen = sizeof(buff)-1;
        string svnUrl = vectmapArray[i]["svn_path"]; // 获取当前 SVN 地址
        //增加&&判断
        if(CheckUrl(svnUrl))
        {
            ErrorLog("SVN地址含有&&,不合法地址");
            throw CTrsExp(ERR_SIGNATURE_INCORRECT,"SVN地址含有&&,不合法地址");
        }

        svnUrl = CIdentPub::escape_svn_path(svnUrl);

        //检查SVN地址的有效性
        if(!isValidSVNUrl(svnUrl,strSvnName,strSvnPasswd))
        {
            ErrorLog("无效的SVN地址,[%s]",svnUrl.c_str());
            throw CTrsExp(ERR_SIGNATURE_INCORRECT,"无效的SVN地址");
        }
        
        inMap["svn_path"] += svnUrl;
        inMap["svn_path"] += "&&";//写文档
    }
    
    //去掉一个&&
    inMap["svn_path"].erase(inMap["svn_path"].length() - 2);
    inMap["total"]=to_string(nCount);

	if(!CIdentRelayApi::FirmDownloadRuleSet(inMap,outMap,true))
    {
        ErrorLog("设置下载规则,ads报错");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"设置下载规则错误");
    }

    InfoLog(": id=[%s]" ,outMap["id"].c_str());
    //触发程序去SVN拉取固件包
    char cmd[256];
    memset(cmd,0x00,sizeof(cmd));
    snprintf(cmd,sizeof(cmd),"/home/httpd/web/web_pay5/tools/start_app %s > /dev/null 2>&1 &",outMap["id"].c_str());
    InfoLog(": cmd=[%s]" ,cmd);
    system(cmd);

	return 0;
}

// 输入判断
void CFirmDownloadSet::CheckParameter( CStr2Map& inMap)
{
    if(inMap["term_type"].empty())
    {
        ErrorLog("机型为空");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"机型为空");
    }
    
    if(inMap["version"].empty())
    {
        ErrorLog("version为空");
		CIdentPub::SendAlarm2("关键字段版本号为空-version[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段版本号为空-version");
    }
    if(inMap["time_spent"].empty())
    {
        ErrorLog("老化时长-time_spent为空");
		CIdentPub::SendAlarm2("老化时长-time_spent为空[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"老化时长-time_spent为空");
    }
    if(inMap["ap_num"].empty())
    {
        inMap["ap_num"]="20";
    }
    if(inMap["go_num"].empty())
    {
        inMap["go_num"]="18";
    }
    if(!inMap["pid_start"].empty() && inMap["pid_end"].empty())
    {
        string suffix = inMap["pid_start"].substr(inMap["pid_start"].length() - 5);
        int suffix_int = std::stoi(suffix);
        int new_suffix_int = suffix_int + atoi(inMap["count"].c_str());
        string new_suffix = std::to_string(new_suffix_int);
        while (new_suffix.length() < 5) 
        {
            new_suffix = "0" + new_suffix; // 在前面补零
        }
        new_suffix = new_suffix.substr(new_suffix.length() - 5); // 取最后 5 位
        inMap["pid_end"] = inMap["pid_start"].substr(0, inMap["pid_start"].length() - 5) + new_suffix;
    }
}

bool CFirmDownloadSet::isValidSVNUrl(const std::string& svn_path, const std::string& username, const std::string& password)
{
    if(svn_path.length() ==0)
        return false;
    std::string command = "svn info --username " + username + " --password " + password + " " + svn_path + " --trust-server-cert --non-interactive > /dev/null 2>&1";
    int result = std::system(command.c_str());

    InfoLog(": command=[%s]" ,command.c_str());
    if(result == 0)
        return true;
    else
    {
        ErrorLog("SVN指令运行错误");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"系统错误,没有SVN命令");
    }
    return false;
}

bool CFirmDownloadSet::CheckUrl(const std::string& svn_path)
{
    // 查找 '&&' 是否存在于 svn_path 中
    if (svn_path.find("&&") != std::string::npos) {
        return true; 
    }
    return false; 
}