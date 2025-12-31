#ifndef _CUPDATEFACTORY_H_
#define _CUPDATEFACTORY_H_

#include "CIdentComm.h"

class CUpdateFactory: public CIdentComm
{
public:
	CUpdateFactory(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CUpdateFactory()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("update_factory");
	};
	virtual void CheckParameter(CStr2Map& inMap);

private:
	int checkVersion(const char* version);
};

CTrans* CCgi::MakeTransObj()
{
	CUpdateFactory* pTrans = new CUpdateFactory(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
