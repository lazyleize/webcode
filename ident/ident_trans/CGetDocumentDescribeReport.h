#ifndef _CGETDOCUMENTDESCRIBEREPORT_H_
#define _CGETDOCUMENTDESCRIBEREPORT_H_
#include "CIdentComm.h"

class CGetDocumentDescribeReport: public CIdentComm
{
public:
    CGetDocumentDescribeReport(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CGetDocumentDescribeReport()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("get_document_describe_report");
	};
};


CTrans* CCgi::MakeTransObj()
{
	CGetDocumentDescribeReport* pTrans = new CGetDocumentDescribeReport(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
