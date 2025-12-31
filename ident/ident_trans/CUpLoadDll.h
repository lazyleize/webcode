#ifndef _CUPLOADDLL_H_
#define _CUPLOADDLL_H_

#include "CIdentComm.h"

class CUpLoadDll: public CIdentComm
{
public:
	CUpLoadDll(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CUpLoadDll()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("upload_dll");
	};

private:
	int checkVersion(const char* version);
};

CTrans* CCgi::MakeTransObj()
{
	CUpLoadDll* pTrans = new CUpLoadDll(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
