#ifndef CGetDateTime_H_
#define CGetDateTime_H_

#include "CAuthComm.h"

class CGetDateTime : public CAuthComm
{
public:
	CGetDateTime(CReqData* pReqData,CResData* pResData)
		: CAuthComm(pReqData,pResData)
	{
	}
	~CGetDateTime(){};
	virtual int AuthCommit(CReqData* pReqData,CResData* pResData);
	string GetTid()
	{
		return string("get_datetime");
	}
};

CTrans* CCgi::MakeTransObj()
{
	CGetDateTime* pTrans = new CGetDateTime(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif

