#ifndef _CQUERYATMLOG_H_
#define _CQUERYATMLOG_H_
#include "CIdentComm.h"

class CQueryAtmLog: public CIdentComm
{
public:
	CQueryAtmLog(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CQueryAtmLog()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("download_ate_log");
	};

	virtual void CheckParameter(CStr2Map& inMap);
};

CTrans* CCgi::MakeTransObj()
{
	CQueryAtmLog* pTrans = new CQueryAtmLog(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
  