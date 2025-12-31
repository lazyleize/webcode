#ifndef CQUERYDOWNRULESLIST_H_
#define CQUERYDOWNRULESLIST_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CIdentComm.h"

class CQueryDownRulesList: public CIdentComm
{
public:
	CQueryDownRulesList(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CQueryDownRulesList()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("qry_down_rules_list");
		return sTid;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CQueryDownRulesList* pTrans = new CQueryDownRulesList(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
