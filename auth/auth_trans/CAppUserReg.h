#ifndef _CAPPUSERREG_H_
#define _CAPPUSERREG_H_

#include "CAuthComm.h"
#include "CRsaTools.h" 

class CAppUserReg: public CAuthComm
{
public:
	CAppUserReg(CReqData *pReqData, CResData *pResData) :
		CAuthComm(pReqData, pResData)
	{
	}
	virtual ~CAppUserReg()
	{
	}
	virtual int AuthCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("app_user_reg");
	};
};

CTrans* CCgi::MakeTransObj()
{
	CAppUserReg* pTrans = new CAppUserReg(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif
