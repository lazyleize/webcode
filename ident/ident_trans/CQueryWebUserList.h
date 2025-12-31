#ifndef CQUERYWEBUSERLIST_H_
#define CQUERYWEBUSERLIST_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CIdentComm.h"

class CQueryWebUserList: public CIdentComm
{
public:
	CQueryWebUserList(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CQueryWebUserList()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("qry_web_user_list");
		return sTid;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CQueryWebUserList* pTrans = new CQueryWebUserList(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
