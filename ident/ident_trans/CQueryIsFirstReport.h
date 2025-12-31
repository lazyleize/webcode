#ifndef _CQUERYISFIRSTREPORT_H_
#define _CQUERYISFIRSTREPORT_H_
#include "CIdentComm.h"

class CQueryIsFirstReport: public CIdentComm
{
public:
    CQueryIsFirstReport(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CQueryIsFirstReport()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("query_is_first_report");
	};
};

CTrans* CCgi::MakeTransObj()
{
	CQueryIsFirstReport* pTrans = new CQueryIsFirstReport(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
