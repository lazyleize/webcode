#ifndef _CCREATEUSERINFO_H_
#define _CCREATEUSERINFO_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CAuthPub.h"
#include "CAuthComm.h"
#include "CRsaTools.h" 

class CCreateUserInfo: public CAuthComm
{
public:
	CCreateUserInfo(CReqData *pReqData, CResData *pResData) :
		CAuthComm(pReqData, pResData)
	{
	}
	virtual ~CCreateUserInfo()
	{
	}
	virtual int AuthCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("create_user");
		return sTid;
	};
	virtual void CheckParameter(CStr2Map& inMap);
};

CTrans* CCgi::MakeTransObj()
{
	CCreateUserInfo* pTrans = new CCreateUserInfo(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif
