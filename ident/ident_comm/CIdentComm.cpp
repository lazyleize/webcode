#include "CIdentComm.h"
#include "uuid/lnxuuid.h"
#include "tools/base64.h"
#include "tlib/tlib_all.h"
#include "cgicomm/adsmem_rules.h"
#include "libmemcached/memcached.h"
#include "tools/tinyxml.h"

/*
 * CGI入口
 */
int CIdentComm::Commit()
{
    //提交业务逻辑
    int iRet = IdentCommit(m_pReqData, m_pResData);
    return iRet;
}

/* 将文件保存到本地 */
string CIdentComm::saveUploadFile(const cgicc::FormFile& formFile,bool isTransform)
{
    /* 全目录路径 */
    char fileFullName[1024]={0};
    //文件名
    char szFileName[1024]={0};
  
    /* 取文件名后缀 */
    string::size_type iPos = formFile.getFilename().find_last_of('.');
  
    if(isTransform)
    {
        /*
         *给每个文件一个唯一标示号,以便存入数据库中
         *并能从前端根据此标示号，下载文件
         */
        CUuid uid(1);
        //获取文件名
        snprintf(szFileName, sizeof(szFileName), "%s%s",uid.GetString(0).c_str(),formFile.getFilename().substr(iPos).c_str());
    }
    else
    {
        string fileName = "";
        iPos = formFile.getFilename().find_last_of('/'); 
  
        if(iPos != string::npos)
        {
            fileName = fileName.substr(iPos+1);
        }
        else
        {
            fileName = formFile.getFilename();
        }
        //获取文件名
        snprintf(szFileName, sizeof(szFileName), "%s",fileName.c_str());
    }
    
    //获取公共路径
    string commPath =CIdentPub::GetTransConfigNotEmpty(this->GetTid(),"upload_path");
    //得到上传文件的全路径名
    snprintf(fileFullName, sizeof(fileFullName), "%s%s",commPath.c_str(),szFileName);
    
    //保存文件到本地
    this->saveUploadFile(formFile,string(fileFullName));
  
    return  szFileName;
}

void CIdentComm::saveUploadFile(const cgicc::FormFile& formFile,const string& fileName)
{
    ofstream fout;
    /*
     * 创建多级目录，并以二进制方式打开写文件
     */
    if(CIdentPub::OpenFile(fileName ,fout, ios::binary))
    {
        ErrorLog((string("创建文件目录失败:")+fileName+string(strerror(errno))).c_str());
        throw(CTrsExp(ERR_MKDIR_UPLOAD ,"创建文件目录失败，请通知管理员"));
    }
    DebugLog("[%s:%d],创建文件目录:%s",__FILE__,__LINE__,fileName.c_str());
    formFile.writeToStream(fout);
    fout.close();
}

string CIdentComm::saveUploadFile(const cgicc::FormFile& formFile)
{
    /* 全目录路径 */
    char fileFullName[1024]={0};
    //文件名
    char szFileName[1024]={0};
  
    /* 取文件名后缀 */
    string::size_type iPos =
    formFile.getFilename().find_last_of('.');
  
    /*
     *给每个文件一个唯一标示号,以便存入数据库中
     *并能从前端根据此标示号，下载文件
     */
    CUuid uid(1);
    //获取文件名
    snprintf(szFileName, sizeof(szFileName), "%s%s",uid.GetString(0).c_str(),formFile.getFilename().substr(iPos).c_str());
  
    //获取公共路径
    string commPath =CIdentPub::GetTransConfigNotEmpty(this->GetTid(),"upload_path");
  
    //得到上传文件的全路径名
    snprintf(fileFullName, sizeof(fileFullName), "%s/%s",commPath.c_str(),szFileName);
    
    //保存文件到本地
    this->saveUploadFile(formFile,string(fileFullName));
    return  szFileName;
}

string CIdentComm::saveUploadFile(const cgicc::FormFile& formFile,string& OutFullPath)
{
    /* 全目录路径 */
    char fileFullName[1024]={0};
    //文件名
    char szFileName[1024]={0};
  
    /* 取文件名后缀 */
    string::size_type iPos =
    formFile.getFilename().find_last_of('.');
  
    /*
     *给每个文件一个唯一标示号,以便存入数据库中
     *并能从前端根据此标示号，下载文件
     */
    CUuid uid(1);
    //获取文件名
    snprintf(szFileName, sizeof(szFileName), "%s%s",uid.GetString(0).c_str(),formFile.getFilename().substr(iPos).c_str());
  
    //获取公共路径
    string commPath =CIdentPub::GetTransConfigNotEmpty(this->GetTid(),"upload_path");
  
    //得到上传文件的全路径名
    snprintf(fileFullName, sizeof(fileFullName), "%s%s",commPath.c_str(),szFileName);

    OutFullPath = fileFullName;
    
    //保存文件到本地
    this->saveUploadFile(formFile,string(fileFullName));
    return  szFileName;
}

int CIdentComm::UploadFile(CReqData *pReqData, CStr2Map &fileName,const unsigned int fileCount)
{
    int uploadCount(0);
  
    //得到上传文件的列表 
    vector<cgicc::FormFile> vcFilelist;
    m_pReqData->GetUploadFileList(vcFilelist);
  
    //判断文件数量 
    if (fileCount > vcFilelist.size())
    {
        ErrorLog("上传文件数量不对 上传文件数量[%d] 要求文件数量大于[%d]", vcFilelist.size(), fileCount);
        throw CTrsExp(ERR_UPFILE_COUNT, "请至少上传一个文件");
    }

    //遍历 
    vector<cgicc::FormFile>::const_iterator it;
    for (it = vcFilelist.begin(); it != vcFilelist.end(); ++it)
    {
        //检查文件 
        this->CheckFile((*it));
    
        //存放文件到本地并保存文件名，便于后续取文件 
        fileName[it->getName()] =this->saveUploadFile(*it);
        uploadCount++;
    }
    return uploadCount;
}

int CIdentComm::UploadFile(CReqData *pReqData, CStr2Map &fileName,const unsigned int fileCount, bool isTransform)
{
    int uploadCount(0);
  
    //得到上传文件的列表
    vector<cgicc::FormFile> vcFilelist;
    m_pReqData->GetUploadFileList(vcFilelist);
  
    //判断文件数量
    if ( vcFilelist.size() < fileCount)
    {
        ErrorLog("上传文件数量不对 上传文件数量[%d] 要求文件数量[%d]", vcFilelist.size(), fileCount);
        throw CTrsExp(ERR_UPFILE_COUNT, "上传文件数量不对");
    }

    //遍历
    vector<cgicc::FormFile>::const_iterator it;
    for (it = vcFilelist.begin(); it != vcFilelist.end(); ++it)
    {
        //检查文件
        this->CheckFile((*it));
    
        //存放文件到本地并保存文件名，便于后续取文件
        fileName[it->getName()] =this->saveUploadFile(*it,isTransform);
        DebugLog("[%s:%d] %s:[%s]", __FILE__,__LINE__,it->getName().c_str(),fileName[it->getName()].c_str());
        uploadCount++;
    }
  
    return uploadCount;
}

void CIdentComm::CheckFile(const cgicc::FormFile formFile)
{
    DebugLog("检查文件名称");
    /*
     *检查文件大小
     */
    string sUploadMaxFileSize(CIdentPub::GetTransConfigNotEmpty(this->GetTid(),"max_file_length"));
    unsigned int maxUploadFileSize = atoi(sUploadMaxFileSize.c_str());
  
    if (formFile.getDataLength() <= 0 || formFile.getDataLength() > maxUploadFileSize)
    {
        ErrorLog("|上传的文件大小存在异常! ClientIp: %s, file size:%d, limit:%d",this->GetClientIp().c_str(),formFile.getDataLength(),maxUploadFileSize);
        throw(CTrsExp(ERR_OVER_MAXUPLOADESIZE, "上传的文件大小存在异常!"));
    }
    /*
     * 检查文件名后缀
     */
    size_t iPos = formFile.getFilename().find_last_of('.');
    if (string::npos == iPos)
    {
        ErrorLog("上传的文件后缀名异常! ClientIp: %s, file size:%d, limit:%d",this->GetClientIp().c_str(),formFile.getDataLength(),maxUploadFileSize);
        throw(CTrsExp(ERR_UPLOAD_FILENAME, "上传的文件后缀名异常!"));
    }
    //得到文件后缀名
    string sFileFmtName(formFile.getFilename().substr(iPos + 1));
    /*
     * 将文件后缀名变为小写，再进行匹配
     */
    sFileFmtName = Tools::lower(sFileFmtName);
  
    //从配置文件读取文件后缀名的配置
    string file_type =CIdentPub::GetTransConfigNotEmpty(this->GetTid(), "file_type");
  
    //做正则表达式匹配
    string strErrmsg;
    if (0 != Tools::regex_match(sFileFmtName, file_type,strErrmsg))
    {
        ErrorLog("上传的文件后缀名异常! ClientIp: %s, file size:%d, limit:%d",this->GetClientIp().c_str(),formFile.getDataLength(),maxUploadFileSize);
        throw(CTrsExp(ERR_UPLOAD_FILENAME, "上传的文件后缀名异常!"));
    }
}

string CIdentComm::GetVerifyCode()
{
    char tmpbuf[8]={0};
    srand((unsigned)time( NULL ));
    sprintf(tmpbuf,"%06d",rand()%1000000);
    return tmpbuf;
}

void CIdentComm::CheckVerifyCode(const string & verify_code)
{
    //获取cookie中的验证码
    string cookie_code = this->m_pReqData->GetCookie(VERIFYSESSION);
    DebugLog("cookie_code:[%s]",cookie_code.c_str());
    if(cookie_code.empty())
    {
        ErrorLog("[%s],[%d]行 验证码过期 uin:[%s]",__FILE__,__LINE__, verify_code.c_str());
        throw CTrsExp(ERR_VERIFYCODE_TIMEOUT,"验证码过期，请从新获取验证码");
    }
    
    //验证码解密
    char verify_decode[32];
    memset(verify_decode, 0x00, sizeof(verify_decode));
    int length;
    //base64解密
    int result = Base64_Decode(cookie_code.c_str(), cookie_code.length(),
    (unsigned char*)verify_decode,sizeof(verify_decode),&length);
    if(result != 0)
    {
        ErrorLog("[%s],[%d]行 验证码解密失败 cookie_code:[%s]",__FILE__,__LINE__,cookie_code.c_str());
        throw CTrsExp(ERR_DECODE_BASE,"系统繁忙,请稍后再试");
    }
    
    //删除cookie记录
    //set_cookie(VERIFYSESSION, "", "Thu, 01 Jan 1970 00:00:00 GMT", "/", "icandai.com",  0);
    
    //校验验证码是否正确
    if(string(verify_decode) != verify_code)
    {
        ErrorLog("[%s],[%d]行 验证码错误 uin:[%s]",__FILE__,__LINE__,verify_code.c_str());
        throw CTrsExp(ERR_VERIFY_CODE,"验证码错误");
    }
}

void CIdentComm::DelMapF(CStr2Map& dataMap,CStr2Map& outMap)
{
    CStr2Map::const_iterator it = dataMap.begin();
    while(it != dataMap.end())
    {
        string::const_iterator s = it->first.begin();
        if(*s == 'F')
        {
            outMap[it->first.substr(1,it->first.length() - 1)] = it->second;
            ++it;
            continue;
        }
        outMap[it->first] = it->second;
        ++it;
    }
}

void CIdentComm::WriteSessionCookie(CStr2Map& inMap,string& sOutErrMsg,string& strSessionKey,const char *domain)
{
    LoginSessionData sessionData;
    sessionData.uType = Tools::StrToLong(inMap["user_type"].c_str());
      
    memcpy(sessionData.szTrueName,  inMap["openid"].c_str(),    inMap["openid"].length()+1);
    memcpy(sessionData.szUid,       inMap["uid"].c_str(),       inMap["uid"].length()+1);  // JiaYeHui Add 20150923
    memcpy(sessionData.szLoginIp,   inMap["loading_ip"].c_str(),inMap["loading_ip"].length()+1);
    
    if(0 != WriteContractSession(&sessionData, sOutErrMsg, strSessionKey,domain))
    {
        ErrorLog("[%s %d]写Session失败: 返回消息=%s", __FILE__, __LINE__, sOutErrMsg.c_str());
        throw CTrsExp(ERR_WRITE_SESSION,"系统繁忙");
    }
    DebugLog("[%s %d]写Session: 返回消息=%s", __FILE__, __LINE__, strSessionKey.c_str());
    
    set_cookie("xstn",   inMap["openid"].c_str(),    NULL, "/", "transt.cn",  0);                            //truename
    set_cookie("ltype",  inMap["user_type"].c_str(), NULL, "/", "transt.cn",  0);

    string cookie = this->GetCookieUin();;
    DebugLog("[%s %d]Cookie : =%s", __FILE__, __LINE__,cookie.c_str());
}

void CIdentComm::WriteDataOrder(CStr2Map& inMap,string& sOutErrMsg,string& strSessionKey,const char *domain)
{
    CAdsmemCli aSession;
	InfoLog("Session配置文件路径: %s", g_allVar.GetValue("sessionfile").c_str());
	enum_session_err ret=aSession.Init(g_allVar.GetValue("sessionfile"), strSessionKey);

	if(SESSIONERR_SUCESS!=ret ){
		sOutErrMsg = aSession.GetErrMsg();
		ErrorLog("[%s %d]创建Session失败: 返回值=%d, 返回消息=%s",__FILE__, __LINE__,ret, aSession.GetErrMsg().c_str());
	}
	InfoLog("localIP =[%s]",aSession.GetSessionCfg().m_strLocalIP.c_str());
	if (GetSessionKey(strSessionKey, aSession.GetSessionCfg())<0)
	{
		sOutErrMsg = aSession.GetErrMsg();
        ErrorLog("[%s %d]获取Session key失败: 返回值=%d, 返回消息=%s", __FILE__, __LINE__,ret, aSession.GetErrMsg().c_str());
	}
	aSession.SetSessionKey(strSessionKey);

}

// 创建memcache连接（CGI每次请求都是新进程，每次调用创建临时连接）
static memcached_st* CreateMemcacheConnection()
{
    memcached_st* pMemc = memcached_create(NULL);
    if (pMemc == NULL)
    {
        ErrorLog("memcached_create failed");
        return NULL;
    }

    // 从配置文件读取memcache服务器配置
    // 配置文件格式: <nodes><node ip="127.0.0.1" tcp_port="15210" desp="" /></nodes>
    string sessionFile = g_allVar.GetValue("sessionfile");
    string server_ip = "127.0.0.1";  // 默认值
    int port = 15210;                 // 默认值
    
    if (!sessionFile.empty())
    {
        TiXmlDocument doc;
        if (doc.LoadFile(sessionFile.c_str()))
        {
            TiXmlElement* root = doc.RootElement();
            if (root != NULL)
            {
                TiXmlElement* nodes = root->FirstChildElement("nodes");
                if (nodes != NULL)
                {
                    TiXmlElement* node = nodes->FirstChildElement("node");
                    if (node != NULL)
                    {
                        const char* ip_attr = node->Attribute("ip");
                        const char* port_attr = node->Attribute("tcp_port");
                        
                        if (ip_attr != NULL && strlen(ip_attr) > 0)
                        {
                            server_ip = ip_attr;
                        }
                        if (port_attr != NULL && strlen(port_attr) > 0)
                        {
                            port = atoi(port_attr);
                        }
                    }
                }
            }
        }
        else
        {
            ErrorLog("加载Session配置文件失败: %s, 使用默认配置", sessionFile.c_str());
        }
    }
    else
    {
        ErrorLog("Session配置文件路径为空，使用默认配置");
    }
    
    InfoLog("Memcache配置: ip=%s, port=%d (配置文件: %s)", server_ip.c_str(), port, sessionFile.c_str());
    
    memcached_return_t rc = memcached_server_add(pMemc, server_ip.c_str(), port);
    if (rc != MEMCACHED_SUCCESS)
    {
        ErrorLog("memcached_server_add failed: %s", memcached_strerror(pMemc, rc));
        memcached_free(pMemc);
        return NULL;
    }
    
    DebugLog("memcache连接创建成功: %s:%d", server_ip.c_str(), port);
    return pMemc;
}

// 设置或更新缓存（如果key存在则更新，不存在则插入）
// 注意：每次更新时，过期时间会重新计算（从更新时刻起重新计时24小时），而不是硬性的24小时
// key: 缓存键
// value: 缓存值
// expire_seconds: 过期时间（秒），默认24小时(86400)
//                 每次更新时，过期时间会从当前时刻重新计算，实现"没更新的24小时"机制
// cache_prefix: 缓存键前缀，默认为空（如需前缀可传入）
int CIdentComm::SetOrUpdateCache(const string& key, const string& value, int expire_seconds, const string& cache_prefix)
{
    // CGI每次请求都是新进程，每次调用创建临时连接
    memcached_st* pMemc = CreateMemcacheConnection();
    if (pMemc == NULL)
    {
        ErrorLog("创建memcache连接失败");
        return -1;
    }
    
    string cache_key = cache_prefix.empty() ? key : (cache_prefix + key);
    
    // memcached_set会自动处理插入和更新：
    // - 如果key不存在则插入，过期时间从当前时刻算起
    // - 如果key存在则更新，过期时间会重新设置为expire_seconds（从更新时刻重新计时）
    // 这样就实现了"没更新的24小时"机制：只要在24小时内更新，过期时间就会重新计算
    memcached_return_t rc = memcached_set(pMemc, cache_key.c_str(), cache_key.length(),
                                         value.c_str(), value.length(), 
                                         expire_seconds, 0);
    
    int result = -1;
    if (rc == MEMCACHED_SUCCESS)
    {
        DebugLog("缓存设置/更新成功: key=[%s], value=[%s], expire=[%d]秒(从当前时刻重新计时)", cache_key.c_str(), value.c_str(), expire_seconds);
        result = 0;
    }
    else
    {
        ErrorLog("缓存设置/更新失败: key=[%s], error=[%s]", cache_key.c_str(), memcached_strerror(pMemc, rc));
    }
    
    // 释放连接
    memcached_free(pMemc);
    return result;
}

// 获取缓存值
// key: 缓存键
// value: 输出参数，返回缓存值
// cache_prefix: 缓存键前缀，默认为"term_firm_complete:"
int CIdentComm::GetCache(const string& key, string& value, const string& cache_prefix)
{
    // CGI每次请求都是新进程，每次调用创建临时连接
    memcached_st* pMemc = CreateMemcacheConnection();
    if (pMemc == NULL)
    {
        ErrorLog("创建memcache连接失败");
        return -1;
    }
    
    string cache_key = cache_prefix.empty() ? key : (cache_prefix + key);
    size_t value_length = 0;
    uint32_t flags = 0;
    memcached_return_t rc;
    
    char* pValue = memcached_get(pMemc, cache_key.c_str(), cache_key.length(), 
                                 &value_length, &flags, &rc);
    
    int result = -1;
    if (rc == MEMCACHED_SUCCESS && pValue != NULL)
    {
        value = string(pValue, value_length);
        free(pValue);
        //DebugLog("缓存获取成功: key=[%s], value=[%s]", cache_key.c_str(), value.c_str());
        result = 0;
    }
    else
    {
        if (pValue != NULL)
            free(pValue);
        DebugLog("缓存不存在或获取失败: key=[%s], error=[%s]",cache_key.c_str(), (rc == MEMCACHED_NOTFOUND) ? "NOTFOUND" : memcached_strerror(pMemc, rc));
    }
    
    // 释放连接
    memcached_free(pMemc);
    return result;
}

string CIdentComm::toString(size_t in)
{
    char tmp[64] = {0};
    sprintf(tmp,"%d",(int)in);
    return string(tmp);
}

void CIdentComm::SetOutputType()
{
    /*
     * 按照外部请求，确定输入输出格式。
     */
    if (this->m_pReqData->GetPara("OutPutType") == "JSONP")
    {
        this->m_pResData->SetOutPutType(OUTPUTJSONP);
    }
    else
    {
        this->m_pResData->SetOutPutType(OUTPUTJSON);
    }
}
