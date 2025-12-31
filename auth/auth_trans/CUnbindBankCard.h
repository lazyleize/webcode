#ifndef CUNBINDBANKCARD_H_ 
#define CUNBINDBANKCARD_H_ 

#include "CAuthComm.h"
#include "CRsaTools.h" 

class CUnbindBankCard : public CAuthComm
{
public:
	CUnbindBankCard(CReqData* pReqData,CResData* pResData)
		: CAuthComm(pReqData,pResData)
	{
	}
	~CUnbindBankCard(){};
	virtual int AuthCommit(CReqData* pReqData,CResData* pResData);
	string GetTid()
	{
		return string("unbind_bank_card");
	}
};

CTrans* CCgi::MakeTransObj()
{
	CUnbindBankCard* pTrans = new CUnbindBankCard(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif

