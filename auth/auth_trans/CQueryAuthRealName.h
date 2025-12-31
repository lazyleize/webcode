#ifndef CQUERYAUTHREALNAME_H_
#define CQUERYAUTHREALNAME_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CAuthPub.h"
#include "CAuthComm.h"

class CQueryAuthRealName: public CAuthComm
{
public:
	CQueryAuthRealName(CReqData *pReqData, CResData *pResData) :
		CAuthComm(pReqData, pResData)
	{
	}
	virtual ~CQueryAuthRealName()
	{
	}
	virtual int AuthCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("query_auth_real_name");
		return sTid;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CQueryAuthRealName* pTrans = new CQueryAuthRealName(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif
