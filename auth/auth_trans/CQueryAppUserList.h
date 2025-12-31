#ifndef CQUERYAPPUSERLIST_H_
#define CQUERYAPPUSERLIST_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CAuthPub.h"
#include "CAuthComm.h"

class CQueryAppUserList: public CAuthComm
{
public:
	CQueryAppUserList(CReqData *pReqData, CResData *pResData) :
		CAuthComm(pReqData, pResData)
	{
	}
	virtual ~CQueryAppUserList()
	{
	}
	virtual int AuthCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("query_app_user_list");
		return sTid;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CQueryAppUserList* pTrans = new CQueryAppUserList(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif
