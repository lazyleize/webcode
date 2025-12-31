#ifndef CQUERYANNUALSHAREREPORT_H_
#define CQUERYANNUALSHAREREPORT_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CIdentComm.h"

class CQueryAnnualShareReport: public CIdentComm
{
public:
    CQueryAnnualShareReport(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CQueryAnnualShareReport()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("query_annual_share_report");
		return sTid;
	};
    virtual void CheckParameter(CStr2Map& inMap);
    void sendToElasticsearch(const string& strUrl,string& inStr,string& strOutJson);
};

CTrans* CCgi::MakeTransObj()
{
	CQueryAnnualShareReport* pTrans = new CQueryAnnualShareReport(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
