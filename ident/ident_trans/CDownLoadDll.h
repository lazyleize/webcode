#ifndef _CDOWNLOADDLL_H_
#define _CDOWNLOADDLL_H_
#include "CIdentComm.h"

class CDownLoadDll: public CIdentComm
{
public:
	CDownLoadDll(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CDownLoadDll()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("download_dll");
	};

	virtual void CheckParameter(CStr2Map& inMap);
};

CTrans* CCgi::MakeTransObj()
{
	CDownLoadDll* pTrans = new CDownLoadDll(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
