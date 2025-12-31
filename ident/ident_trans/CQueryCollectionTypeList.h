#ifndef CQUERYCOLLECTTYPELIST_H_
#define CQUERYCOLLECTTYPELIST_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CIdentComm.h"

class CQueryCollectTypeList: public CIdentComm
{
public:
    CQueryCollectTypeList(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CQueryCollectTypeList()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("query_collection_type_list");
		return sTid;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CQueryCollectTypeList* pTrans = new CQueryCollectTypeList(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
