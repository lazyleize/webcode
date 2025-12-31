//批量查询采集数据JSON列表
#ifndef CQUERYCOLLECTIONDATALIST_H_
#define CQUERYCOLLECTIONDATALIST_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CIdentComm.h"

class CQueryCollectionDataList: public CIdentComm
{
public:
    CQueryCollectionDataList(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CQueryCollectionDataList()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("query_collection_data_list");
		return sTid;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CQueryCollectionDataList* pTrans = new CQueryCollectionDataList(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
