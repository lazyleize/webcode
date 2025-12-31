#ifndef _CACCONTROLLERTRAFFICTREND_H_
#define _CACCONTROLLERTRAFFICTREND_H_
#include "CIdentComm.h"
#include <vector>
#include <string>

using namespace std;

class CAcControllerTrafficTrend: public CIdentComm
{
public:
    CAcControllerTrafficTrend(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CAcControllerTrafficTrend()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("ac_controller_traffic_trend");
	};

private:
};

CTrans* CCgi::MakeTransObj()
{
	CAcControllerTrafficTrend* pTrans = new CAcControllerTrafficTrend(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
