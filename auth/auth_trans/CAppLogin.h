#ifndef _CUSER_LOGIN_H__
#define _CUSER_LOGIN_H__

#include "CAuthComm.h"
#include "CAuthPub.h"
#include "CRsaTools.h" 

class CAppLogin : public CAuthComm
{
public:
	CAppLogin(CReqData *pReqData, CResData *pResData) : 
		CAuthComm(pReqData, pResData) 
	{
	}
	virtual ~CAppLogin(){}
    int AuthCommit(CReqData *pReqData, CResData *pResData);
	unsigned char* Str2Hex(const char *str, unsigned char *hex, int sLen);
	string GetTid(){
		return string("app_login");
	};
	virtual void CheckParameter(CStr2Map& inMap);
	int GetLoginType(const string & uin);
	int compare_version(const char *dest, const char *str, int number);
};

CTrans* CCgi::MakeTransObj()
{
	CAppLogin* pTrans = new CAppLogin(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif
