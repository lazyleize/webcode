#ifndef _CQUERYTERMRUNFILE_H_
#define _CQUERYTERMRUNFILE_H_
#include "CIdentComm.h"

class CQueryTermRunFile: public CIdentComm
{
public:
    CQueryTermRunFile(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CQueryTermRunFile()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("query_term_run_file");
	};

	virtual void CheckParameter(CStr2Map& inMap);
};

CTrans* CCgi::MakeTransObj()
{
	CQueryTermRunFile* pTrans = new CQueryTermRunFile(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
