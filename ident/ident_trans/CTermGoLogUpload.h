#ifndef _CTERMGOLOGUPLOAD_H_
#define _CTERMGOLOGUPLOAD_H_

//终端日志上传
#include "CIdentComm.h"

class CTermGoLogUpload: public CIdentComm
{
public:
    CTermGoLogUpload(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CTermGoLogUpload()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("term_go_log_upload");
	};

private:
	void CheckParameter(CStr2Map& inMap);
};

CTrans* CCgi::MakeTransObj()
{
	CTermGoLogUpload* pTrans = new CTermGoLogUpload(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
