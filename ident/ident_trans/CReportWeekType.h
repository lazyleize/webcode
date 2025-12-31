#ifndef CQRYPRODUCERECORDLIST_H_
#define CQRYPRODUCERECORDLIST_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CIdentComm.h"

class CReportWeekType: public CIdentComm
{
public:
	CReportWeekType(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CReportWeekType()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("report_week_type");
		return sTid;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CReportWeekType* pTrans = new CReportWeekType(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
