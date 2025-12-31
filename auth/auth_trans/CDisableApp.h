#ifndef _CDISABLEAPP_H_
#define _CDISABLEAPP_H_

#include "CAuthComm.h"
#include "CRsaTools.h" 

class CDisableApp: public CAuthComm
{
public:
	CDisableApp(CReqData *pReqData, CResData *pResData) :
		CAuthComm(pReqData, pResData)
	{
	}
	virtual ~CDisableApp()
	{
	}
	virtual int AuthCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("disable_app");
	};
	virtual void CheckParameter(CStr2Map& inMap);
};

CTrans* CCgi::MakeTransObj()
{
	CDisableApp* pTrans = new CDisableApp(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif
