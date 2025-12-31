#ifndef CQRYPRODUCERECORDLIST_H_
#define CQRYPRODUCERECORDLIST_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include <base/all.hpp>
#include "CIdentComm.h"

using namespace aps;
class CQryK3AgingOrderList: public CIdentComm
{
public:
    CQryK3AgingOrderList(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CQryK3AgingOrderList()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("qry_k3_aging_order_list");
		return sTid;
	};
	virtual void parseOrderJson(const std::string& jsonString, std::vector<CStr2Map>& vectmapArray);
};

CTrans* CCgi::MakeTransObj()
{
	CQryK3AgingOrderList* pTrans = new CQryK3AgingOrderList(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
