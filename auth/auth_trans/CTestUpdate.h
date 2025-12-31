#ifndef _CTESTUPDATE_H_
#define _CTESTUPDATE_H_

#include "CAuthComm.h"

class CTestUpdate: public CAuthComm
{
public:
	CTestUpdate(CReqData *pReqData, CResData *pResData) :
		CAuthComm(pReqData, pResData)
	{
	}
	virtual ~CTestUpdate()
	{
	}
	virtual int AuthCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("test_update");
	};
};

CTrans* CCgi::MakeTransObj()
{
	CTestUpdate* pTrans = new CTestUpdate(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif
