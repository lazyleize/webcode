#ifndef _CFIRMCHECKHASH_H_
#define _CFIRMCHECKHASH_H_
#include "CIdentComm.h"

class CFirmCheckHash: public CIdentComm
{
public:
	CFirmCheckHash(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CFirmCheckHash()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("firm_check_hash");
	};

	virtual void CheckParameter(CStr2Map& inMap);
};

CTrans* CCgi::MakeTransObj()
{
	CFirmCheckHash* pTrans = new CFirmCheckHash(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
