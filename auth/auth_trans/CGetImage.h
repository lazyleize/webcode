#ifndef _CGET_IMAGE_H_
#define _CGET_IMAGE_H_ 

#include "CAuthComm.h"

class CGetImage : public CAuthComm
{
public:
	CGetImage(CReqData* pReqData,CResData* pResData)
		: CAuthComm(pReqData,pResData)
	{
	}
	~CGetImage(){};
	virtual int AuthCommit(CReqData* pReqData,CResData* pResData);
	string GetTid()
	{
		return string("get_image");
	}
};

CTrans* CCgi::MakeTransObj()
{
	CGetImage* pTrans = new CGetImage(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif

