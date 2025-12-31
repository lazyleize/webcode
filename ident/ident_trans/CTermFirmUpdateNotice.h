#ifndef _CRESULTNOTIFY_H_
#define _CRESULTNOTIFY_H_
#include "CIdentComm.h"

class CTermFirmUpdateNotice: public CIdentComm
{
public:
	CTermFirmUpdateNotice(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CTermFirmUpdateNotice()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("term_firm_update_notice");
	};

	vector<string> split(const std::string &s, char delimiter);
	virtual void CheckParameter(CStr2Map& inMap);
	int compareVersions(const std::string &version1, const std::string &version2);
	int compareSpecialParts(const std::string &v1, const std::string &v2) ;
	int compareVersionParts(const std::string &v1, const std::string &v2);
};

CTrans* CCgi::MakeTransObj()
{
	CTermFirmUpdateNotice* pTrans = new CTermFirmUpdateNotice(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
