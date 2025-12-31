#ifndef _CCHECKUSERANSWER_H_
#define _CCHECKUSERANSWER_H_

#include "CAuthComm.h"

class CCheckUserAnswer: public CAuthComm
{
public:
	CCheckUserAnswer(CReqData *pReqData, CResData *pResData) :
		CAuthComm(pReqData, pResData)
	{
	}
	virtual ~CCheckUserAnswer()
	{
	}
	virtual int AuthCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("check_user_answer");;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CCheckUserAnswer* pTrans = new CCheckUserAnswer(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif
