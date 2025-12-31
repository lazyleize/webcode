#ifndef CUPDATEMOBILE_H_
#define CUPDATEMOBILE_H_

#include "CAuthComm.h"

class CUpdateMobile : public CAuthComm
{
public:
	CUpdateMobile(CReqData* pReqData,CResData* pResData)
		: CAuthComm(pReqData,pResData)
	{
	}
	~CUpdateMobile(){};
	virtual int AuthCommit(CReqData* pReqData,CResData* pResData);
	string GetTid()
	{
		return string("update_mobile");
	}
};

CTrans* CCgi::MakeTransObj()
{
	CUpdateMobile* pTrans = new CUpdateMobile(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif

