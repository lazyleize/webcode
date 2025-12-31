#ifndef _CSENDPHONEVERICODE_H_
#define _CSENDPHONEVERICODE_H_ 

#include "CAuthComm.h"
#include "CRsaTools.h" 

class CSendPhoneVericode : public CAuthComm
{
public:
	CSendPhoneVericode(CReqData* pReqData,CResData* pResData)
		: CAuthComm(pReqData,pResData)
	{
	}
	~CSendPhoneVericode(){};
	virtual int AuthCommit(CReqData* pReqData,CResData* pResData);
	string GetTid()
	{
		return string("send_phone_vericode");
	}
};

CTrans* CCgi::MakeTransObj()
{
	CSendPhoneVericode* pTrans = new CSendPhoneVericode(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif

