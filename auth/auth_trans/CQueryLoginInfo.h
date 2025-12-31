#ifndef CQUERYLOGININFO_H_
#define CQUERYLOGININFO_H_

#include "CAuthComm.h"

class CQueryLoginInfo : public CAuthComm
{
public:
	CQueryLoginInfo(CReqData* pReqData,CResData* pResData)
		: CAuthComm(pReqData,pResData)
	{
	}
	~CQueryLoginInfo(){};
	virtual int AuthCommit(CReqData* pReqData,CResData* pResData);
	string GetTid()
	{
		return string("query_login_info");
	}
};

CTrans* CCgi::MakeTransObj()
{
	CQueryLoginInfo* pTrans = new CQueryLoginInfo(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif

