#ifndef CQUERY_REGISTER_INFO_H_
#define CQUERY_REGISTER_INFO_H_

#include "CAuthComm.h"

class CQueryRegisterInfo : public CAuthComm
{
public:
	CQueryRegisterInfo(CReqData* pReqData,CResData* pResData)
		: CAuthComm(pReqData,pResData)
	{
	}
	~CQueryRegisterInfo(){};
	virtual int AuthCommit(CReqData* pReqData,CResData* pResData);
	string GetTid()
	{
		return string("query_register_info");
	}
};

CTrans* CCgi::MakeTransObj()
{
	CQueryRegisterInfo* pTrans = new CQueryRegisterInfo(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif

