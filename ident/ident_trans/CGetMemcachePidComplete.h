#ifndef _CGETMEMCACHEPIDCOMPLETE_H_
#define _CGETMEMCACHEPIDCOMPLETE_H_
#include "CIdentComm.h"

#include <sstream>
#include <vector>
#include <algorithm>

using namespace std;
class CGetMemcachePidComplete: public CIdentComm
{
public:
    CGetMemcachePidComplete(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CGetMemcachePidComplete()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("get_memcache_pid_complete");
	};

	virtual void CheckParameter(CStr2Map& inMap);
};

CTrans* CCgi::MakeTransObj()
{
	CGetMemcachePidComplete* pTrans = new CGetMemcachePidComplete(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
