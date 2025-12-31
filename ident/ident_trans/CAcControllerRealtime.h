#ifndef _CACCONTROLLERREALTIME_H_
#define _CACCONTROLLERREALTIME_H_
#include "CIdentComm.h"
#include <vector>
#include <string>

using namespace std;

class CAcControllerRealtime: public CIdentComm
{
public:
    CAcControllerRealtime(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CAcControllerRealtime()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("ac_controller_realtime");
	};

private:
};

CTrans* CCgi::MakeTransObj()
{
	CAcControllerRealtime* pTrans = new CAcControllerRealtime(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
