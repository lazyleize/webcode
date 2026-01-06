/************************************************************
 Desc:     AC控制器Trap告警信息列表接口头文件
 Modify:   2025-12-08
 ***********************************************************/
 #ifndef _CACTRAPMESSAGELIST_H_
 #define _CACTRAPMESSAGELIST_H_
 
 #include "CIdentComm.h"
 
 class CAcTrapMessageList: public CIdentComm
 {
 public:
    CAcTrapMessageList(CReqData *pReqData, CResData *pResData) :
         CIdentComm(pReqData, pResData)
     {
     }
     virtual ~CAcTrapMessageList()
     {
     }
     virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
     string GetTid(){
         return string("ac_trap_message_list");
     };
 };
 
 CTrans* CCgi::MakeTransObj()
 {
     CAcTrapMessageList* pTrans = new CAcTrapMessageList(&m_reqData, &m_resData);
     m_resData.SetOutPutType(OUTPUTJSON);
     m_resData.SetTid(pTrans->GetTid());
     m_resData.SetSubModule("ident");
     return pTrans;
 }
 
 #endif
 
 