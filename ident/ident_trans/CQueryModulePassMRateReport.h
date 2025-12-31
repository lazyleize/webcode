#ifndef CQUERYMODULEPASSMRATEREPORT_H_
#define CQUERYMODULEPASSMRATEREPORT_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CIdentComm.h"

class CQueryModulePassMRateReport: public CIdentComm
{
public:
    CQueryModulePassMRateReport(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CQueryModulePassMRateReport()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("query_module_pass_m_rate_report");
		return sTid;
	};
    virtual void CheckParameter(CStr2Map& inMap);
    string createAggregationJson(int size, const string& posname, const vector<string>& moduleList);
	string createAggregationJson(int& possize, int& module, int month_num = 3);
	string createAggregationJson1(int& possize, int& module, int month_num = 3);
    void sendToElasticsearch(const string& strUrl,string& inStr,string& strOutJson);
};

CTrans* CCgi::MakeTransObj()
{
	CQueryModulePassMRateReport* pTrans = new CQueryModulePassMRateReport(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTTEXT);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
