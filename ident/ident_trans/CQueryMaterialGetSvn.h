#ifndef CQUERYMATERIALGETSVN_H_
#define CQUERYMATERIALGETSVN_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CIdentComm.h"

class CQueryMaterialGetSvn: public CIdentComm
{
public:
    CQueryMaterialGetSvn(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CQueryMaterialGetSvn()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("query_material_get_svn");
		return sTid;
	};
    virtual void CheckParameter( CStr2Map& inMap);
    virtual string buildJsonStringBom(int startRow, int limit,string& materialValue);
    virtual void parseBomNumJson(const std::string& jsonString, string& OutMap);
    virtual string buildJsonStringForBDMaterial(const std::string& materialValue);
    virtual void parseSVNPathJson(const std::string& jsonString, string& OutMap);
    virtual string getFileNameFromUrl(const std::string& url);
};

CTrans* CCgi::MakeTransObj()
{
	CQueryMaterialGetSvn* pTrans = new CQueryMaterialGetSvn(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
