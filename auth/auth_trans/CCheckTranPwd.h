#ifndef _CCHECKTRANPWD_H_
#define _CCHECKTRANPWD_H_

#include "CAuthComm.h"
#include "CRsaTools.h" 

class CCheckTranPwd: public CAuthComm
{
public:
	CCheckTranPwd(CReqData *pReqData, CResData *pResData) :
		CAuthComm(pReqData, pResData)
	{
	}
	virtual ~CCheckTranPwd()
	{
	}
	virtual int AuthCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("check_tran_pwd");;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CCheckTranPwd* pTrans = new CCheckTranPwd(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif
