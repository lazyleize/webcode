#ifndef _CQUERYDATAEXPORTCOUNT_H_
#define _CQUERYDATAEXPORTCOUNT_H_
#include "CIdentComm.h"

class CQueryDataExportCount: public CIdentComm
{
public:
    CQueryDataExportCount(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CQueryDataExportCount()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("query_data_export_count");
	};
};

CTrans* CCgi::MakeTransObj()
{
	CQueryDataExportCount* pTrans = new CQueryDataExportCount(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
