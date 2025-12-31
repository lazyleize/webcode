#ifndef _CCHECK_VERIFYCODE_H_
#define _CCHECK_VERIFYCODE_H_ 

#include "CAuthComm.h"

class CCheckVerifyCode : public CAuthComm
{
public:
	CCheckVerifyCode(CReqData* pReqData,CResData* pResData)
		: CAuthComm(pReqData,pResData)
	{
	}
	~CCheckVerifyCode(){};
	virtual int AuthCommit(CReqData* pReqData,CResData* pResData);
	string GetTid()
	{
		return string("check_verify_code");
	}
};

CTrans* CCgi::MakeTransObj()
{
	CCheckVerifyCode* pTrans = new CCheckVerifyCode(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif

