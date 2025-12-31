#ifndef _CCHECK_REAL_NAME_H_
#define _CCHECK_REAL_NAME_H_

#include "CAuthComm.h"

class CCheckRealName: public CAuthComm
{
public:
	CCheckRealName(CReqData *pReqData, CResData *pResData) :
		CAuthComm(pReqData, pResData)
	{
	}
	virtual ~CCheckRealName()
	{
	}
	virtual int AuthCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("check_real_name");;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CCheckRealName* pTrans = new CCheckRealName(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif
