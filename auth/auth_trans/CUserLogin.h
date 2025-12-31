#ifndef _CUSER_LOGIN_H__
#define _CUSER_LOGIN_H__

#include "CAuthComm.h"
#include "CAuthPub.h"
#include "CRsaTools.h" 

class CUserLogin : public CAuthComm
{
public:
	CUserLogin(CReqData *pReqData, CResData *pResData) : 
		CAuthComm(pReqData, pResData) 
	{
	}
	virtual ~CUserLogin(){}
    int AuthCommit(CReqData *pReqData, CResData *pResData);
	unsigned char* Str2Hex(const char *str, unsigned char *hex, int sLen);
	string GetTid(){
		return string("user_login");
	};
	int GetLoginType(const string & uin);
};

CTrans* CCgi::MakeTransObj()
{
	CUserLogin* pTrans = new CUserLogin(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif
