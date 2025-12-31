#ifndef CMODIFYEMAIL_H_ 
#define CMODIFYEMAIL_H_ 

#include "CAuthComm.h"

class CModifyEmail : public CAuthComm
{
public:
	CModifyEmail(CReqData* pReqData,CResData* pResData)
		: CAuthComm(pReqData,pResData)
	{
	}
	~CModifyEmail(){};
	virtual int AuthCommit(CReqData* pReqData,CResData* pResData);
	string GetTid()
	{
		return string("modify_email");
	}
};

CTrans* CCgi::MakeTransObj()
{
	CModifyEmail* pTrans = new CModifyEmail(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif

