#include "CSendPhoneVericode.h"
#include "CAuthRelayApi.h"

int CSendPhoneVericode::AuthCommit(CReqData* pReqData,CResData* pResData)
{
        CStr2Map inMap,outMap,sessMap;
        
        //取得所有的请求参数
        pReqData->GetStrMap(inMap);
        /*add---by---zhangwuyu---20150911--begin---*/ 
        if(inMap["op_stat"] == "1")
        {
            // 用程序判断，前端有传来vm
            if(inMap["vm"].empty())
            {
                ErrorLog("vm null [%s][%s][%s]",inMap["mobile"].c_str(),inMap["v"].c_str(),inMap["name"].c_str());
                sleep(1);
                //throw CTrsExp(ERR_VMDECRYPT_NOT_VALID,"IP不合法");
                //throw CTrsExp("0000","OK");
                return 0;
            }


            //asd123,asd423 屏蔽这2个用户的发起
            if(!inMap["name"].empty())
            {
               int ilen = inMap["name"].length();
               if(ilen == 6)
               {
                   if((memcmp(inMap["name"].c_str(),"asd423",6) == 0) || (memcmp(inMap["name"].c_str(),"asd123",6) == 0))
                   {
                       ErrorLog("vm name [%s][%s][%s]",inMap["mobile"].c_str(),inMap["v"].c_str(),inMap["name"].c_str());
                       usleep(1100000);
                       return 0;
                   }
               }
               else if(ilen < 5)
               {
                   ErrorLog("vm name less 5 [%s][%s][%s]",inMap["mobile"].c_str(),inMap["v"].c_str(),inMap["name"].c_str());
                   usleep(1500000);
                   return 0;
               }
               //要判断用户名非全数字
               if(ads_isnum(const_cast<char*>(inMap["name"].c_str())))
               {
                   ErrorLog("vm name fault [%s][%s][%s]",inMap["mobile"].c_str(),inMap["v"].c_str(),inMap["name"].c_str());
                   usleep(1500000);
                   return 0;
               }
            }
            

            string vm_decrypt = CAuthPub::undes3_mobile(inMap["vm"]);
            //if(!ads_isnum(const_cast<char*>(vm_decrypt.c_str())))
            //{
            //     ErrorLog("vm isnot num: vm_undes=[%s][%s][%s][%s]",vm_decrypt.c_str(),inMap["mobile"].c_str(),inMap["v"].c_str(),inMap["name"].c_str());
            //     usleep(1500000);
            //     //sleep(15);
            //     return 0;
            //}
            
            string mobile_v_combination = inMap["mobile"] + inMap["code"].substr(inMap["code"].length() - 7,6);
            if(vm_decrypt != mobile_v_combination)
            {
                 ErrorLog("vm diff: vm_undes=[%s],vm_must=[%s][%s][%s][%s]",vm_decrypt.c_str(),mobile_v_combination.c_str(),inMap["mobile"].c_str(),inMap["code"].c_str(),inMap["name"].c_str());
                 usleep(1500000);
                 //sleep(15);
                 return 0;
            }
            
        } 
        else
        {
            //检查用户登录态
            CheckLogin(sessMap);
            inMap["name"]=sessMap["uin"];
            inMap["uid"] = sessMap["uid"];
            inMap["uin"] = sessMap["uin"];

            //有交易密码 要验证交易密码
            if(!inMap["in_tran_pwd"].empty())
            {
                /*
                string tranpwd_open = CAuthPub::undes3(inMap["in_tran_pwd"]);
                ErrorLog("交易密码: tranpwd_open=[%s]",tranpwd_open.c_str());
            
                char des_open[64];
                char des_enc[64];
                memset(des_open,0x00,sizeof(des_open));
                memset(des_enc,0x00,sizeof(des_enc));
                strcpy(des_open,tranpwd_open.c_str());
                CAuthPub::ads_des_idstr_see("E",1,des_open,strlen(des_open),des_enc);
                inMap["pwd_tran"] = des_enc;
                */
                inMap["pwd_tran"] = CAuthPub::WebrsaToAdsdes(1,inMap["in_tran_pwd"],1,TIMEOUT_RSA);
                CAuthRelayApi::CheckTranPwd(inMap,outMap,true);
            }
        }
        /*add---by---zhangwuyu---20150911--end---*/ 
        //查询IP限制表
        /*
        inMap["loading_ip"] = pReqData->GetEnv("ClientIp");
        CAuthRelayApi::QueryAuthIPInfo(inMap["loading_ip"],outMap,true);
        if("0" != outMap["Ftotal_num"])
        {
            ErrorLog("用户IP:[%s]尊敬的客户，您的行为被系统判断为恶意操作，请停止攻击。如果造成投资人的损失，将追究您的责任。", inMap["loading_ip"].c_str());
            sleep(10);
            return 0;
            //throw CTrsExp(ERR_USER_LIMIT_IP, "尊敬的客户，您的行为被系统判断为恶意操作，请停止攻击。如果造成投资人的损失，将追究您的责任。");
            //throw CTrsExp("0000","OK");
        }
        */

        CAuthRelayApi::SendPhoneVericode(inMap,outMap,true);
    
        return 0;
}
