#ifndef CGETK3PIDEND_H_
#define CGETK3PIDEND_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CIdentComm.h"

class CGetK3PidEnd: public CIdentComm
{
public:
    CGetK3PidEnd(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CGetK3PidEnd()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("get_K3_pid_end");
		return sTid;
	};
    virtual void CheckParameter( CStr2Map& inMap);
};

CTrans* CCgi::MakeTransObj()
{
	CGetK3PidEnd* pTrans = new CGetK3PidEnd(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
