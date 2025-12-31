#ifndef _CTERMCLOSEGO_H_
#define _CTERMCLOSEGO_H_
#include "CIdentComm.h"

class CTermCloseGo: public CIdentComm
{
public:
    CTermCloseGo(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CTermCloseGo()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("term_close_go");
	};

	virtual void CheckParameter(CStr2Map& inMap);
};

CTrans* CCgi::MakeTransObj()
{
	CTermCloseGo* pTrans = new CTermCloseGo(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
