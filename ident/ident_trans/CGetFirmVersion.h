#ifndef _CGETFIRMVERSION_H_
#define _CGETFIRMVERSION_H_
#include "CIdentComm.h"

#include <sstream>
#include <vector>
#include <algorithm>

using namespace std;
class CGetFirmVersion: public CIdentComm
{
public:
	CGetFirmVersion(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CGetFirmVersion()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("get_firm_version");
	};

	virtual void CheckParameter(CStr2Map& inMap);

private:
	vector<string> split(const std::string &s, char delimiter);
	int compareVersionParts(const std::string &v1, const std::string &v2);
	int compareVersions(const std::string &version1, const std::string &version2);
	int compareSpecialParts(const std::string &v1, const std::string &v2) ;
	bool mergeStrings(std::string& strOnlineGo, const std::string& strTermUpdate,vector<std::string>& onlyInOnline);
};

CTrans* CCgi::MakeTransObj()
{
	CGetFirmVersion* pTrans = new CGetFirmVersion(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
