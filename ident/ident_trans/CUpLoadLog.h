#ifndef _CUPLOADLOG_H_
#define _CUPLOADLOG_H_

#include "CIdentComm.h"

class CUpLoadLog: public CIdentComm
{
public:
	CUpLoadLog(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CUpLoadLog()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("upload_log");
	};

private:
	int checkVersion(const char* version);
};

CTrans* CCgi::MakeTransObj()
{
	CUpLoadLog* pTrans = new CUpLoadLog(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
