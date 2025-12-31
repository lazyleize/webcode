//数据采集单笔查询,融合二级数组返回
#ifndef CQUERYCOLLECTIONDATA_H_
#define CQUERYCOLLECTIONDATA_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CIdentComm.h"

class CQueryCollectionData: public CIdentComm
{
public:
    CQueryCollectionData(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CQueryCollectionData()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("query_collection_data");
		return sTid;
	};
    virtual void CheckParameter(CStr2Map& inMap);
	void QueryESdoc(const string& strUrl,string& inStr,string& outStr);
};

CTrans* CCgi::MakeTransObj()
{
	CQueryCollectionData* pTrans = new CQueryCollectionData(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
