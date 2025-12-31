#ifndef CQUERYDATACOLLECTIONLIST_H_
#define CQUERYDATACOLLECTIONLIST_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CIdentComm.h"

class CQueryDataCollectList: public CIdentComm
{
public:
    CQueryDataCollectList(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CQueryDataCollectList()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("query_data_collection_list");
		return sTid;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CQueryDataCollectList* pTrans = new CQueryDataCollectList(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
