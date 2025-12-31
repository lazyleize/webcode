#ifndef _CMODIFYFACTORY_H_
#define _CMODIFYFACTORY_H_
#include "CIdentComm.h"

class CModifyFactory: public CIdentComm
{
public:
	CModifyFactory(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CModifyFactory()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("modify_factory");
	};

	virtual void CheckParameter(CStr2Map& inMap);
};

CTrans* CCgi::MakeTransObj()
{
	CModifyFactory* pTrans = new CModifyFactory(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
