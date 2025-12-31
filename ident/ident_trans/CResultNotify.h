#ifndef _CRESULTNOTIFY_H_
#define _CRESULTNOTIFY_H_
#include "CIdentComm.h"

class CResultNotify: public CIdentComm
{
public:
	CResultNotify(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CResultNotify()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("result_notify");
	};

	virtual void CheckParameter(CStr2Map& inMap);
};

CTrans* CCgi::MakeTransObj()
{
	CResultNotify* pTrans = new CResultNotify(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
