#ifndef _CACCONTROLLERRESTART_H_
#define _CACCONTROLLERRESTART_H_
#include "CIdentComm.h"
#include <vector>
#include <string>

using namespace std;

class CAcControllerRestart: public CIdentComm
{
public:
    CAcControllerRestart(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CAcControllerRestart()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("ac_controller_restart.cgi");
	};

private:
};

CTrans* CCgi::MakeTransObj()
{
	CAcControllerRestart* pTrans = new CAcControllerRestart(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
