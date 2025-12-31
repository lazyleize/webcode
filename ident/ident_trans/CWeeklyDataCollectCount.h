#ifndef CWEEKLYDATACOLLECTCOUNT_H_
#define CWEEKLYDATACOLLECTCOUNT_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CIdentComm.h"

class CWeeklyDataCollectCount: public CIdentComm
{
public:
    CWeeklyDataCollectCount(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CWeeklyDataCollectCount()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("weekly_data_collect_count");
		return sTid;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CWeeklyDataCollectCount* pTrans = new CWeeklyDataCollectCount(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
