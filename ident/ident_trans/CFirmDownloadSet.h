#ifndef _CFIRMDOWNLIADSET_H_
#define _CFIRMDOWNLIADSET_H_
#include "CIdentComm.h"

class CFirmDownloadSet: public CIdentComm
{
public:
	CFirmDownloadSet(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CFirmDownloadSet()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("firm_download_set");
	};

	virtual void CheckParameter(CStr2Map& inMap);
    virtual bool isValidSVNUrl(const std::string& svn_path, const std::string& username, const std::string& password);
	virtual bool CheckUrl(const std::string& svn_path);
};

CTrans* CCgi::MakeTransObj()
{
	CFirmDownloadSet* pTrans = new CFirmDownloadSet(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
