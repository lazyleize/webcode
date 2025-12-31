#ifndef CJIFENAPPCOMM_H_
#define CJIFENAPPCOMM_H_
#include "CCgi.h"
#include "CIdentComm.h"
#include "CIdentPub.h"
#include <iomanip>


/**
 * 
 */
//#define CITIC_SUC_RET "AAAAAAA"
/*
 *
 */

class CAppInitEnv
{
public:
	string conf_path;
	string svr_ip;
	string app_name;
};

#define CONF_PATH_KEY "app_conf"
#define SVRIP_KEY "app_svrip"

/*
 *
 */
class CIdentAppComm: private CCgi
{
private:
	/*
	 * 服务器IP地址
	 */
	string svr_ip;
	/*
	 * 应用名称，用作标识用
	 */
	string app_name;
	//本次启动的配置文件路径
	string conf_path;
	string sMsgNo;//MSGNO
	string sUin;//sUin
	/*
	 * 初始化用函数
	 */
	CInitGlobalData initAppData;
	/*
	 * 把Map变成一个可打印的字符串
	 */
	void MapToStrPrintFilter(CStr2Map& inMap, string &formData, string tid,
			string sEleSep = "&", string sNvSep = "=");
	string ReplaceStr(const string& str);

public:
	CIdentAppComm();
	virtual ~CIdentAppComm();
	/*
	 * 日志监控系统的标识
	 */
	const static string SYSTEM_NAME;
	const static string SUBSYSTEM_NAME;
	//系统模块标识，三位数字，用作错误码的前三位，MSGNO的前三位。
	const static int XD_MODULE_NO = 529;
	inline string GetAppName() const
	{
		return app_name;
	}
	inline string GetSvrIp() const
	{
		return svr_ip;
	}
	inline string GetConfPath() const
	{
		return conf_path;
	}

	void LoadConfigure();
	/*
	 * 做初始化，传入配置文件，以及IP地址
	 */
	void InitEnv(const CAppInitEnv & initData);
	char* MakeMsgNo();
	static void GetCiticHead(const string & sTransName, CStr2Map& bankInmap);
	/*
	 *检查从命令行参数中输入的日期的格式，正确格式:0000-00-00
	 *param   inputDate  待检查的日期
	 *return  0 检查合规   1 检查不合规
	 */
	int CheckInputDateFormat(const string& inputDate);
	/**
	 * 新增函数，用来刷新MSG_NO。
	 * 避免应用在长时间的多笔处理中，使用同一个MSG_NO。
	 * 在CGI中，MSGNO的生命周期等同于一次完整的业务处理，这里借鉴该设置。
	 * 在每一个业务类开始业务前，调用该函数来更新MSG_NO
	 */
	void RefreshMsgNo(const string & uin);
	inline void SetMsgNo(const string & sMsgNo)
	{
		this->sMsgNo = sMsgNo;
	}
	inline void SetUin(const string & sUin)
	{
		this->sUin = sUin;
	}
	inline const string & GetMsgNo(){
		return this->sMsgNo;
	}
	inline const string & GetUin(){
		return this->sUin;
	}
	/**
	 * 检查当前进程已经执行的时间，如果已经执行超过了minutes分钟，自动正常退出。
	 */
	static void CheckRunTime(const CAppInitEnv & env,const int minutes=30);
};

//定义几个写日志宏，将__FILE__, __LINE__默认写入日志中
#define XdDebugLog(format, argv...) do{ g_RuntimeGather.SaveLog(DEBUG, (string("[%s|%d|%s|%s]")+string(format)).c_str(), __FILE__, __LINE__,NULL==pIdentAppComm?"":pIdentAppComm->GetMsgNo().c_str(),NULL==pIdentAppComm?"":pIdentAppComm->GetUin().c_str(), ##argv); }while(0)
#define XdInfoLog(format, argv...) do{ g_RuntimeGather.SaveLog(INFO, (string("[%s|%d|%s|%s]")+string(format)).c_str(), __FILE__, __LINE__,NULL==pIdentAppComm?"":pIdentAppComm->GetMsgNo().c_str(),NULL==pIdentAppComm?"":pIdentAppComm->GetUin().c_str(), ##argv); }while(0)
#define XdWarnLog(format, argv...) do{ g_RuntimeGather.SaveLog(WARN, (string("[%s|%d|%s|%s]")+string(format)).c_str(), __FILE__, __LINE__,NULL==pIdentAppComm?"":pIdentAppComm->GetMsgNo().c_str(),NULL==pIdentAppComm?"":pIdentAppComm->GetUin().c_str(), ##argv); }while(0)
#define XdErrorLog(format, argv...) do{ g_RuntimeGather.SaveLog(ERROR, (string("[%s|%d|%s|%s]")+string(format)).c_str(), __FILE__, __LINE__,NULL==pIdentAppComm?"":pIdentAppComm->GetMsgNo().c_str(),NULL==pIdentAppComm?"":pIdentAppComm->GetUin().c_str(), ##argv); }while(0)

#endif /* CAPPDEMO_H_ */
