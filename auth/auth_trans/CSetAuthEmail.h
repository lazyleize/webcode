#ifndef CSETAUTHEMAIL_H_ 
#define CSETAUTHEMAIL_H_ 

#include "CAuthComm.h"

class CSetAuthEmail : public CAuthComm
{
public:
	CSetAuthEmail(CReqData* pReqData,CResData* pResData)
		: CAuthComm(pReqData,pResData)
	{
	}
	~CSetAuthEmail(){};
	virtual int AuthCommit(CReqData* pReqData,CResData* pResData);
	string GetTid()
	{
		return string("set_auth_email");
	}
};

CTrans* CCgi::MakeTransObj()
{
	CSetAuthEmail* pTrans = new CSetAuthEmail(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif

