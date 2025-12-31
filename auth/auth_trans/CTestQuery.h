#ifndef _CTESTQUERY_H_
#define _CTESTQUERY_H_

#include "CAuthComm.h"

class CTestQuery: public CAuthComm
{
public:
	CTestQuery(CReqData *pReqData, CResData *pResData) :
		CAuthComm(pReqData, pResData)
	{
	}
	virtual ~CTestQuery()
	{
	}
	virtual int AuthCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("test_query");
	};
};

CTrans* CCgi::MakeTransObj()
{
	CTestQuery* pTrans = new CTestQuery(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif
