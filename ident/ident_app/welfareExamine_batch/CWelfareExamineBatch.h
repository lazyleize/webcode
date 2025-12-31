/************************************************************
 Desc:     根据t_jifen_batch_temp明细表发起卡激活交易
 Auth:     leize
 Modify:
 data:     2018-09-20
 ***********************************************************/
#ifndef CWELFAREEXAMINEBATCH_H_
#define CWELFAREEXAMINEBATCH_H_

#include "CIdentAppComm.h"
#include "xml/unicode.h"

class CWelfareExamineBatch
{
    public:
    CWelfareExamineBatch();
    virtual ~CWelfareExamineBatch();
    int Commit(CStr2Map & inMap,CStr2Map &outMap);
    string GetTid(){
        return string("welfare_examine_batch");
    };
 
    void DelMapF(CStr2Map& dataMap,CStr2Map& outMap);
	   bool dealOneBatchChargeInfo(CStr2Map& inMap);//逐条处理批量充值记录

    //生成新的兑换码，兑换密码
    bool getRedeemcodeAndCipher(string& strRedeem_code, string& strExchange_cipher);

    //向用户发送通知短信
				bool SendPhoneNotifyVericode(string strRedeem_code, string strExchange_cipher, string strInitPwd,
				                                              	string strAmount, string strMobile, int iRegFlag);

};

#endif 
