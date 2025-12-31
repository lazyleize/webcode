/*********************************************************
 Desc:  财大户发送邮件
 Auth:  Jerry
 Modify:
 date:  2015-07-30
 *********************************************************/
#ifndef _CSENDUSEREMAIL_H_
#define _CSENDUSEREMAIL_H_ 

#include "CAuthComm.h"

class CSendUserEmail : public CAuthComm
{
public:
	CSendUserEmail(CReqData* pReqData,CResData* pResData)
		: CAuthComm(pReqData,pResData)
	{
	}
	~CSendUserEmail(){};
	virtual int AuthCommit(CReqData* pReqData,CResData* pResData);
	string GetTid()
	{
		return string("send_email");
	}
	void SendEmail();
};

CTrans* CCgi::MakeTransObj()
{
	CSendUserEmail* pTrans = new CSendUserEmail(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif

