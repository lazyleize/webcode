#ifndef _CDATACOLLECTCOUNTREPORT_H_
#define _CDATACOLLECTCOUNTREPORT_H_
#include "CIdentComm.h"

class CDataCollectCountReport: public CIdentComm
{
public:
    CDataCollectCountReport(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CDataCollectCountReport()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("data_collect_count_report");
	};
};

CTrans* CCgi::MakeTransObj()
{
	CDataCollectCountReport* pTrans = new CDataCollectCountReport(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
