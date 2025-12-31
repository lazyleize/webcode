#ifndef CGETTERMNAMELIST_H_
#define CGETTERMNAMELIST_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CIdentComm.h"

class CGetTermNameList: public CIdentComm
{
public:
    CGetTermNameList(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CGetTermNameList()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("get_term_name_list");
		return sTid;
	};
};

CTrans* CCgi::MakeTransObj()
{
	CGetTermNameList* pTrans = new CGetTermNameList(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
