#ifndef _CSETMESSAGEUSE_H_
#define _CSETMESSAGEUSE_H_
#include "CIdentComm.h"

class CSetMessageUse: public CIdentComm
{
public:
    CSetMessageUse(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CSetMessageUse()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("set_message_use");
	};

	virtual void CheckParameter(CStr2Map& inMap);
};

CTrans* CCgi::MakeTransObj()
{
	CSetMessageUse* pTrans = new CSetMessageUse(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
