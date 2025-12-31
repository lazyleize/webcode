#ifndef CAUTHCOMM_H_
#define CAUTHCOMM_H_
#include "auth_err.h"
#include "cgicomm/CCgi.h"
#include "CAuthPub.h"

class CAuthComm: public CTrans
{
public:
	CAuthComm(CReqData *pReqData, CResData *pResData) :
		CTrans(pReqData, pResData)
	{
		m_pReqData = pReqData;
		m_pResData = pResData;
	}

	virtual ~CAuthComm()
	{
	}
protected:
	/**
	 * CGI框架的业务入口。
	 * 此处重写该方法，并尽可能对下隐藏，转而提供AuthCommit函数作为替代。
	 */
	int Commit();
	/*
	 * 业务的入口函数。
	 * 继承子类时，需要从重写该函数开始。
	 */
	virtual int AuthCommit(CReqData *pReqData, CResData *pResData) = 0;
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
	void CheckFile(const cgicc::FormFile formFile);
	void saveUploadFile(const cgicc::FormFile& formFile,const string& fileName);
	string saveUploadFile(const cgicc::FormFile& formFile);
	int UploadFile(CReqData *pReqData, CStr2Map &fileName,const unsigned int fileCount = 1);
	string GetVerifyCode();
	void CheckVerifyCode(const string & verify_code);
	 /*删除Map中key中开头为F字样*/
    void DelMapF(CStr2Map& dataMap,CStr2Map& outMap);
protected:
	CReqData* m_pReqData;
	CResData* m_pResData;
};

#endif /* CAUTHCOMM_H_ */
