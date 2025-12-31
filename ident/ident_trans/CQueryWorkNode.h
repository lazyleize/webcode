#ifndef CQUERYWORKNODE_H_
#define CQUERYWORKNODE_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CIdentComm.h"

class CQueryWorkNode: public CIdentComm
{
public:
    CQueryWorkNode(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CQueryWorkNode()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("query_work_node");
		return sTid;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CQueryWorkNode* pTrans = new CQueryWorkNode(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
