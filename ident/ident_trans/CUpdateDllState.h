#ifndef _CUPDATEDLLSTATE_H_
#define _CUPDATEDLLSTATE_H_

#include "CIdentComm.h"

class CUpdateDllState: public CIdentComm
{
public:
	CUpdateDllState(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CUpdateDllState()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("update_dll_state");
	};
	virtual void CheckParameter(CStr2Map& inMap);

private:
	int checkVersion(const char* version);
};

CTrans* CCgi::MakeTransObj()
{
	CUpdateDllState* pTrans = new CUpdateDllState(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
