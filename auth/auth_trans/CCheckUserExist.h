#ifndef _CCHECKUSEREXIST_H_
#define _CCHECKUSEREXIST_H_

#include "CAuthComm.h"

class CCheckUserExist: public CAuthComm
{
public:
	CCheckUserExist(CReqData *pReqData, CResData *pResData) :
		CAuthComm(pReqData, pResData)
	{
	}
	virtual ~CCheckUserExist()
	{
	}
	virtual int AuthCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("check_user_exist");
	};
};

CTrans* CCgi::MakeTransObj()
{
	CCheckUserExist* pTrans = new CCheckUserExist(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif
