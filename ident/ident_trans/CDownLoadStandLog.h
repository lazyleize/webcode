#ifndef _CDOWNLOADSTANDLOG_H_
#define _CDOWNLOADSTANDLOG_H_
#include "CIdentComm.h"

class CDownLoadStandLog: public CIdentComm
{
public:
	CDownLoadStandLog(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CDownLoadStandLog()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("download_stand_log");
	};

	virtual void CheckParameter(CStr2Map& inMap);
};

CTrans* CCgi::MakeTransObj()
{
	CDownLoadStandLog* pTrans = new CDownLoadStandLog(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
