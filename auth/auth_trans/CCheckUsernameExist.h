#ifndef _CCHECKUSERNAMEEXIST_H_
#define _CCHECKUSERNAMEEXIST_H_

#include "CAuthComm.h"

class CCheckUsernameExist: public CAuthComm
{
public:
	CCheckUsernameExist(CReqData *pReqData, CResData *pResData) :
		CAuthComm(pReqData, pResData)
	{
	}
	virtual ~CCheckUsernameExist()
	{
	}
	virtual int AuthCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("check_username_exist");
	};
};

CTrans* CCgi::MakeTransObj()
{
	CCheckUsernameExist* pTrans = new CCheckUsernameExist(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif
