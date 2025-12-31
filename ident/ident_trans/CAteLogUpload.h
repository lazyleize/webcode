#ifndef _CATELOGUPLOAD_H_
#define _CATELOGUPLOAD_H_

//射频日志上传
#include "CIdentComm.h"

class CAteLogUpload: public CIdentComm
{
public:
    CAteLogUpload(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CAteLogUpload()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("Ate_log_upload");
	};

private:
	void CheckParameter(CStr2Map& inMap);
};

CTrans* CCgi::MakeTransObj()
{
	CAteLogUpload* pTrans = new CAteLogUpload(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
