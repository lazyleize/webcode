#ifndef _CMATERIALREVERSECHECKLIST_H_
#define _CMATERIALREVERSECHECKLIST_H_
#include "CIdentComm.h"

class CMaterialReverseCheckList: public CIdentComm
{
public:
	CMaterialReverseCheckList(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CMaterialReverseCheckList()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("material_reverse_check_list");
	};

	virtual void CheckParameter(CStr2Map& inMap);
    virtual string buildJsonString(int startRow, int limit,const std::string& materialChildValue);
	virtual string buildJsonStringWithValue(const std::string& materialValue);
	virtual void parseMaterialJson(const std::string& jsonString, vector<CStr2Map>& OutMap);
};

CTrans* CCgi::MakeTransObj()
{
	CMaterialReverseCheckList* pTrans = new CMaterialReverseCheckList(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
