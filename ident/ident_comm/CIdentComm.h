#ifndef CJIFENCOMM_H_
#define CJIFENCOMM_H_

#include "ident_err.h"
#include "cgicomm/CCgi.h"
#include "CIdentPub.h"

class CIdentComm: public CTrans
{
public:
  CIdentComm(CReqData *pReqData, CResData *pResData) : CTrans(pReqData, pResData)
  {
    m_pReqData = pReqData;
    m_pResData = pResData;
  }

  virtual ~CIdentComm()
  {
  }

  /*
   * 内部函数，按照用户的输入来指定输出方式。
   * 默认输出为JSON。
   */
  void SetOutputType();
  string toString(size_t in);

protected:
  /**
   * CGI框架的业务入口。
   * 此处重写该方法，并尽可能对下隐藏，转而提供IdentCommit函数作为替代。
   */
  int Commit();
  /*
   * 业务的入口函数。
   * 继承子类时，需要从重写该函数开始。
   */
  virtual int IdentCommit(CReqData *pReqData, CResData *pResData) = 0;
  virtual string GetTid() = 0;

protected:
    //获取登录态 分别有用户名登录、邮箱登录、手机号登录
    inline string GetCookieUin() const
    {
        return this->m_pReqData->GetCookie("xsuin");
    }

    //获取用户的权限状态，分别有管理用户，财务用户，客服用户，普通用户
    inline string GetCookieUserType() const
    {
        return this->m_pReqData->GetCookie("ltype");
    }
    inline string GetClientIp() const
    {
        return this->m_pReqData->GetEnv("ClientIp");
    }
    inline string GetWeiXinAppid() const
    {
        return string("wx5c06aebc3b67f70f");
    }
    
    void CheckFile(const cgicc::FormFile formFile);
    void saveUploadFile(const cgicc::FormFile& formFile,const string& fileName);
    string saveUploadFile(const cgicc::FormFile& formFile,bool isTransform);    
    string saveUploadFile(const cgicc::FormFile& formFile);
    string saveUploadFile(const cgicc::FormFile& formFile,string& OutFullPath);
    string GetVerifyCode();

    int UploadFile(CReqData *pReqData, CStr2Map &fileName,const unsigned int fileCount = 1);
    int UploadFile(CReqData *pReqData, CStr2Map &fileName,const unsigned int fileCount = 1, bool isTransform = true);

    void CheckVerifyCode(const string & verify_code);
    void DelMapF(CStr2Map& dataMap,CStr2Map& outMap);
    void WriteSessionCookie(CStr2Map& inMap, string& sOutErrMsg, string& strSessionKey,const char *domain);
    void WriteDataOrder(CStr2Map& inMap, string& sOutErrMsg, string& strSessionKey,const char *domain);
    
    // memcache缓存操作函数
    // 设置或更新缓存（如果key存在则更新，不存在则插入）
    // 注意：每次更新时，过期时间会重新计算（从更新时刻起重新计时24小时），实现"没更新的24小时"机制
    // key: 缓存键（直接使用，不加前缀）
    // value: 缓存值
    // expire_seconds: 过期时间（秒），默认24小时(86400)
    //                 每次更新时，过期时间会从当前时刻重新计算，而不是硬性的24小时
    // cache_prefix: 缓存键前缀，默认为空（如需前缀可传入）
    int SetOrUpdateCache(const string& key, const string& value, int expire_seconds = 86400, const string& cache_prefix = "");
    
    // 获取缓存值
    // key: 缓存键（直接使用，不加前缀）
    // value: 输出参数，返回缓存值
    // cache_prefix: 缓存键前缀，默认为空（如需前缀可传入）
    int GetCache(const string& key, string& value, const string& cache_prefix = "");

protected:
  CReqData* m_pReqData;
  CResData* m_pResData;
};

#endif /* CJIFENCOMM_H_ */
