#ifndef _CGETDATAACQUISITIONREPORT_H_
#define _CGETDATAACQUISITIONREPORT_H_
#include "CIdentComm.h"
#include <datetime.hpp>

class CGetDataAcquisitionReport: public CIdentComm
{
public:
    CGetDataAcquisitionReport(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CGetDataAcquisitionReport()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("get_data_acquisition_report");
	};
    virtual void CheckParameter(CStr2Map& inMap);
	string createAggregationJson(CStr2Map& inMap);
    void sendToElasticsearch(const string& strUrl,string& inStr,string& strOutJson);

private:
	aps::Date m_date;
};


CTrans* CCgi::MakeTransObj()
{
	CGetDataAcquisitionReport* pTrans = new CGetDataAcquisitionReport(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
