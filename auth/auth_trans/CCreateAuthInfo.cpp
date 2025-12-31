#include "CCreateAuthInfo.h"
#include "CAuthRelayApi.h"
#include "CAuthPub.h"
#include "adstcp.h"

#define MAX_FAIL_NUM 5

int CCreateAuthInfo::AuthCommit(CReqData* pReqData,CResData* pResData)
{
    CStr2Map inMap,outMap,returnMap,sessMap;
    pReqData->GetStrMap(inMap);

        /*vm    varchar(64)    M    对身份证号10-17位加密3des*/
        if(inMap["vm"].empty())
        {
                ErrorLog("vm null id_no=[%s][%s][%s]",inMap["id_no"].c_str(),inMap["v"].c_str(),inMap["name"].c_str());
                sleep(20);
                return 0;
        }
        string vm_decrypt = CAuthPub::undes3_mobile(inMap["vm"]);
        if(!ads_isnum(const_cast<char*>(vm_decrypt.c_str())))
        {
                ErrorLog("字段须全为数字: vm_decrypt=[%s][%s][%s]",vm_decrypt.c_str(),inMap["v"].c_str(),inMap["name"].c_str());
                sleep(10);
                return 0;
        }
        string id_no_cut = inMap["id_no"].substr(inMap["id_no"].length() - 9,8);
        if(vm_decrypt != id_no_cut)
        {
                ErrorLog("身份证传输有误,vm_decrypt=[%s],id_no_cut=[%s][%s][%s]",vm_decrypt.c_str(),id_no_cut.c_str(),inMap["v"].c_str(),inMap["name"].c_str());
                sleep(10);
                return 0;
        }
        /*---add---by---zhangwuyu---20150916---end---*/

    //检查用户登录态
    /*--JiaYeHui --- 20151104 --- 从cess获取UID及UIN --*/
    CheckLogin(sessMap);
    inMap["name"] = sessMap["uin"];
    /*
    int iCount = FILE_ZERO;
    if(!inMap["id_addr_a"].empty() && !inMap["id_addr_b"].empty())
    {
        iCount = FILE_TWO;
    }
    else if(!inMap["id_addr_a"].empty())
    {
        iCount = FILE_ONE;
    }
    else if(!inMap["id_addr_b"].empty())
        {
        iCount = FILE_ONE;
        }

    //保存用户上传的文件，将文件名存入map
    if(iCount != FILE_ZERO)
    {
        CAuthComm::UploadFile(pReqData,inMap,iCount);
        inMap["id_addr_a"] = inMap["id_addr_a_"];
        inMap["id_addr_b"] = inMap["id_addr_b_"];
    }
    */
        /* JiaYeHui
            1 先判断长度一定要是18位，前17位是数字，最后一位是x或者X
            2 ads_creid_check 判断输入身份证号是否正确
            3 姓名：去掉左右空格，判断输入的姓名有没有空格，有数字,英文 + ,的要返回错误
        */
        /*---add---by---zhangwuyu---20150911---begin---*/
        if(inMap["id_no"].length() != 18)                   //先判断长度一定要是18位，前17位是数字，最后一位是x或者X
        {
            ErrorLog("身份证号码必须是18位,且此身份证号码=[%s]",inMap["id_no"].c_str());
            throw CTrsExp(ERR_USER_HAS_ID_NO,"身份证号码输入有误");
        }
        if(!ads_isnum(const_cast<char*>(inMap["id_no"].substr(0,17).c_str())))
        {
            ErrorLog("身份证号码的前17位必须均是数字,且此身份证号码的前17位=[%s]",inMap["id_no"].substr(0,17).c_str());
            throw CTrsExp(ERR_USER_HAS_ID_NO,"身份证号码输入有误");
        }
        if(!ads_isnum(const_cast<char*>(inMap["id_no"].substr(17,1).c_str())) && inMap["id_no"].substr(17,1) != "X" && inMap["id_no"].substr(17,1) != "x")
        {
            ErrorLog("身份证号码的第18位必须是数字或X或x,且此身份证号码=[%s]",inMap["id_no"].c_str());
            throw CTrsExp(ERR_USER_HAS_ID_NO,"身份证号码输入有误");
        }
        if(inMap["id_no"].substr(17,1) == "x")
        {
            inMap["id_no"] = inMap["id_no"].substr(0,17) + "X";
            DebugLog("小写的x转化为X后的身份证号码=[%s]",inMap["id_no"].c_str());
        }
        if(CAuthPub::ads_creid_check(const_cast<char*>(inMap["id_no"].c_str())) == -1)    //判断输入身份证号是否正确
        {
            ErrorLog("身份证号码格式有错误,且此身份证号码=[%s]",inMap["id_no"].c_str());
            throw CTrsExp(ERR_USER_HAS_ID_NO,"身份证号码输入有误");
        }


        char real_name_tmp[64+1];
        memset(real_name_tmp,0x00,sizeof(real_name_tmp));
        strcpy(real_name_tmp,inMap["real_name"].c_str());
        DebugLog("从real_name拷贝而来的real_name_tmp=[%s]",real_name_tmp);
        ads_ltrim(real_name_tmp);            //去掉左空格
        ads_rtrim(real_name_tmp);            //去掉右空格
        DebugLog("去掉左右空格以后的的real_name_tmp=[%s]",real_name_tmp);
        inMap["real_name"] = real_name_tmp;
        DebugLog("去掉左右空格以后的real_name=[%s]",inMap["real_name"].c_str());
        /*---add---by---zhangwuyu---20150911---end---*/
    //检查用户身份证是否存在    
    outMap["uid"] = CAuthRelayApi::QueryUidByIDNumber(inMap["id_no"],true);
    if(!outMap["uid"].empty())
    {
            ErrorLog("该身份证号码已被绑定");
                throw CTrsExp(ERR_USER_HAS_ID_NO,"该身份证号码已被绑定");
    }
    
    CAuthRelayApi::UpdateAuthInfo(inMap,outMap,true);
    CStr2Map InMap,OutMap;    
    InMap["id_no"] = Tools::rtrim(inMap["id_no"]);
    InMap["real_name"] = inMap["real_name"];
    
    inMap["op_type"] = "1";//实名成功
    /*
    //廖斌注释，等四要素接口有了，重新接入
    CAuthRelayApi::CheckUserIdentity(InMap,OutMap,false);
    if(OutMap["errorcode"] != "0000")
    {
            DebugLog("[%s:%d] 用户实名失败",__FILE__,__LINE__);
        inMap["op_type"] = "2";//实名失败
    }
    else
    {
            DebugLog("[%s:%d] 用户实名成功",__FILE__,__LINE__);
        inMap["op_type"] = "1";//实名成功
    }
    */
    //保存请求数据
    //CAuthRelayApi::CreateAuthInfo(inMap,outMap,true);

    return 0;
}
