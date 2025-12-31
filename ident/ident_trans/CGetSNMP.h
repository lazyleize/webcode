#ifndef _CGETSNMP_H_
#define _CGETSNMP_H_
#include "CIdentComm.h"

class CGetSnmp: public CIdentComm
{
public:
	CGetSnmp(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CGetSnmp()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("getsnmp");
	};
};

CTrans* CCgi::MakeTransObj()
{
	CGetSnmp* pTrans = new CGetSnmp(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
