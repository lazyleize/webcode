#ifndef _CQUERYANSWERINFO_H_
#define _CQUERYANSWERINFO_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CAuthPub.h"
#include "CAuthComm.h"

class CQueryAnswerInfo: public CAuthComm
{
public:
	CQueryAnswerInfo(CReqData *pReqData, CResData *pResData) :
		CAuthComm(pReqData, pResData)
	{
	}
	virtual ~CQueryAnswerInfo()
	{
	}
	virtual int AuthCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("query_pwd_question");
		return sTid;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CQueryAnswerInfo* pTrans = new CQueryAnswerInfo(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif
