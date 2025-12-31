#ifndef CQUERYUSERRECOMMENDLIST_H_
#define CQUERYUSERRECOMMENDLIST_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CAuthPub.h"
#include "CAuthComm.h"

class CQueryUserRecommendList: public CAuthComm
{
public:
	CQueryUserRecommendList(CReqData *pReqData, CResData *pResData) :
		CAuthComm(pReqData, pResData)
	{
	}
	virtual ~CQueryUserRecommendList()
	{
	}
	virtual int AuthCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("query_auth_recommend");
		return sTid;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CQueryUserRecommendList* pTrans = new CQueryUserRecommendList(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif
