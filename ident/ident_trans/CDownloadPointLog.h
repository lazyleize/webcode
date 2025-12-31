#ifndef _CDOWNLOADPOINTLOG_H_
#define _CDOWNLOADPOINTLOG_H_
#include "CIdentComm.h"

class CDownloadPointLog: public CIdentComm
{
public:
    CDownloadPointLog(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CDownloadPointLog()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("download_point_log");
	};

	virtual void CheckParameter(CStr2Map& inMap);
};

CTrans* CCgi::MakeTransObj()
{
	CDownloadPointLog* pTrans = new CDownloadPointLog(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
