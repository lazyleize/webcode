#ifndef _CSET_NEW_PWD_H_
#define _CSET_NEW_PWD_H_ 

#include "CAuthComm.h"
#include "CRsaTools.h" 

class CSetNewPwd : public CAuthComm
{
public:
	CSetNewPwd(CReqData* pReqData,CResData* pResData)
		: CAuthComm(pReqData,pResData)
	{
	}
	~CSetNewPwd(){};
	virtual int AuthCommit(CReqData* pReqData,CResData* pResData);
	string GetTid()
	{
		return string("set_new_pwd");
	}
};

CTrans* CCgi::MakeTransObj()
{
	CSetNewPwd* pTrans = new CSetNewPwd(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif

