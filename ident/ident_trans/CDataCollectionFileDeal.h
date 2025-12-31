#ifndef _CDATACOLLECTIONFILEDEAL_H_
#define _CDATACOLLECTIONFILEDEAL_H_

#include "CIdentComm.h"

class CDataCollectionFileDeal: public CIdentComm
{
public:
	CDataCollectionFileDeal(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CDataCollectionFileDeal()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("data_collection_file_deal");
	};

private:
    void sendToElasticsearch(string& inStr,CStr2Map& outMap);
};

CTrans* CCgi::MakeTransObj()
{
	CDataCollectionFileDeal* pTrans = new CDataCollectionFileDeal(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
