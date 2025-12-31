#ifndef _CRESTPWDAPP_H__
#define _CRESTPWDAPP_H__

#include "CAuthComm.h"
#include "CAuthPub.h"
#include "CRsaTools.h" 

class CRestPwdApp : public CAuthComm
{
public:
	CRestPwdApp(CReqData *pReqData, CResData *pResData) : 
		CAuthComm(pReqData, pResData) 
	{
	}
	virtual ~CRestPwdApp(){}
    int AuthCommit(CReqData *pReqData, CResData *pResData);
	unsigned char* Str2Hex(const char *str, unsigned char *hex, int sLen);
	string GetTid(){
		return string("reset_pwd_app");
	};
};

CTrans* CCgi::MakeTransObj()
{
	CRestPwdApp* pTrans = new CRestPwdApp(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif
