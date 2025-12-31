#ifndef _CTEST_H_
#define _CTEST_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CAuthPub.h"
#include "CAuthComm.h"

class CTest: public CAuthComm
{
public:
	CTest(CReqData *pReqData, CResData *pResData) :
		CAuthComm(pReqData, pResData)
	{
	}
	virtual ~CTest()
	{
	}
	virtual int AuthCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("test");
		return sTid;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CTest* pTrans = new CTest(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif
