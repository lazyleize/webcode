/*
 * CAuthPub.h 公共函数
 *  Authated on: 2013-4-10
 *      Author: 

 */

#ifndef CAUTHPUB_H_
#define CAUTHPUB_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iconv.h>
#include <string>
#include "auth_err.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "tools/CTrsExp.h"
#include "tools/transxmlcfg.h"
#include "tools/CRsaTools.h"
#include "enc/adsenc.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <iomanip>
#include <fstream>
#include "tcp/sysunix.h"
#include "tcp/adsdef.h"
#include "tcp/adsresp.h"
#include "tcp/adstcp.h"
#include "enc/adsenc.h"
#include "ParseXmlData.h"

//企业经济性质定义
#define ECO_7 "7"//企业性质，个体网店经营
#define ECO_6 "6"//企业性质，个体工商户

#define FILE_ZERO 0 // 文件数量0
#define FILE_ONE 1//文件数量1
#define FILE_TWO 2//文件数量2
#define FILE_THREE 3//文件数量3
#define FILE_FOUR 4//文件数量4

//授信表状态从0到16
#define CTT_STATE_APPLY "0"//客户申请
#define CTT_STATE_PASS "1"//额度审批通过
#define CTT_STATE_WAIT "2"//已发送申请，等第三方回复
#define CTT_STATE_APPROVE1 "3"//一级审批通过,即贷款审批通过，等待签合同
#define CTT_STATE_APPROVE_CONTRACT "4"//担保授信，客户同意合同
#define CTT_STATE_CANCEL "6"//撤销贷款
#define CTT_STATE_ABOLISH "8"//作废
#define CTT_STATE_REFUSE "9"//贷款拒绝
#define CTT_STATE_QUIT "10"//退出
#define CTT_STATE_CHECK_PASS "11"//合规检查通过
#define CTT_STATE_CHECK_FAIL "12"//合规检查失败
#define CTT_STATE_WAIT_QUERYCONTRACT "13"//贷款审批通过，等待查合同
#define CTT_STATE_RUNDATA_PREPARED "14"//经营数据已准备
#define CTT_STATE_CONTRACT_SIGN "15"//合同已签订
#define CTT_STATE_CONTRACT_SENDBANK "16"//合同已发送银行
#define CTT_STATE_97 "97"//中信生成客户信息初始状态
#define CTT_STATE_98 "98"//中信生成经营信息状态
#define CTT_STATE_99 "99"//中信初始状态

#define CONTRACT_TYPE_LOAN "1"//授信类型，贷款授信
#define CONTRACT_TYPE_ASSURE "2"//授信类型，担保授信
#define CONTRACT_TYPE_ASSUER_LOAN "3"//授信类型，担保贷款授信
#define RET_OK "0"
/*
 *cmd 公共更新授信信息时的控制参数
 */
/*
 *更新作废状态
 */
#define  CMD_UPDATE_ABOLISH_STATE "1"
/*
 *更新贷款额度
 */
#define  CMD_UPDATE_CONTRACT_AMOUNT "2"
/*
 *更新已放款状态
 */
#define  CMD_UPDATE_OFFER_STATE "3"

//登录类型
#define LOGIN_TYPE_NAME     1      //会员名登录
#define LOGIN_TYPE_MOBILE   2      //手机登录
#define LOGIN_TYPE_EMAIL    3      //邮箱登录
#define LOGIN_NULL          4      //无用户登录          

//用户安全认证方式
#define CHECK_TYPE_MOBILE   "1"      //手机验证
#define CHECK_TYPE_EMAIL    "2"      //邮箱验证

using namespace std;
namespace CAuthPub
{
    /*------------------------------------------------------------------------
     *Function Name: GetDivDay
     *       Desc: 根据起始日、间隔天数、方向计算结束日
     *      Input: 日期串(YYYY-MM-DD)
     *     Return: 结束日串(YYYY-MM-DD)
     *-----------------------------------------------------------------------*/
    string GetDivDay( const char * start_date, int nday);
    
    /*------------------------------------------------------------------------
     *Function Name: GetXyyDay
     *       Desc: 根据起始日、间隔天数、方向计算结束日
     *      Input: 日期串(YYYY-MM-DD)
     *             间隔天数(XYY): X: 1天 2月 3年 (155 55天 203 3个月 301 一年)
     *     Return: 结束日串(YYYY-MM-DD)
     *-----------------------------------------------------------------------*/
    string GetXyyDay(const  char * start_date, int nday);

    //仅取当前时间 HHMMSS
    string GetCurrentTime();

    //取当前日期时间 YYYYMMDDHHMMSS
    string GetCurrentDateTime();
    
    // 获取当前时间和日期 YYYY-MM-DD HH:MM:SS
    int GetTimeNow(char * str);
    
    // 获取当前日期 YYYY-MM-DD
    string GetDateNow();
    //获取当前日期 YYYYMMDD
    string GetFormatDateNow();

    //返回当前日期YYYYMMDD
    //string GetCurrentDate();
    
    // 生成时间 YYYY-MM-DD HH:MM:SS
    void GetTime(string & strTime);
    /*------------------------------------------------------------------------
     *Function Name: GetDivHourTime 
     *       Desc: 根据起始时间(格式YYYY-MM-DD HH:MM:SS)和间隔小时数算时间
     *      Input: @param sStartTime 日期串(YYYY-MM-DD HH:MM:SS)
     *             @param nHour 间隔小时数:
     *     Return: 结束日串(YYYY-MM-DD HH:MM:SS)
     *-----------------------------------------------------------------------*/
    string GetDivHourTime(const string& sStartTime, int nHour);

    /*------------------------------------------------------------------------
     *Function Name: TranslateDate
     *       Desc: 将YYYY-MM-DD HH:MM:SS等的分隔符去掉，只返回数字
     *      Input: 
     *        srcDate: 类似(YYYY-MM-DD HH:MM:SS)等组合日期串
     *           flag: 1 只返回前8位
     *     Return: 纯数字的结束串 YYYYMMDDHHMMSS等
     *-----------------------------------------------------------------------*/
    string TranslateDate(const string& srcDate,const int flag=0);

    //转码函数,由ChangeCharacterSize调用
    int codeConvert(const char* src_page, const char* dst_page,
                const char* szSourceText, int inLength,
                char* szDestBuff, int bufSize);

    // 转码 
    // 输入参数
    //     strBuf  需要转移的字符，转码后的字符也保存在这里
    //     szFrom  输入的字符集 GB18030 GB2312 UTF-8 
    //     szTo    输出的字符集 GB18030 GB2312 UTF-8 
    // 返回 0成功 -1失败
    static const int MAX_CONVERT_SIZE = 64 * 1024;
    int ChangeCharacterSize(string &strBuf, const char * szFrom, const char * szTo);

    /*------------------------------------------------------------------------
     *Function Name: genBankTransID
     *       Desc: 生成银行流水号
     *      Input: 
     *     Return: 6位银行流水号
     *-----------------------------------------------------------------------*/
    //static  const int BILLNO_BANKTRANSID=251;  //生成银行流水号
    //string genBankTransID();

    /*------------------------------------------------------------------------
     *Function Name: genCftTransID
     *       Desc: 生成财付通流水号
     *      Input: 平台号
     *     Return: 当前日期+平台号+流水号
     *-----------------------------------------------------------------------*/
    //static  const int BILLNO_CFTTRANSID=9;  //生成财付通流水号
    //string genCftTransID(const string&);

    /*------------------------------------------------------------------------
     *Function Name: ChangeContractState
     *       Desc: 根据不同发起人,返回不同状态
     *      Input: 数据库状态,发起人类别
     *     Return: 转换后的状态
     *-----------------------------------------------------------------------*/
    static  const int APPLY_NO_PUB=1;         //中信借款人
    static  const int ASSURE_APPLY_NO_PUB=2;  //担保贷款借款人
    static  const int ASSURE_NO_PUB=3;        //担保贷款担保人
    static  const int LOAN_NO_PUB=4;          //担保贷款贷款人
    string ChangeContractState(const string &state, int strno);

    /*获取配置文件的3DES密钥 cre_3des_key*/
    string GetAuthDesKey();
    void GetDeptAndTeller(string& dept, string& teller);
    string undes3(const string& src_str);
    string undes3_mobile(const string& src_str);
    string des3(const string& src_str);
    /*------------------------------------------------------------------------
     *Function Name: GetDateAfterXYear
     *       Desc: 根据起始日、间隔年数、方向计算结束日
     *             如果MM-DD == 02-29，并且目标年为闰年，则MM-DD=02-29
     *             否则，MM-DD=02-28
     *      start_date: 开始日期串(YYYY-MM-DD)
     *      int nYear   间隔年数
     *     Return: 结束日串(YYYY-MM-DD)
     *-----------------------------------------------------------------------*/
    string GetDateAfterXYear(const string& start_date, int nYear);
    /*------------------------------------------------------------------------
     *       Desc: 对银行卡号进行掩码，显示后4位明文，其他以'*'进行掩码
     *       bank_id :银行卡号
     *     Return: 掩码后卡号 如66666666 ->掩码后(****6666)
     *-----------------------------------------------------------------------*/
    string BankIdMask(const string& bank_id);
    /*
    *函数功能：   读取节点属性值，为空抛出异常
    *param: tid   配置节点名
    *       attrName 要取值的属性节点名 
    *return 属性值
    */
   string GetTransConfigNotEmpty(const string & tid, const string & attrName);
   
   /*
    *函数功能： 读取节点属性值，为空抛出异常
    *param: name  配置文件中<variable>节点name属性值 
    *       attrName 要取值的其他属性名 
    *return 属性值
    */
   string GetVarConfigNotEmpty(const string & name, const string & attrName);
   /*
    *函数功能：打开指定路径的文件
    *   
    */
   int OpenFile(const string & filename, ofstream & os,const ios_base::openmode openmode);

   /************************************************************************
    *  Subroutine: ads_des_idstr_see(option, id, source, sourcelen, dest)   *
    *  Input:                                                               *
    *    unsigned char *option : 选项("D":解密; "E":加密)                   *
    *    unsigned char *id     : 支持(0-39)个选项加解密 0登录密码 1交易密码 *
    *    unsigned char *source : 输入串                                     *
    *    unsigned char *sourcelen : 输入串长度，字符不超过512,16进制<=1024  *
    *    unsigned char *dest   : 输出串 大于输入串长度  ((sourcelen/8)+1)*8 *
    *  Return:                                                              *
    *    返回dest字符串长度  返回-1是错误                                   *    
    *  Description:                                                         *
    *    DES算法 内设40组密钥 对字符串加密解密                              *
    *    (1)加密的时候，输入的是字符串明文，                                *
    *           输出的是16进制的明文,输出长度=(((sourcelen/8)+1)*8)*2       *
    *    (2)解密的时候，输入的是16进制明文，                                *
    *           输出的是字符串明文,输出长度约=sourcelen/2                   *
    *    DES算法 内设40组密钥 对字符串加密解密                              *
    *   Auth: JiaYeHui                                                      *
    *   Date: 2015-04-09                                                    *
    ************************************************************************/
   int ads_des_idstr_see(char *option,int id,char *instr,int instrlen,char *outstr);

    /************************************************************************
    *  Subroutine: ads_creid_check( char *creid)                            *
    *  Input:                                                               *
    *    char *creid: 输入身份证号 必须是18位                               *
    *  Return:                                                              *
    *    返回:1成功 -1错误                                                  *
    *  Description:                                                         *
    *    使用这个函数前，要判断前17位全是数字，身份证长度必须是18           *
    *   Auth: JiaYeHui                                                      *
    *   Date: 2015-09-11                                                    *
    ************************************************************************/
   int ads_creid_check(char *creid);

    /*------------------------------------------------------------------------
     *Function Name:CheckTransTime 
     *       Desc: 
     *             交易时间误差在300秒内
     *      Input: YYYYMMDDHHMMSS
     *       Auth: JiaYeHui
     *       Date: 2018-03- 30                                                
     *-----------------------------------------------------------------------*/
    void CheckTransTime(string &trans_time,const int num_sec=300);

    /*------------------------------------------------------------------------
     *Function Name:WebrsaToAdsdes
     *       Desc: 将前端的RSA密码转为ADS后台DES密码
     *      Input: 
     *            web_pwd:前端密码 urlbase64
     *              n_web:前端第几个秘钥,目前只有2个
     *              n_ads:后台第几个秘钥,目前只有40个[0-39] 0登录密码 1交易密码
     *            num_sec:rsa精确到几秒，默认20秒
     *     Return:ADS后台DES密码
     *       Auth: JiaYeHui
     *       Date: 2018-03-30                                                
     *-----------------------------------------------------------------------*/
    string WebrsaToAdsdes(int n_web,string &web_pwd,int n_ads,const int num_sec=20);

	/*------------------------------------------------------------------------
     *Function Name: parsePubRespJson
     *       Desc: 
     *        将":" json串转成map 单层输出
     *      Input:  
     *        待转码串 {"respDesc":"订单号重复","returnUrl":""}
     *        转码结果Map
     *     Return: -1失败 0成功
     *       Auth: JiaYeHui
     *       Date: 2018-01-28                                                   
     *-----------------------------------------------------------------------*/
     int parsePubRespJson(string &data, CStr2Map& outMap);

	 char* solve(char* dest,const char*src);
};

#endif
