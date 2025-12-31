#ifndef CQRYPRODUCERECORDLIST_H_
#define CQRYPRODUCERECORDLIST_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CIdentComm.h"

class CQueryCollectFuncList: public CIdentComm
{
public:
	CQueryCollectFuncList(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CQueryCollectFuncList()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("query_collect_func_list");
		return sTid;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CQueryCollectFuncList* pTrans = new CQueryCollectFuncList(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
