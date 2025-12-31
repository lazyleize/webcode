#ifndef _CGETFIRMDOWNPATH_H_
#define _CGETFIRMDOWNPATH_H_
#include "CIdentComm.h"

class CGetFirmDownPath: public CIdentComm
{
public:
	CGetFirmDownPath(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CGetFirmDownPath()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("get_firm_down_path");
	};

	virtual void CheckParameter(CStr2Map& inMap);
};

CTrans* CCgi::MakeTransObj()
{
	CGetFirmDownPath* pTrans = new CGetFirmDownPath(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
