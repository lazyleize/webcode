#ifndef SETAUTHMOBILE_H_
#define SETAUTHMOBILE_H_

#include "CAuthComm.h"

class CSetAuthMobile : public CAuthComm
{
public:
	CSetAuthMobile(CReqData* pReqData,CResData* pResData)
		: CAuthComm(pReqData,pResData)
	{
	}
	~CSetAuthMobile(){};
	virtual int AuthCommit(CReqData* pReqData,CResData* pResData);
	string GetTid()
	{
		return string("set_auth_mobile");
	}
};

CTrans* CCgi::MakeTransObj()
{
	CSetAuthMobile* pTrans = new CSetAuthMobile(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif

