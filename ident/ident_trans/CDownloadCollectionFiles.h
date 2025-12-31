#ifndef _CDOWNLOADCOLLECTIONFILES_H_
#define _CDOWNLOADCOLLECTIONFILES_H_
#include "CIdentComm.h"

class CDownloadCollectionFiles: public CIdentComm
{
public:
	CDownloadCollectionFiles(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CDownloadCollectionFiles()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("download_collection_files");
	};

	virtual void CheckParameter(CStr2Map& inMap);
};

CTrans* CCgi::MakeTransObj()
{
	CDownloadCollectionFiles* pTrans = new CDownloadCollectionFiles(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
