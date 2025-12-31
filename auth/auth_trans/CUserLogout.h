#ifndef _CUSER_LOGOUT_H__
#define _CUSER_LOGOUT_H__

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CAuthPub.h"
#include "CAuthComm.h"

class CUserLogout: public CAuthComm
{
public:
	CUserLogout(CReqData *pReqData, CResData *pResData) :
		CAuthComm(pReqData, pResData)
	{
	}
	virtual ~CUserLogout()
	{
	}
	virtual int AuthCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("user_logout");
		return sTid;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CUserLogout* pTrans = new CUserLogout(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif
