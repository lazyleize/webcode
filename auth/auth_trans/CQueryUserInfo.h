#ifndef CQUERYUSERINFO_H_
#define CQUERYUSERINFO_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CAuthPub.h"
#include "CAuthComm.h"

class CQueryUserInfo: public CAuthComm
{
public:
	CQueryUserInfo(CReqData *pReqData, CResData *pResData) :
		CAuthComm(pReqData, pResData)
	{
	}
	virtual ~CQueryUserInfo()
	{
	}
	virtual int AuthCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("query_auth_user_info");
		return sTid;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CQueryUserInfo* pTrans = new CQueryUserInfo(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif
