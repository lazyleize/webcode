#ifndef CQUERYFIRMDOWNLIST_H_
#define CQUERYFIRMDOWNLIST_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CIdentComm.h"

class CQueryFirmDownList: public CIdentComm
{
public:
	CQueryFirmDownList(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CQueryFirmDownList()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("query_firm_down_list");
		return sTid;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CQueryFirmDownList* pTrans = new CQueryFirmDownList(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
