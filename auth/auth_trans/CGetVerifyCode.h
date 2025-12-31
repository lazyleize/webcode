#ifndef _CGET_VERIFYCODE_H_
#define _CGET_VERIFYCODE_H_ 

#include "CAuthComm.h"

class CGetVerifyCode : public CAuthComm
{
public:
	CGetVerifyCode(CReqData* pReqData,CResData* pResData)
		: CAuthComm(pReqData,pResData)
	{
	}
	~CGetVerifyCode(){};
	virtual int AuthCommit(CReqData* pReqData,CResData* pResData);
	string GetTid()
	{
		return string("get_verify_code");
	}
};

CTrans* CCgi::MakeTransObj()
{
	CGetVerifyCode* pTrans = new CGetVerifyCode(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif

