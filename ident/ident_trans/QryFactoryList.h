#ifndef CQRYFACTORYLIST_H_
#define CQRYFACTORYLIST_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CIdentComm.h"

class CQryFactoryList: public CIdentComm
{
public:
	CQryFactoryList(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CQryFactoryList()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("qry_factory_list");
		return sTid;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CQryFactoryList* pTrans = new CQryFactoryList(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
