#ifndef _CDATAACQUISITION_H_
#define _CDATAACQUISITION_H_

#include "CIdentComm.h"

class CDataAcquisition: public CIdentComm
{
public:
	CDataAcquisition(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CDataAcquisition()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("data_acquisition");
	};
	virtual void CheckParameter(CStr2Map& inMap);
	virtual void GetType(CStr2Map& inMap);

private:
	int checkVersion(const char* version);
	void sendToElasticsearch(const string& strUrl,string& inStr,CStr2Map& outMap);
};

CTrans* CCgi::MakeTransObj()
{
	CDataAcquisition* pTrans = new CDataAcquisition(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
