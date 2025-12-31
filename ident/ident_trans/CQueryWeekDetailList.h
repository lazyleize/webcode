#ifndef CQUERYWEEKDETAILLIST_H_
#define CQUERYWEEKDETAILLIST_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CIdentComm.h"

class CQueryWeekDetailList: public CIdentComm
{
public:
    CQueryWeekDetailList(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CQueryWeekDetailList()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("query_week_detail_list");
		return sTid;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CQueryWeekDetailList* pTrans = new CQueryWeekDetailList(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
