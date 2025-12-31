//查询全部关联记录
#ifndef CQUERYOVERALLLIST_H_
#define CQUERYOVERALLLIST_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CIdentComm.h"

class CQueryOverallList: public CIdentComm
{
public:
    CQueryOverallList(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CQueryOverallList()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("query_overall_list");
		return sTid;
	};
    void CheckParameter(CStr2Map& inMap);
    string GetStageType(string& strtype, CStr2Map& termProOutMap);
};

CTrans* CCgi::MakeTransObj()
{
	CQueryOverallList* pTrans = new CQueryOverallList(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
