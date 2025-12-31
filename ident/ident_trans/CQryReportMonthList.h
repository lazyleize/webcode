#ifndef CQRYPRODUCERECORDLIST_H_
#define CQRYPRODUCERECORDLIST_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CIdentComm.h"

class CQryReportMonthList: public CIdentComm
{
public:
    CQryReportMonthList(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CQryReportMonthList()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("qry_report_month_list");
		return sTid;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CQryReportMonthList* pTrans = new CQryReportMonthList(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
