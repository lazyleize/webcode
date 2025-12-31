#ifndef CQUERYFIRMDOWNDETAILS_H_
#define CQUERYFIRMDOWNDETAILS_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CIdentComm.h"

class CQueryFirmDownDetails: public CIdentComm
{
public:
    CQueryFirmDownDetails(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CQueryFirmDownDetails()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("query_firm_down_details");
		return sTid;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CQueryFirmDownDetails* pTrans = new CQueryFirmDownDetails(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
