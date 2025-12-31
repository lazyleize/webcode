#ifndef CQUERYPRDRECORD_H_
#define CQUERYPRDRECORD_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CIdentComm.h"

class CQryPrdRecord: public CIdentComm
{
public:
	CQryPrdRecord(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CQryPrdRecord()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("qry_prd_record");
		return sTid;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CQryPrdRecord* pTrans = new CQryPrdRecord(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
