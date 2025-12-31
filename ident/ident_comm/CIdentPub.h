/*
 * CIdentPub.h 公共函数
 *  Identated on: 2013-4-10
 */

#ifndef CJIFENPUB_H
#define CJIFENPUB_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iconv.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include "ident_err.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "tools/CTrsExp.h"
#include "tools/transxmlcfg.h"
#include "tools/CRsaTools.h"
#include "tools/CDesTools.h"
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


namespace CIdentPub
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
    
    // 获取divsec秒间隔的时间和日期 YYYY-MM-DD HH:MM:SS
    int GetTimeNowDivSec(char * str,int divsec);
    // 获取当前日期 YYYY-MM-DD
    string GetDateNow();
    //获取当前日期 YYYYMMDD
    string GetFormatDateNow();

    //取当前的时间格式YYYYMMDD HH:MM:SS
    string GetFormatTimeNow();

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
    string GetIdentDesKey();
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
    *   Ident: JiaYeHui                                                      *
    *   Date: 2015-04-09                                                    *
    ************************************************************************/
   int ads_des_idstr_see(char *option,int id,char *instr,int instrlen,char *outstr);

   /*
    *函数功能：根据系统号获取签名key
    *   
    */
   int GetSignatureKey(string& syscode, string& sig_key);

   /*------------------------------------------------------------------------
    *  Function Name: Convertrate
    *         Desc: 费率转换，将传入的数字除以10万
    *        Input: 费率，如千分之3.8，则值为380
    *       Return: 返回小数点表示的数字 0.0038
    *-----------------------------------------------------------------------*/
   string Convertrate(const string& sNum);

    /*------------------------------------------------------------------------
    *  Function Name: HEX_TO_ASCII
    *         Desc: 将两个16进制字符转换为1个ASCII码字符
    *        Input: 
    *       Return: 
    *-----------------------------------------------------------------------*/
   std::string HEX_TO_ASCII(char* ascii, int asc_len);
   char hex_to_asc(char asc);

   /*------------------------------------------------------------------------
    *  Function Name: ASCII_TO_HEX
    *         Desc: 将1个ASCII码字符转换为2个16进制字符
    *        Input: 
    *       Return: 
    *-----------------------------------------------------------------------*/
   std::string ASCII_TO_HEX(char *bytes,int len);


    /*------------------------------------------------------------------------
    *  Function Name: RsaPublicEncrypt
    *         Desc: RSA公钥加密
    *  Input:                                                               *
    *    unsigned char *data    : 待加密数据                                *
    *    int data_len           : 待加密数据长度                            *
    *    unsigned char *key     : 公钥                                      *
    *    std::string& encrypted : 加密结果                                  *
    *  Return:返回加密结果数据长度 -1 失败                                  *
    *-----------------------------------------------------------------------*/    
   int RsaPublicEncrypt(unsigned char * data,int data_len,unsigned char * key, std::string& encrypted);
   
 
    /*------------------------------------------------------------------------
    *  Function Name: RsaPrivateEncrypt
    *         Desc: RSA公钥加密
    *  Input:                                                               *
    *    unsigned char *data    : 待加密数据                                *
    *    int data_len           : 待加密数据长度                            *
    *    unsigned char *key     : 私钥                                      *
    *    std::string& encrypted : 加密结果                                  *
    *  Return:返回加密结果数据长度 -1 失败                                  *
    *-----------------------------------------------------------------------*/     
   int RsaPrivateEncrypt(unsigned char * data,int data_len,unsigned char * key, std::string& encrypted);

    
    /*------------------------------------------------------------------------
    *  Function Name: RsaPrivateDecrypt
    *         Desc: RSA私钥揭秘额
    *  Input:                                                               *
    *    unsigned char *data    : 待解密密文                                *
    *    int data_len           : 待解密密文长度                            *
    *    unsigned char *key     : 私钥                                      *
    *    std::string& encrypted : 解密结果                                  *
    *  Return:返回解密结果数据长度 -1 失败                                  *
    *-----------------------------------------------------------------------*/   
   int RsaPrivateDecrypt(unsigned char * enc_data,int data_len,unsigned char * key, std::string& decrypted);


    /*------------------------------------------------------------------------
    *  Function Name: RsaPublicDecrypt
    *         Desc: RSA公钥揭秘额
    *  Input:                                                               *
    *    unsigned char *data    : 待解密密文                                *
    *    int data_len           : 待解密密文长度                            *
    *    unsigned char *key     : 公钥                                      *
    *    std::string& encrypted : 解密结果                                  *
    *  Return:返回解密结果数据长度 -1 失败                                  *
    *-----------------------------------------------------------------------*/     
   int RsaPublicDecrypt(unsigned char * enc_data,int data_len,unsigned char * key, std::string& decrypted);


   /*------------------------------------------------------------------------
   *  Function Name: RsaPayDecrypt
   *         Desc: RSA公钥揭秘额
   *  Input:                                                               *
   *    unsigned char *data    : 待解密密文                                *
   *    int data_len           : 待解密密文长度                            *
   *    unsigned char *key     : 公钥                                      *
   *    std::string& encrypted : 解密结果                                  *
   *  Return:返回解密结果数据长度 -1 失败                                  *
   *-----------------------------------------------------------------------*/     
   int RsaPayDecrypt(unsigned char * enc_data,int data_len,std::string& decrypted);


    /*------------------------------------------------------------------------
    *  Function Name: HttpPost
    *         Desc: http post 方法
    *  Input:                                                               *
    *    std::string & strUrl    : URL地址                                  *
    *    std::string & strPost   : post数据                                 *
    *    std::string& strResponse  : 响应数据                               *
    *  Return: -1 失败  0 成功                                              *
    *-----------------------------------------------------------------------*/  
   int HttpPost(const std::string & strUrl, const std::string & strPost, std::string & strResponse); 
   int HttpPostXml(const std::string & strUrl, const std::string & strPost, std::string & strResponse);
   int HttpPostCUC(const std::string & strUrl, const std::string & strPost, std::string & strResponse);
   int HttpPostBusiness(const std::string & strUrl, const std::string & strPost, std::string & strResponse);
   int HttpESPost(const std::string & strUrl, const std::string & strPost, std::string & strResponse); 
   int HttpERPtoken(const std::string &strUrl, const std::string &strName, const std::string &strPasswd, std::string &strToken); 
   int HttpPostERP(const std::string & strUrl, const std::string & strPost, std::string & strResponse,const string &strToken);
    /*------------------------------------------------------------------------
    *  Function Name: HttpGet
    *         Desc: http get 方法
    *  Input:                                                               *
    *    std::string & strUrl    : URL地址                                  *
    *    std::string& strResponse  : 响应数据                               *
    *  Return: -1 失败  0 成功                                              *
    *-----------------------------------------------------------------------*/  
   int HttpGet(const std::string & strUrl, std::string & strResponse);

    /*------------------------------------------------------------------------
    *  Function Name: HttpsPost
    *         Desc: http ssl post 方法
    *  Input:                                                               *
    *    std::string & strUrl    : URL地址                                  *
    *    std::string & strPost   : post数据                                 *
    *    std::string& strResponse  : 响应数据                               *
    *    std::string& pCaPath      : 证书路径                               *
    *  Return: -1 失败  0 成功                                              *
    *-----------------------------------------------------------------------*/  
   int HttpsPost(const std::string & strUrl, const std::string & strPost, std::string & strResponse, const char * pCaPath);  

    /*------------------------------------------------------------------------
    *  Function Name: HttpsPost2
    *         Desc: http ssl post 方法
    *  Input:                                                               *
    *    std::string & strUrl    : URL地址                                  *
    *    std::string & strPost   : post数据                                 *
    *    std::string& strResponse  : 响应数据                               *
    *    std::string& pCaPath      : 证书路径                               *
    *  Return: -1 失败  0 成功                                              *
    *-----------------------------------------------------------------------*/  
   int HttpsPost2(const std::string & strUrl, const std::string & strPost, std::string & strResponse, const char * pCaPath);  

    /*------------------------------------------------------------------------
    *  Function Name: HttpsGet
    *         Desc: http ssl get 方法
    *  Input:                                                               *
    *    std::string & strUrl    : URL地址                                  *
    *    std::string& strResponse  : 响应数据                               *
    *    std::string& pCaPath      : 证书路径                               *
    *  Return: -1 失败  0 成功                                              *
    *-----------------------------------------------------------------------*/  
   int HttpsGet(const std::string & strUrl, std::string & strResponse, const char * pCaPath);


    /*------------------------------------------------------------------------
    *  Function Name: GetDESKey
    *         Desc:根据系统号获取DES秘钥
    *  Input:                                                               *
    *    std::string syscode    : 系统码                                     *
    *  Return: -1 失败  0 成功                                              *
    *-----------------------------------------------------------------------*/  
    int GetDESKey(string& syscode, string& des_key);

    /*------------------------------------------------------------------------
    *  Function Name: GetRsaPublicKey
    *         Desc:根据系统号获取rsa公钥
    *  Input:                                                               *
    *    std::string syscode    : 系统码                                     *
    *  Return: -1 失败  0 成功                                              *
    *-----------------------------------------------------------------------*/  
    int GetRsaPublicKey(string& syscode, string& rsa_key);
    int RSA_MD5_DEC(string &strRsaPubKey,string &strMD5Key,string &signature,string &instr);
    int RSA_MD5_ENC(string &strRsaPubKey,string &strMD5Key,string &instr,string &signature);

    /*------------------------------------------------------------------------
    *  Function Name: CheckSyscode
    *         Desc:校验系统码
    *  Input:                                                               *
    *    std::string syscode    : 系统码                                     *
    *  Return: -1 失败  0 成功                                              *
    *-----------------------------------------------------------------------*/  
    int CheckSyscode(std::string &syscode);
    
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
    int ads_creid_check( char *creid);


    /********************************************************************************
    *   Func: ads_datetime_check(string &time_beg,string &time_end,int nday,int div_day)
    *  Input:                                                                       
    *    time_beg: 输入的起始日期时间 YYYY-MM-DD HH:MM:SS 返回时会覆盖新值
    *    time_end: 输入的截止日期时间 YYYY-MM-DD HH:MM:SS 返回时会覆盖新值
    *    nday   : 当为空值时，默认日期时间 0则当天 -1昨天
    *    div_day : 时间跨度，起始日期和截止日期的范围天数 
    *  Description:                                                         
    *    (1)判断起始和截止时间是否正确，格式错误抛出异常
    *    (2)判断起始和截止时间日期范围是否正确，超出范围抛出异常
    *    (3)当前端传来空值时，按nday要求的规则，重新生成时间
    *  Return:                                                              
    *    返回: 无
    *   Auth: JiaYeHui                                                      
    *   Date: 2017-01- 08                                                   
    ********************************************************************************/
    void ads_datetime_check(string &time_beg,string &time_end,int nday,int div_day);

    int PayNotify(CStr2Map& paramap);
    int parseApiJson(std::string &data, CStr2Map& outMap);
    int OPayNotify(CStr2Map& paramap);
    int OPayNotifyM(CStr2Map& paramap);

    void DelMapF(CStr2Map& dataMap,CStr2Map& outMap);


    /*----------------------------------------------------------------------
    *  Function Name: CheckUid
    *         Desc:校验手机号的有效性
    *  Input:                                                               *
    *    std::string uid    : 商户号                                        *
    *  Return: -1 失败  0 成功                                              *
    *-----------------------------------------------------------------------*/  
    int CheckUid(string uid);

    /*----------------------------------------------------------------------
    *  Function Name: CheckPhone
    *         Desc:校验手机号的有效性
    *  Input:                                                               *
    *    std::string phone    : 手机号                                      *
    *  Return: -1 失败  0 成功                                              *
    *-----------------------------------------------------------------------*/  
    int CheckPhone(string phone);
    
    /*----------------------------------------------------------------------
    *  Function Name: CheckTransID
    *         Desc:校验流水号的有效性
    *  Input:                                                               *
    *    std::string trans_id    : 流水号                                   *
    *  Return: 抛异常
    *-----------------------------------------------------------------------*/  
    void CheckTransID(string trans_id);


    /*----------------------------------------------------------------------
    *  Function Name: CheckContractID
    *         Desc:校验签约号的有效性
    *  Input:                                                               *
    *    std::string contract_id    : 签约号                                *
    *  Return: -1 失败  0 成功                                              *
    *-----------------------------------------------------------------------*/  
    int CheckContractID(string contract_id);


    /*----------------------------------------------------------------------
    *  Function Name: CheckIDCard
    *         Desc:校验身份证号的有效性
    *  Input:                                                               *
    *    std::string id_card    : 身份证号                                  *
    *  Return: -1 失败  0 成功                                              *
    *-----------------------------------------------------------------------*/     
    int CheckIDCard(string id_card);


    /*----------------------------------------------------------------------
    *  Function Name: CheckBankno
    *         Desc:校验银行卡号的有效性
    *  Input:                                                               *
    *    std::string bank_no    : 银行卡号                                  *
    *  Return: -1 失败  0 成功                                              *
    *-----------------------------------------------------------------------*/      
    int CheckBankno(string bank_no);


    /*----------------------------------------------------------------------
    *  Function Name: CheckAmount
    *         Desc:校验金额的有效性
    *  Input:                                                               *
    *    std::string amount    : 金额                                       *
    *  Return: -1 失败  0 成功                                              *
    *-----------------------------------------------------------------------*/   
    int CheckAmount(string amount);


    /*----------------------------------------------------------------------
    *  Function Name: CheckNumDot
    *         Desc:校验金额是由数字和小数点组成的
    *  Input:                                                               *
    *    std::string amount    : 金额                                       *
    *  Return: -1 失败  0 成功                                              *
    *-----------------------------------------------------------------------*/
    int CheckNumDot(string amount);

    /*----------------------------------------------------------------------
    *  Function Name: ContractDecode
    *         Desc:签约号解密 
    *  Input:                                                               *
    *    std::string contract    : 签约号                                   *
    *  Return: 解密后的签约号                                              *
    *-----------------------------------------------------------------------*/   
    std::string ContractDecode(std::string &contract);

    /*----------------------------------------------------------------------
    *  Function Name: ContractEncode
    *         Desc:签约号加密 
    *  Input:                                                               *
    *    std::string contract    : 签约号                                   *
    *  Return: 加密后的签约号                                              *
    *-----------------------------------------------------------------------*/   
    std::string ContractEncode(std::string &contract);

    /*----------------------------------------------------------------------
    *    Function Name: CheckAppID
    *    Desc: 校验商户流水号是否合规则
    *    Input:                                                             *
    *    std::string contract    : 商户流水号                               *
    *    Return: 正确返回 0  错误返回 -1                                    *
    *-----------------------------------------------------------------------*/
    int CheckAppId(std::string app_id);
  

  /********************************************************************************
   * *  Func: DecryptDESContext(std::string ciphertext, std::string deskey, std::string& plaintext)
   * *  Input:                                                                       
   * *    ciphertext: 密文
   * *    deskey: des秘钥 
   * *    plaintext: 解密结果明文
   * *  Description:                                                         
   * *    (1)3des解密
   * *  Return:                                                              
   * *    正确返回: 0
   *      错误返回: -1
   * *   Auth: liaobin                                                     
   * *   Date: 2017-04-26                                                   
   * ********************************************************************************/
  int DecryptDESContext(std::string ciphertext, std::string deskey, std::string& plaintext);

  
  /********************************************************************************
   * *  Func: EncryptDESContext(std::string plaintext, std::string deskey, std::string& ciphertext)
   * *  Input:  
   * *    plaintext: 明文
   * *    deskey: des秘钥 
   * *    ciphertext: 加密结果密文
   * *  Description:                                                         
   * *    (1)3des加密
   * *  Return:                                                              
   * *    正确返回: 0
   *      错误返回: -1
   * *   Auth: liaobin                                                     
   * *   Date: 2017-04-26                                                   
   * ********************************************************************************/
  int EncryptDESContext(std::string plaintext, std::string deskey, std::string& ciphertext);

  /*------------------------------------------------------------------------
   *Function Name: unicode2gbk
   *       Desc:
   *             转码函数 ---  "\u5bb6\u7528\u7535\u5668" 转为 GBK
   *             转码函数 ---  "\/" 转为 "/"
   *             转码函数 ---  单独为佰银JAVA通讯返回创建
   *      Input: 待转码串，待转码串长度，转码结果空间，转码结果空间大小
   *     Return: 转码后长度
   *       Auth: JiaYeHui
   *       Date: 2017-04-27
   *-----------------------------------------------------------------------*/
  int unicode2gbk(char *inbuf, size_t inlen, char *outbuf, size_t outlen);

  /*------------------------------------------------------------------------
   *Function Name: json2http
   *       Desc: 
   *        将":" json串转成&=这样的格式     
   *      Input: 
   *        待转码串 {"respDesc":"订单号重复","returnUrl":"http://www.transt.cn/cgi-bin/cdh_in_amount.cgi","transAmt":1000,"commodityName":"电器"}
   *        待转码串长度
   *        转码结果空间 respDesc=订单号重复&returnUrl=http://www.transt.cn/cgi-bin/cdh_in_amount.cgi&transAmt=1000&commodityName=电器
   *        转码结果空间大小
   *     Return: 转码后长度
   *       Auth: JiaYeHui
   *       Date: 2017-04-28                                                   
   *-----------------------------------------------------------------------*/
  int json2http(char *inbuf, size_t inlen, char *outbuf, size_t outlen);

  /*------------------------------------------------------------------------
   *Function Name: httpjson2map
   *       Desc: 
   *        将":" json串转成map
   *      Input: 
   *        待转码串 {"respDesc":"订单号重复","returnUrl":"http://www.transt.cn/cgi-bin/cdh_in_amount.cgi","transAmt":1000,"commodityName":"电器"}
   *        待转码串长度
   *        转码结果Map
   *     Return: -1失败 0成功
   *       Auth: JiaYeHui
   *       Date: 2017-04-28                                                   
   *-----------------------------------------------------------------------*/
  int httpjson2map(char *inbuf, size_t inlen, CStr2Map& outMap);
  
  /*------------------------------------------------------------------------
   *Function Name: JfpNotifyMd5 
   *       Desc: 
   *             支付结果通知
   *      Input: map有5个值
   *             trans_id    返回的交流流水号，唯一
   *             result      1：成功  2：失败
   *             amount      单位：分
   *             app_id      商户的流水号或者应用号，须保证流水唯一
   *             notify_url  通知给第三方的URL
   *     Return: -1失败 0成功
   *       Auth: JiaYeHui
   *       Date: 2017-08- 09                                                  
   *-----------------------------------------------------------------------*/
  int JfpNotifyMd5(CStr2Map& paramap);

  
/*------------------------------------------------------------------------
 *Function Name: AesEncrypt
 *       Desc: 
 *             aes加密
 *      Input: map有5个值
 *             trans_id    返回的交流流水号，唯一
 *             result      1：成功  2：失败
 *             amount      单位：分
 *             app_id      商户的流水号或者应用号，须保证流水唯一
 *             notify_url  通知给第三方的URL
 *     Return: -1失败 0成功
 *       Auth: JiaYeHui
 *       Date: 2017-08-13
 *-----------------------------------------------------------------------*/
int AesEncrypt(unsigned char *key,unsigned char* in, int len, unsigned char* out);

/*------------------------------------------------------------------------
 *Function Name: AesDecrypt
 *       Desc: 
 *             aes解密
 *      Input: map有5个值
 *             trans_id    返回的交流流水号，唯一
 *             result      1：成功  2：失败
 *             amount      单位：分
 *             app_id      商户的流水号或者应用号，须保证流水唯一
 *             notify_url  通知给第三方的URL
 *     Return: -1失败 0成功
 *       Auth: JiaYeHui
 *       Date: 2017-08-13
 *-----------------------------------------------------------------------*/
int AesDecrypt(unsigned char* key,unsigned char* in,int len,unsigned char* out);


/********************************************************************************
 *  Func: DecryptDesUrlbase64(std::string ciphertext, std::string deskey, std::string& plaintext)
 *  Input:                                                                       
 *    ciphertext: 密文
 *    deskey: des秘钥 
 *    plaintext: 解密结果明文
 *  Description:                                                         
 *    (1)3des解密
 *  Return:                                                              
 *    正确返回: 0
 *    错误返回: -1
 *   Auth: JiaYeHui
 *   Date: 2017-08-18                                                  
 ********************************************************************************/
int DecryptDesUrlbase64(std::string ciphertext, std::string deskey, std::string& plaintext);

/********************************************************************************
 *  Func: EncryptDesUrlbase64(std::string plaintext, std::string deskey, std::string& ciphertext)
 *  Input:  
 *    plaintext: 明文
 *    deskey: des秘钥 
 *    ciphertext: 加密结果密文
 *  Description:                                                         
 *    (1)3des加密
 *  Return:                                                              
 *    正确返回: 0
 *    错误返回: -1
 *   Auth: JiaYeHui
 *   Date: 2017-08-18                                                  
 ********************************************************************************/
int EncryptDesUrlbase64(std::string plaintext, std::string deskey, std::string& ciphertext);

int JfpNotifyDesUrlBase(CStr2Map& paramap);

/*------------------------------------------------------------------------
 *Function Name: html_unicode2gbk
 *       Desc: 
 *             转码函数 ---  "&#36134;&#25143;[partid:9,827,373,accountnumber:8800009845129]&#20313;" 转为 GBK
 *             转码函数 ---  上面内容转为GBK结果： "账户[partid:9,827,373,accountnumber:8800009845129]余"
 *             转码函数 ---  单独为佰银JAVA通讯返回创建
 *             &#x0000;
 *             &#00000;
 *             这种编码称为 HTML 编码。HTML 编码有两种形式：
 *             第一种是 Unicode 代码点的十六进制表示，后一种是 Unicode 代码点的十进制表示。
 *      Input: 待转码串，待转码串长度，转码结果空间，转码结果空间大小
 *     Return: 转码后长度
 *       Auth: JiaYeHui
 *       Date: 2017-04-27                                                   
 *-----------------------------------------------------------------------*/
int html_unicode2gbk(char *inbuf, size_t inlen, char *outbuf, size_t outlen);

/*------------------------------------------------------------------------
 *Function Name:CheckTransTime 
 *       Desc: 
 *             交易时间误差在300秒内
 *      Input: YYYYMMDDHHMMSS
 *       Auth: JiaYeHui
 *       Date: 2017-11-18                                                 
 *-----------------------------------------------------------------------*/
void CheckTransTime(string &trans_time,const int num_sec=300);

/*------------------------------------------------------------------------
 *Function Name: JfpNotifyMd5Urlbase
 *       Desc: 
 *             支付结果通知
 *      Input: map有5个值
 *             trans_id    返回的交流流水号，唯一
 *             result      1：成功  2：失败
 *             amount      单位：分
 *             app_id      商户的流水号或者应用号，须保证流水唯一
 *             notify_url  通知给第三方的URL
 *     Return: -1失败 0成功
 *       Auth: JiaYeHui
 *       Date: 2017-08- 09                                                  
 *-----------------------------------------------------------------------*/
int JfpNotifyMd5Urlbase(CStr2Map& paramap);

//报文统一解析  只在接口使用
int parseRespJson(std::string &data, CStr2Map& outMap);

//
int parseRespJsonERP(std::string &data, string& outToken);

/********************************************************************************
 * *  Func: CheckAppIdNoDate(string &app_id)
 * *  Input:                                                                       
 * *    app_id: 输入app_id
 * *  Description:                                                         
 * *    (1)判断用户的流水号是否合法
 * *    (2)不判断日期开头
 * *  Return:                                                              
 * *    正确返回: 0
 *      错误返回: -1
 * *   Auth: JiaYeHui
 * *   Date: 2017-11-25                                                   
 * ********************************************************************************/
int CheckAppIdNoDate(std::string app_id);

/*------------------------------------------------------------------------
 *Function Name: parsePubRespJson
 *       Desc: 
 *        将":" json串转成map 单层输出
 *      Input:  
 *        待转码串 {"respDesc":"订单号重复","returnUrl":"http://www.jfbill.com/cgi-bin/cdh_in_amount.cgi"}
 *        转码结果Map
 *     Return: -1失败 0成功
 *       Auth: JiaYeHui
 *       Date: 2018-01-28                                                   
 *-----------------------------------------------------------------------*/
int parsePubRespJson(string &data, CStr2Map& outMap);

/*------------------------------------------------------------------------
 *Function Name: parsePubRespJsonList
 *       Desc: 
 *        将":" json串转成map 多层输出
 *      Input:  
 *        待转码串data
 *        转码结果Map
 *     Return: -1失败 0成功
 *       Auth: WuZhiHao
 *       Date: 2018-06-16                                                   
 *-----------------------------------------------------------------------*/
int parsePubRespJsonList(string &data, CStr2Map& outMap,vector<CStr2Map>& vectmapArray);


/*------------------------------------------------------------------------
 *Function Name: parsePubReaJsonList
 *       Desc: 
 *        将":" json串转成map 多层输出
 *      Input:  
 *        待转码串data
 *        转码结果Map
 *     Return: -1失败 0成功
 *       Auth: WuZhiHao
 *       Date: 2025-01-16                                                   
 *-----------------------------------------------------------------------*/
int parsePubReqJsonList(string &data, CStr2Map& outMap,vector<CStr2Map>& vectmapArray);
int parsePubReqJsonList1(string &data, CStr2Map& outMap,vector<CStr2Map>& vectmapArray);

/***********************************************************************
 *  Func: CheckDebitCard(string card_no)
 *  Input:                                                                       
 *    card_no: 输入借记卡号
 *  Description:                                                         
 *    (1)判断借记卡是否符合规则
 *  Return:                                                              
 *    正确卡号返回: 0
 *    错误卡号返回: -1
 *   Auth: JiaYeHui
 *   Date: 2018-02-25                                                   
 * *********************************************************************/
int CheckDebitCard(string card_no);

//配置秘钥，给通讯用的，只有1-8个秘钥
//第1个给收银台用
//第2个给CGI通讯使用
string GetConDESKey(int sub);

//配置秘钥，给通讯用的，只有1-8个秘钥
//第1个给收银台用
//第2个给CGI通讯使用
string GetConMD5Key(int sub);

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
     *Function Name:MapToJson
     *       Desc: 将Map转为Json格式
     *      Input: 
     *            map:Map键值对
     *     Return:json
     *       Auth: JiaYeHui
     *       Date: 2018-04-04                                                
     *-----------------------------------------------------------------------*/
    string MapToJson(const map<string, string> &m);
    
    //DES/ECB/PKCS5PADDING java缺多少补多少去进行加密
    string des3_java(const string& des3Key,const string& srcStr);

	string des3_base64(const string& des3Key,const string& srcStr);
	
    /*------------------------------------------------------------------------
     *Function Name:RsaNumEnc
     *       Desc: 将cgi的字符串数据rsa加密
     *      Input: 
     *              n_key:第几个秘钥,目前只有2个
     *           cgi_data:明文字符串
     *     Return: urlbase64的rsa密文
     *       Auth: JiaYeHui
     *       Date: 2018-04-25                                               
     *-----------------------------------------------------------------------*/
    string RsaNumEnc(int n_key,string &strData);

    /*------------------------------------------------------------------------
     *Function Name: gbk2unicode
     *       Desc: 
     *             转码函数 ---  GBK 转为 "\u5bb6\u7528\u7535\u5668" 
     *      Input: 待转码串，待转码串长度，转码结果空间，转码结果空间大小
     *     Return: 转码后长度
     *       Auth: JiaYeHui
     *       Date: 2018-06-08
     *-----------------------------------------------------------------------*/
    int gbk2unicode(char *inbuf, size_t inlen, char *outbuf, size_t outlen);
	
	    /*------------------------------------------------------------------------
     *Function Name:WebrsaToOpen
     *       Desc: 将前端的RSA密码转为ADS后台DES密码
     *      Input: 
     *            web_pwd:前端密码 urlbase64
     *              n_web:前端第几个秘钥,目前只有2个
     *     Return:ADS后台DES密码
     *       Auth: JiaYeHui
     *       Date: 2018-03-30                                                
     *-----------------------------------------------------------------------*/
    string WebrsaToOpen(int n_web,string &web_pwd);


	/*------------------------------------------------------------------------
	*Function Name:WebrsaToOpen
	*       Desc: 将前端的RSA加密信息解密为明文
	*      Input: 
	*            web_pwd:前端密码 urlbase64
	*              n_web:前端第几个秘钥,目前只有2个
	*              n_ads:后台第几个秘钥,目前只有40个[0-39] 0登录密码 1交易密码
	*            num_sec:rsa精确到几秒，默认20秒
	*     Return:ADS后台DES密码
	*       Auth: leize
	*       Date: 2018-09-06                                            
	*-----------------------------------------------------------------------*/
	string WebrsaDec(int n_web,string &web_pwd,int n_ads,const int num_sec);

	/*------------------------------------------------------------------------
	*Function Name:deleteAllMark
	*       Desc: 去除指定字符
	*      Input: 
	*            s:目标串
	*            mark:去除的指定字符
	*     Return:ADS后台DES密码
	*       Auth: leize
	*       Date: 2018-09-06                                            
	*-----------------------------------------------------------------------*/
	void deleteAllMark(string &s, const string &mark);

	/*------------------------------------------------------------------------
	*  Function Name: Http2MerLink
	*         Desc: http 通讯到商联预付费卡接口
	*  Input:                                                               *
	*    std::string & strUrl    : URL地址                                  *
 *    CStr2Map & reqMap     : get数据                                  *
	*    std::string& strResponse  : 响应数据                               *
	*  Return: -1 失败  0 成功                                              *
	*-----------------------------------------------------------------------*/  
	int Http2MerLink(const std::string & strUrl, CStr2Map & reqMap, CStr2Map & respMap);

 string des3_base64_ml(const string& des3Key,const string& srcStr);

 string Decdes3_base64_ml(const string& des3Key,const string& srcStr);

 string des3_tohex(const string& des3Key,const string& srcStr);

 string RsaEnc(string &strData);

 string RsaDESenc(string &srcStr);

 unsigned char* Str2Hex(const char *str, unsigned char *hex, int sLen);

 char* Hex2Str(const unsigned char *hex, char *str, int hLen);

 int hex2bytearray(unsigned char* s,unsigned char bits[]);

 vector<string> split(const string& s, const string& delim, const bool keep_empty = true);

 char* solve(char* dest,const char*src);

 int UploadFile2MinIO(S3Cfg &cfg, const char *localPath, const char *remotePath=NULL, char *objectName=NULL);

 int DownloadFileFromMinIO(S3Cfg &cfg, const char *objectName, char *localPath);

 int CheckFileFromMinIO(S3Cfg &cfg, const char *objectName);
 
 int SendAlarm2(const char *format, ...);

 bool exists(const string& path);

 bool IsPIDNumber(string id);

 //计算文件hash
 string calculateFileHash(const std::string& filePath);

 //svn路径转义
 string escape_svn_path(const std::string& svn_path);

 //解析ES返回报文并转换成本系统接口形式
 string TransformJson(const string& rawJson);

};

#endif
