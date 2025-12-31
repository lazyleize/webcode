#ifndef CQUERYUSERBASEINFO_H_
#define CQUERYUSERBASEINFO_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CAuthPub.h"
#include "CAuthComm.h"

class CQueryUserBaseInfo: public CAuthComm
{
public:
	CQueryUserBaseInfo(CReqData *pReqData, CResData *pResData) :
		CAuthComm(pReqData, pResData)
	{
	}
	virtual ~CQueryUserBaseInfo()
	{
	}
	virtual int AuthCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("query_auth_user_baseinfo");
		return sTid;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CQueryUserBaseInfo* pTrans = new CQueryUserBaseInfo(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif
