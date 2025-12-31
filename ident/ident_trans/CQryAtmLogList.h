#ifndef CQRYATMLOGLIST_H_
#define CQRYATMLOGLIST_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CIdentComm.h"

class CQryAtmLogList: public CIdentComm
{
public:
CQryAtmLogList(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CQryAtmLogList()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("CQryAtmLogList");
		return sTid;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CQryAtmLogList* pTrans = new CQryAtmLogList(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
