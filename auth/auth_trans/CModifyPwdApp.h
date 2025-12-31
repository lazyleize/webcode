#ifndef _CMODIFY_PWD_APP_H_
#define _CMODIFY_PWD_APP_H_ 

#include "CAuthComm.h"
#include "CRsaTools.h" 

class CModifyPwdApp : public CAuthComm
{
public:
	CModifyPwdApp(CReqData* pReqData,CResData* pResData)
		: CAuthComm(pReqData,pResData)
	{
	}
	~CModifyPwdApp(){};
	virtual int AuthCommit(CReqData* pReqData,CResData* pResData);
	string GetTid()
	{
		return string("modify_pwd_app");
	}
};

CTrans* CCgi::MakeTransObj()
{
	CModifyPwdApp* pTrans = new CModifyPwdApp(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif

