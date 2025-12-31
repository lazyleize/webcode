#ifndef _CTERMLOGUPLOAD_H_
#define _CTERMLOGUPLOAD_H_

//终端日志上传
#include "CIdentComm.h"

class CTermLogUpload: public CIdentComm
{
public:
	CTermLogUpload(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CTermLogUpload()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("term_log_upload");
	};

private:
	void CheckParameter(CStr2Map& inMap);
};

CTrans* CCgi::MakeTransObj()
{
	CTermLogUpload* pTrans = new CTermLogUpload(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
