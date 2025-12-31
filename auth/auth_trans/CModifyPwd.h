#ifndef _CMODIFY_PWD_H_
#define _CMODIFY_PWD_H_ 

#include "CAuthComm.h"
#include "CRsaTools.h" 

class CModifyPwd : public CAuthComm
{
public:
	CModifyPwd(CReqData* pReqData,CResData* pResData)
		: CAuthComm(pReqData,pResData)
	{
	}
	~CModifyPwd(){};
	virtual int AuthCommit(CReqData* pReqData,CResData* pResData);
	string GetTid()
	{
		return string("modify_pwd");
	}
};

CTrans* CCgi::MakeTransObj()
{
	CModifyPwd* pTrans = new CModifyPwd(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif

