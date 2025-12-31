#ifndef CQUERYAUTHSAFESTATUS_H_
#define CQUERYAUTHSAFESTATUS_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CAuthPub.h"
#include "CAuthComm.h"

class CQueryAuthSafeStatus: public CAuthComm
{
public:
	CQueryAuthSafeStatus(CReqData *pReqData, CResData *pResData) :
		CAuthComm(pReqData, pResData)
	{
	}
	virtual ~CQueryAuthSafeStatus()
	{
	}
	virtual int AuthCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("query_auth_safe_stats");
		return sTid;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CQueryAuthSafeStatus* pTrans = new CQueryAuthSafeStatus(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif
