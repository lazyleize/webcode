/************************************************************
 Desc:     AC控制器网络质量统计接口头文件
 Auth:     Auto
 Modify:   2025-12-02
 ***********************************************************/
#ifndef _CACCONTROLLERQUALITY_H_
#define _CACCONTROLLERQUALITY_H_

#include "CIdentComm.h"

class CAcControllerQuality: public CIdentComm
{
public:
    CAcControllerQuality(CReqData *pReqData, CResData *pResData) :
        CIdentComm(pReqData, pResData)
    {
    }
    virtual ~CAcControllerQuality()
    {
    }
    virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
    string GetTid(){
        return string("ac_controller_quality");
    };
};

CTrans* CCgi::MakeTransObj()
{
    CAcControllerQuality* pTrans = new CAcControllerQuality(&m_reqData, &m_resData);
    m_resData.SetOutPutType(OUTPUTJSON);
    m_resData.SetTid(pTrans->GetTid());
    m_resData.SetSubModule("ident");
    return pTrans;
}

#endif

