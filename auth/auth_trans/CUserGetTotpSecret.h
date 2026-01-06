#ifndef CUSERGETTOTPSECRET_H_ 
#define CUSERGETTOTPSECRET_H_ 

#include "CAuthComm.h"

class CUserGetTotpSecret : public CAuthComm
{
public:
    CUserGetTotpSecret(CReqData* pReqData,CResData* pResData)
		: CAuthComm(pReqData,pResData)
	{
	}
	~CUserGetTotpSecret(){};
	virtual int AuthCommit(CReqData* pReqData,CResData* pResData);
	string GetTid()
	{
		return string("user_get_totp_secret");
	}
};

CTrans* CCgi::MakeTransObj()
{
	CUserGetTotpSecret* pTrans = new CUserGetTotpSecret(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif

