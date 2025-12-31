#ifndef _CSVNRETRYFIRM_H_
#define _CSVNRETRYFIRM_H_
#include "CIdentComm.h"

class CSvnRetryFirm: public CIdentComm
{
public:
	CSvnRetryFirm(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CSvnRetryFirm()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("svn_retry_firm");
	};

	virtual void CheckParameter(CStr2Map& inMap);
};

CTrans* CCgi::MakeTransObj()
{
	CSvnRetryFirm* pTrans = new CSvnRetryFirm(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
