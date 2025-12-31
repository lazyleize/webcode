#ifndef _CNORMUPLOAD_H_
#define _CNORMUPLOAD_H_

#include "CIdentComm.h"

class CNormUpload: public CIdentComm
{
public:
	CNormUpload(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CNormUpload()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("norm_upload_log");
	};

	virtual void CheckParameter(CStr2Map& inMap);

private:
	int checkVersion(const char* version);
	std::string replaceAll(std::string str, const std::string& from, const std::string& to); 
};

CTrans* CCgi::MakeTransObj()
{
	CNormUpload* pTrans = new CNormUpload(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
