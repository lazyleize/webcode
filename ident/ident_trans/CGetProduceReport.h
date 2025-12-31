#ifndef _CGETPRODUCEREPORT_H_
#define _CGETPRODUCEREPORT_H_
#include "CIdentComm.h"

class CGetProduceReport: public CIdentComm
{
public:
    CGetProduceReport(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CGetProduceReport()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("get_produce_report");
	};

	virtual void CheckParameter(CStr2Map& inMap);
};

CTrans* CCgi::MakeTransObj()
{
	CGetProduceReport* pTrans = new CGetProduceReport(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
