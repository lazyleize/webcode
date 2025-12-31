#include "CIdentAppComm.h"
#include "CTools.h"
#include <arpa/inet.h>
CIdentAppComm::CIdentAppComm()
{
	//每次启动时，把runtime的启动时间归零
	g_RuntimeGather.RuntimeBegin();
}
CIdentAppComm::~CIdentAppComm()
{
}
;

void CIdentAppComm::InitEnv(const CAppInitEnv & initData)
{
	/*
	 * 按照CGI框架的常规方式，设置环境变量。
	 * 后面将使用这两个环境变量来进行初始化，读取配置文件。
	 */
	setenv(CONF_PATH_KEY, initData.conf_path.c_str(), 1);
	setenv(SVRIP_KEY, initData.svr_ip.c_str(), 1);
	this->app_name = initData.app_name;
	this->svr_ip = initData.svr_ip;
	this->conf_path = initData.conf_path;
	//读取配置文件
	this->LoadConfigure();
	/*
	 * 初始化LOG的全局配置信息
	 */
	g_RuntimeGather.SetMsgNo(this->MakeMsgNo());
	g_RuntimeGather.SetSessionKey("");
	g_RuntimeGather.SetCgiName(initData.app_name);
	g_RuntimeGather.SetClientIp("");
	g_RuntimeGather.SetCookie("");

	this->SetMsgNo(g_RuntimeGather.GetMsgNo());
}
char* CIdentAppComm::MakeMsgNo()
{
	//生成全局唯一的msgno
	static char cMsgNo[32] =
	{ 0 };
	char* pLocalIp = getenv(SVRIP_KEY);
	if (pLocalIp == NULL)
	{
		ThrowFrameWorkErr("System Error 02 ,Please Contact WebMaster");
	}

	in_addr_t lIp = inet_addr(pLocalIp);
	struct timeval tv1;
	gettimeofday(&tv1,NULL);
	pid_t iPid = getpid();
	snprintf(cMsgNo, sizeof(cMsgNo) - 1, "%3d%08x%010ld%04ld%05u", XD_MODULE_NO, lIp,
			tv1.tv_sec,tv1.tv_usec, iPid);
	return cMsgNo;
}

void CIdentAppComm::LoadConfigure()
{

	const char* confname = getenv(CONF_PATH_KEY);
	if (confname == NULL)
	{
		//致命错误,直接退出执行
		ThrowFrameWorkErr("System Error 01 ,Please Contact WebMaster");
	}
	initAppData.LoadCfg(string(confname));

	//读取服务配置
	string relaypath = g_allVar.GetValue("serverconf");
	if (relaypath != "")
	{
		initAppData.LoadCfg(relaypath);
	}
	//读取服务配置
	string confpath = g_allVar.GetSubModule("ident_ident");
	if (confpath != "")
	{
		initAppData.LoadCfg(confpath);
	}
	//读取服务配置
	string appconfpath = g_allVar.GetSubModule("ident_ident");
	if (appconfpath != "")
	{
		initAppData.LoadCfg(appconfpath);
	}
}

CTrans * CCgi::MakeTransObj()
{
	return NULL;
}

/*
 *检查从命令行参数中输入的日期的格式，正确格式:YYYY-MM-DD
 *param   inputDate  待检查的日期
 *return  0 检查合规   1 检查不合规
 */
int CIdentAppComm::CheckInputDateFormat(const string& inputDate)
{
	if (inputDate.empty())
	{
		return 1;
	}
	std::string::const_iterator iter = inputDate.begin();
	std::string::size_type count = 0;
	for (; iter != inputDate.end(); iter++)
	{
		if (*iter == '-')
			count++;
	}
	if (count != 2 || inputDate.length() != 10 || inputDate[4] != '-'
			|| inputDate[7] != '-')
	{
		return 1;
	}
	return 0;
}
/**
 * leungma 20180717110151
 * 新增函数，用来刷新MSG_NO。
 * 避免应用在长时间的多笔处理中，使用同一个MSG_NO。
 * 在CGI中，MSGNO的生命周期等同于一次完整的业务处理，这里借鉴该设置。
 * 在每一个业务类开始业务前，调用该函数来更新MSG_NO
 */
void CIdentAppComm::RefreshMsgNo(const string & uin)
{
	this->SetMsgNo(this->MakeMsgNo());
	this->SetUin(uin);
}

void CIdentAppComm::CheckRunTime(const CAppInitEnv & env,const int minutes){
	long lRunTime=g_RuntimeGather.GetRuntime();
	long lTargetTime=minutes*60*1000000L;
	if(lTargetTime<lRunTime){
		InfoLog("%s Has Run %ld us,exit.Wait auto pull up.",env.app_name.c_str(),lRunTime);
		exit(0);
	}else{
		InfoLog("%s Has Run %ld us,continue.",env.app_name.c_str(),lRunTime);
	}
}

//报文头赋值
void CIdentAppComm::GetCiticHead(const string & sTransName, CStr2Map& bankInmap)
{
    //公共报文头赋值
    bankInmap["stdmsgtype"] = g_mTransactions[sTransName].m_mVars["stdmsgtype"];
    bankInmap["std400aqid"] = g_mTransactions[sTransName].m_mVars["std400aqid"];
    bankInmap["stdmercno"] = g_mTransactions[sTransName].m_mVars["stdmercno"];
    bankInmap["std400tsys"] = g_mTransactions[sTransName].m_mVars["std400tsys"];

    bankInmap["std400trcd"] = g_mTransactions[sTransName].m_mVars["std400trcd"];
    bankInmap["stdprocode"] = g_mTransactions[sTransName].m_mVars["stdprocode"];
    bankInmap["stdlocdate"] = CIdentPub::GetFormatDateNow();
    bankInmap["std400trdt"] = CIdentPub::GetFormatDateNow();
    bankInmap["stdloctime"] = CIdentPub::GetCurrentTime();
}

