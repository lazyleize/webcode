#ifndef CSETPWDQUESTION_H_
#define CSETPWDQUESTION_H_

#include "CAuthComm.h"

class CSetPwdQuestion : public CAuthComm
{
public:
	CSetPwdQuestion(CReqData* pReqData,CResData* pResData)
		: CAuthComm(pReqData,pResData)
	{
	}
	~CSetPwdQuestion(){};
	virtual int AuthCommit(CReqData* pReqData,CResData* pResData);
	string GetTid()
	{
		return string("set_pwd_question");
	}
};

CTrans* CCgi::MakeTransObj()
{
	CSetPwdQuestion* pTrans = new CSetPwdQuestion(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif

