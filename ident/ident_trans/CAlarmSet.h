#ifndef _CALARMSET_H_
#define _CALARMSET_H_
#include "CIdentComm.h"

class CAlarmSet: public CIdentComm
{
public:
    CAlarmSet(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CAlarmSet()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("alarm_set");
	};

private:
	void CheckParameter(CStr2Map& inMap);
};

CTrans* CCgi::MakeTransObj()
{
	CAlarmSet* pTrans = new CAlarmSet(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
