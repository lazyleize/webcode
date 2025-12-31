#ifndef CQUERYEXPORTTYPELIST_H_
#define CQUERYEXPORTTYPELIST_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CIdentComm.h"

class CQueryExportTypeList: public CIdentComm
{
public:
    CQueryExportTypeList(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CQueryExportTypeList()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("query_export_type_list");
		return sTid;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CQueryExportTypeList* pTrans = new CQueryExportTypeList(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
