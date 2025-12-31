#ifndef _CCREATEPWDTRAN_H_
#define _CCREATEPWDTRAN_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CAuthPub.h"
#include "CAuthComm.h"
#include "CRsaTools.h" 

class CCreatePwdTran: public CAuthComm
{
public:
	CCreatePwdTran(CReqData *pReqData, CResData *pResData) :
		CAuthComm(pReqData, pResData)
	{
	}
	virtual ~CCreatePwdTran()
	{
	}
	virtual int AuthCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("create_pwd_tran");
		return sTid;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CCreatePwdTran* pTrans = new CCreatePwdTran(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif
