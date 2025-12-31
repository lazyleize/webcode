#ifndef CCREATEAUTHINFO_H_
#define CCREATEAUTHINFO_H_

#include "CAuthComm.h"

class CCreateAuthInfo : public CAuthComm
{
public:
	CCreateAuthInfo(CReqData* pReqData,CResData* pResData)
		: CAuthComm(pReqData,pResData)
	{
	}
	~CCreateAuthInfo(){};
	virtual int AuthCommit(CReqData* pReqData,CResData* pResData);
	string GetTid()
	{
		return string("create_auth_real_name");
	}
};

CTrans* CCgi::MakeTransObj()
{
	CCreateAuthInfo* pTrans = new CCreateAuthInfo(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif

