#ifndef _CGETACTIVECODE_H_
#define _CGETACTIVECODE_H_ 

#include "CAuthComm.h"

class CGetActiveCode : public CAuthComm
{
public:
	CGetActiveCode(CReqData* pReqData,CResData* pResData)
		: CAuthComm(pReqData,pResData)
	{
	}
	~CGetActiveCode(){};
	virtual int AuthCommit(CReqData* pReqData,CResData* pResData);
	string GetTid()
	{
		return string("get_active_code");
	}
};

CTrans* CCgi::MakeTransObj()
{
	CGetActiveCode* pTrans = new CGetActiveCode(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif

