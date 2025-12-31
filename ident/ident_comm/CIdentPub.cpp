/*
 * CIdentPub.cpp
 *
 *  Identated on: 2013-3-11
 *      Identor: JiaYeHui
 */

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "CIdentPub.h"
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/aes.h>
#include <openssl/sha.h>
#include "xml/unicode.h"
#include "curl/curl.h"  
#include <ifaddrs.h>
#include <base/charset.hpp>

using namespace aps;
using namespace rapidjson;   
using namespace std;  

#define STATE_16 "16"
/**
 * RSA最大加密明文大小
 */
#define MAX_ENCRYPT_BLOCK  117

/**
 * RSA最大解密密文大小
 */
#define MAX_DECRYPT_BLOCK  128

/*------------------------------------------------------------------------
 *Function Name: GetDivDay
 *       Desc: 根据起始日、间隔天数、方向计算结束日
 *      Input: 日期串(YYYY-MM-DD)
 *     Return: 结束日串(YYYY-MM-DD)
 *-----------------------------------------------------------------------*/
string CIdentPub::GetDivDay(const char * start_date, int nday)
{
     int day;
     int mon;
     int year;
     time_t t1;
     struct tm m1;
     char tmpbuf[20];

     time(&t1);
     m1 = *localtime(&t1);
     memset(tmpbuf, 0x00, sizeof(tmpbuf));
     memcpy(tmpbuf, start_date, 4);
     year = atoi(tmpbuf);
     memset(tmpbuf, 0x00, sizeof(tmpbuf));
     memcpy(tmpbuf, start_date + 5, 2);
     mon = atoi(tmpbuf);
     memset(tmpbuf, 0x00, sizeof(tmpbuf));
     memcpy(tmpbuf, start_date + 8, 2);
     day = atoi(tmpbuf);

     m1.tm_year = year - 1900;
     m1.tm_mon = mon - 1;
     m1.tm_mday = day + nday;
     t1 = mktime(&m1);
     m1 = *localtime(&t1);

     year = m1.tm_year + 1900;
     mon = m1.tm_mon + 1;
     day = m1.tm_mday;
     memset(tmpbuf, 0x00, sizeof(tmpbuf));
     sprintf(tmpbuf, "%04d-%02d-%02d", year, mon, day);

     return string(tmpbuf);
}

/*------------------------------------------------------------------------
 *Function Name: GetXyyDay
 *       Desc: 根据起始日、间隔天数、方向计算结束日
 *      Input: 日期串(YYYY-MM-DD)
 *             间隔天数(XYY): X: 1天 2月 3年 (155 55天 203 3个月 301 一年)
 *     Return: 结束日串(YYYY-MM-DD)
 *-----------------------------------------------------------------------*/
string CIdentPub::GetXyyDay(const char * start_date, int nday)
{
     int day;
     int mon;
     int year;
     time_t t1;
     struct tm m1;
     char tmpbuf[20];
     int i;

     time(&t1);
     m1 = *localtime(&t1);
     memset(tmpbuf, 0x00, sizeof(tmpbuf));
     memcpy(tmpbuf, start_date, 4);
     year = atoi(tmpbuf);
     memset(tmpbuf, 0x00, sizeof(tmpbuf));
     memcpy(tmpbuf, start_date + 5, 2);
     mon = atoi(tmpbuf);
     memset(tmpbuf, 0x00, sizeof(tmpbuf));
     memcpy(tmpbuf, start_date + 8, 2);
     day = atoi(tmpbuf);

     if (nday > 0)
     {
          if (nday > 300)
          {
               sprintf(tmpbuf, "%04d-%02d-%02d", year + nday - 300, mon, day);
          }
          else if (nday > 200)
          {
               i = mon + nday - 200;
               if (i == 12)
                    sprintf(tmpbuf, "%04d-%02d-%02d", year, 12, day);
               else if ((i % 12) == 0)
                    sprintf(tmpbuf, "%04d-%02d-%02d", year + (i / 12) - 1, 12, day);
               else
                    sprintf(tmpbuf, "%04d-%02d-%02d", year + (i / 12), i % 12, day);
          }
          else if (nday > 100)
          {
               nday -= 100;
               m1.tm_year = year - 1900;
               m1.tm_mon = mon - 1;
               m1.tm_mday = day + nday;
               t1 = mktime(&m1);
               m1 = *localtime(&t1);

               year = m1.tm_year + 1900;
               mon = m1.tm_mon + 1;
               day = m1.tm_mday;
               memset(tmpbuf, 0x00, sizeof(tmpbuf));
               sprintf(tmpbuf, "%04d-%02d-%02d", year, mon, day);
          }
          else
          {
               return string("");
          }
     }
     else
     {
          if (nday < -300)
          {
               sprintf(tmpbuf, "%04d-%02d-%02d", year + nday + 300, mon, day);
          }
          else if (nday < -200)
          {
               i = mon + nday + 200;
               if (i > 0)
                    sprintf(tmpbuf, "%04d-%02d-%02d", year, i, day);
               else if (i == 0)
                    sprintf(tmpbuf, "%04d-%02d-%02d", year - 1, 12, day);
               else if ((i % 12) == 0)
                    sprintf(tmpbuf, "%04d-%02d-%02d", year + (i / 12) - 1, 12, day);
               else
                    sprintf(tmpbuf, "%04d-%02d-%02d", year + (i / 12) - 1, 12 + (i
                              % 12), day);
          }
          else if (nday < -100)
          {
               nday += 100;
               m1.tm_year = year - 1900;
               m1.tm_mon = mon - 1;
               m1.tm_mday = day + nday;
               t1 = mktime(&m1);
               m1 = *localtime(&t1);

               year = m1.tm_year + 1900;
               mon = m1.tm_mon + 1;
               day = m1.tm_mday;
               memset(tmpbuf, 0x00, sizeof(tmpbuf));
               sprintf(tmpbuf, "%04d-%02d-%02d", year, mon, day);
          }
          else
          {
               return string("");
          }
     }

     return string(tmpbuf);
}

//取当前时间
int CIdentPub::GetTimeNow(char *str)
{
     time_t tt;
     struct tm stTm;

     tt = time(NULL);
     memset(&stTm, 0, sizeof(stTm));
     localtime_r(&tt, &stTm);
     sprintf(str, "%04d-%02d-%02d %02d:%02d:%02d", stTm.tm_year + 1900,
               stTm.tm_mon + 1, stTm.tm_mday, stTm.tm_hour, stTm.tm_min,
               stTm.tm_sec);

     return 0;
}

//取秒间隔时间
int CIdentPub::GetTimeNowDivSec(char *str,int divsec)
{
     time_t tt;
     struct tm stTm;

     tt = time(NULL);
        tt = tt + divsec;
     memset(&stTm, 0, sizeof(stTm));
     localtime_r(&tt, &stTm);
     sprintf(str, "%04d-%02d-%02d %02d:%02d:%02d", stTm.tm_year + 1900,
               stTm.tm_mon + 1, stTm.tm_mday, stTm.tm_hour, stTm.tm_min,
               stTm.tm_sec);

     return 0;
}

string CIdentPub::GetCurrentTime()
{
     time_t tt;
     struct tm stTm;

     tt = time(NULL);
     char str[22];
     memset(str, 0, sizeof(str));
     memset(&stTm, 0, sizeof(stTm));
     localtime_r(&tt, &stTm);
     sprintf(str, "%02d%02d%02d", stTm.tm_hour, stTm.tm_min, stTm.tm_sec);
     return string(str);
}

string CIdentPub::GetCurrentDateTime()
{
     time_t tt;
     struct tm stTm;

     tt = time(NULL);
     char str[22];
     memset(str, 0, sizeof(str));
     memset(&stTm, 0, sizeof(stTm));
     localtime_r(&tt, &stTm);
     sprintf(str, "%04d%02d%02d%02d%02d%02d", stTm.tm_year + 1900, stTm.tm_mon
               + 1, stTm.tm_mday, stTm.tm_hour, stTm.tm_min, stTm.tm_sec);
     return string(str);
}

//取当前的时间格式YYYYMMDD HH:MM:SS
string CIdentPub::GetFormatTimeNow()
{
     time_t tt;
     struct tm stTm;

     tt = time(NULL);
     char str[20];
     memset(str, 0, sizeof(str));
     memset(&stTm, 0, sizeof(stTm));
     localtime_r(&tt, &stTm);
     sprintf(str, "%04d%02d%02d %02d:%02d:%02d", stTm.tm_year + 1900, stTm.tm_mon
               + 1, stTm.tm_mday, stTm.tm_hour, stTm.tm_min, stTm.tm_sec);
     return string(str);
}

//取当前日期格式为YYYY-MM-DD
string CIdentPub::GetDateNow()
{
     time_t tt;
     struct tm stTm;
     char str[20];

     tt = time(NULL);
     memset(&stTm, 0, sizeof(stTm));
     localtime_r(&tt, &stTm);
     memset(str, 0x00, sizeof(str));
     sprintf(str, "%04d-%02d-%02d", stTm.tm_year + 1900, stTm.tm_mon + 1,
               stTm.tm_mday);

     return string(str);
}
//取当前日期格式为YYYYMMDD
string CIdentPub::GetFormatDateNow()
{
     time_t tt;
     struct tm stTm;
     char str[20];

     tt = time(NULL);
     memset(&stTm, 0, sizeof(stTm));
     localtime_r(&tt, &stTm);
     memset(str, 0x00, sizeof(str));
     sprintf(str, "%04d%02d%02d", stTm.tm_year + 1900, stTm.tm_mon + 1,
               stTm.tm_mday);

     return string(str);
}

// 生成时间
void CIdentPub::GetTime(string & strTime)
{
     struct tm stTm;
     time_t tt;
     tt = time(NULL);

     char str[21] =
     { 0 };

     memset(&stTm, 0, sizeof(stTm));
     localtime_r(&tt, &stTm);
     sprintf(str, "%04d-%02d-%02d %02d:%02d:%02d", stTm.tm_year + 1900,
               stTm.tm_mon + 1, stTm.tm_mday, stTm.tm_hour, stTm.tm_min,
               stTm.tm_sec);

     strTime = string(str);
}
string CIdentPub::GetDivHourTime(const string& sStartTime, int nHour)
{
        struct tm m1;
        memset(&m1,0x0,sizeof(m1));
        time_t t1;
        /*
         *将日期转换成以秒为单位的整型时间
         */
        m1.tm_year = atoi(sStartTime.substr(0,4).c_str())- 1900;
        m1.tm_mon  = atoi(sStartTime.substr(5,2).c_str())- 1;
        m1.tm_mday = atoi(sStartTime.substr(8,2).c_str());
        m1.tm_hour = atoi(sStartTime.substr(11,2).c_str()) + nHour;
        m1.tm_min  = atoi(sStartTime.substr(14,2).c_str());
        m1.tm_sec  = atoi(sStartTime.substr(17,2).c_str());

        t1 = mktime(&m1);
        m1 = *localtime(&t1);

        char tmpbuf[20];
        memset(tmpbuf, 0x00, sizeof(tmpbuf));
        sprintf(tmpbuf, "%04d-%02d-%02d %02d:%02d:%02d",
                m1.tm_year + 1900, m1.tm_mon + 1, m1.tm_mday, m1.tm_hour,m1.tm_min, m1.tm_sec);
        return tmpbuf;
}

/*------------------------------------------------------------------------
 *Function Name: TranslateDate
 *       Desc: 将YYYY-MM-DD HH:MM:SS等的分隔符去掉，只返回数字
 *      Input: 
 *        srcDate: 类似(YYYY-MM-DD HH:MM:SS)等组合日期串
 *           flag: 1 只返回前8位
 *     Return: 纯数字的结束串 YYYYMMDDHHMMSS等
 *-----------------------------------------------------------------------*/
string CIdentPub::TranslateDate(const string& srcDate, const int flag)
{
     char tmpbuf[60];
     char *s1;
     char *s2;

     memset(tmpbuf, 0x00, sizeof(tmpbuf));
     strncpy(tmpbuf, srcDate.c_str(), 50);

     s1 = tmpbuf;
     s2 = tmpbuf;

     while (*s1)
     {
          if ((*s1 >= '0') && (*s1 <= '9'))
               *s2++ = *s1;
          s1++;
     }
     *s2 = '\0';
     if (1 == flag)
     {
          tmpbuf[8] = '\0';
     }

     return string(tmpbuf);
}

int CIdentPub::codeConvert(const char* src_page, const char* dst_page,
          const char* szSourceText, int inLength, char* szDestBuff, int bufSize)
{
     iconv_t conv;
     conv = iconv_open(dst_page, src_page);
     if (conv == (iconv_t) - 1)
          return -1;

     char* ptrInput = (char*) szSourceText;
     char* ptrOutput = szDestBuff;

     size_t nInLeft = inLength;
     size_t nOutLeft = bufSize;

     size_t ret;
     ret = iconv(conv, &ptrInput, &nInLeft, &ptrOutput, &nOutLeft);
     iconv_close(conv);

     if (ret == (size_t) -1)
          return -1;

     return (bufSize - nOutLeft);
}

// 转码 
// 输入参数
//     strBuf  需要转移的字符，转码后的字符也保存在这里
//     szFrom  输入的字符集 GB18030 GB2312 UTF-8 
//     szTo    输出的字符集 GB18030 GB2312 UTF-8 
// 返回 0成功 -1失败
int CIdentPub::ChangeCharacterSize(string &strBuf, const char * szFrom,
          const char * szTo)
{
     // 有一个为空，不转义
     if (strcmp(szFrom, "") == 0 || strcmp(szTo, "") == 0)
          return 0;

     // 不需要转义
     if (strcmp(szFrom, szTo) == 0)
          return 0;

     // 输入为空，不转义
     if (strBuf.empty())
          return 0;

     char szTmp[MAX_CONVERT_SIZE] =
     { 0 };
     if (CIdentPub::codeConvert(szFrom, szTo, strBuf.c_str(),
               strBuf.length(), szTmp, MAX_CONVERT_SIZE) < 0)
     {
          return -1;
     }

     strBuf = szTmp;
     return 0;
}

/*------------------------------------------------------------------------
 *Function Name: ChangeContractState
 *       Desc: 根据不同发起人,返回不同状态
 *      Input: 数据库状态,发起人类别
 *     Return: 转换后的状态
 *-----------------------------------------------------------------------*/
string CIdentPub::ChangeContractState(const string &state, int strno)
{
     //借款人发起查询
     if (strno == CIdentPub::APPLY_NO_PUB)
     {
          //审批中
          if ((state == CTT_STATE_APPLY) || (state == CTT_STATE_WAIT) || (state
                    == CTT_STATE_CHECK_PASS) || (state == CTT_STATE_CHECK_FAIL)
                    || (state == CTT_STATE_WAIT_QUERYCONTRACT) || (state
                    == CTT_STATE_RUNDATA_PREPARED) ) 
          {
               return string("1");
          }
          //待确认
          if ((state == CTT_STATE_APPROVE1))
          {
               return string("2");
          }
          //成功
          if ((state == CTT_STATE_PASS) || (state == CTT_STATE_CONTRACT_SIGN)
                    || (state == CTT_STATE_CONTRACT_SENDBANK))
          {
               return string("3");
          }
          //失败
          if ((state == CTT_STATE_CANCEL) || (state == CTT_STATE_ABOLISH)
                    || (state == CTT_STATE_REFUSE) || (state == CTT_STATE_QUIT))
          {
               return string("4");
          }
     }

     //担保借款人查询
     if (strno == CIdentPub::ASSURE_APPLY_NO_PUB)
     {
          //不确定
          if (state == CTT_STATE_APPROVE_CONTRACT)
          {
               return string("0");
          }
          //待审核
          if ((state == CTT_STATE_APPLY) || (state == CTT_STATE_WAIT) || (state
                    == CTT_STATE_APPROVE1) || (state == CTT_STATE_CHECK_PASS)
                    || (state == CTT_STATE_CHECK_FAIL) || (state
                    == CTT_STATE_WAIT_QUERYCONTRACT) || (state
                    == CTT_STATE_RUNDATA_PREPARED))
          {
               return string("1");
          }
          //已放款
          if ((state == CTT_STATE_PASS) || (state == CTT_STATE_CONTRACT_SIGN)
                    || (state == CTT_STATE_CONTRACT_SENDBANK))
          {
               return string("4");
          }
          //审核未通过
          if ((state == CTT_STATE_CANCEL) || (state == CTT_STATE_ABOLISH)
                    || (state == CTT_STATE_REFUSE) || (state == CTT_STATE_QUIT))
          {
               return string("3");
          }
     } 
     //担保人发起查询
     if (strno == CIdentPub::ASSURE_NO_PUB)
     {
          //待审核
          if ((state == CTT_STATE_APPLY) || (state
                    == CTT_STATE_CHECK_FAIL) || (state
                    == CTT_STATE_WAIT_QUERYCONTRACT) || (state
                    == CTT_STATE_RUNDATA_PREPARED))
          {
               return string("1");
          }
          //待审批,返回2有审批按钮
          if (state == CTT_STATE_CHECK_PASS)
          {
               return string("2");
          }
                //xieduhui 20130822  担保人查询时，修改状态为2时返回5(审批通过 ）
          //审核通过
          if (state == CTT_STATE_APPROVE1 || state == CTT_STATE_WAIT)
          {
               return string("5");
          }
          //已放款
          if ((state == CTT_STATE_PASS) || (state == CTT_STATE_CONTRACT_SIGN)
                    || (state == CTT_STATE_CONTRACT_SENDBANK))
          {
               return string("4");
          }
          //审核未通过
          if ((state == CTT_STATE_CANCEL) || (state == CTT_STATE_ABOLISH)
                    || (state == CTT_STATE_REFUSE) || (state == CTT_STATE_QUIT))
          {
               return string("3");
          }
     }

     //贷款人发起查询
     if (strno == CIdentPub::LOAN_NO_PUB)
     {
          //待审核
          if ((state == CTT_STATE_APPLY) || (state == CTT_STATE_APPROVE1)
                    || (state == CTT_STATE_CHECK_PASS) || (state
                    == CTT_STATE_CHECK_FAIL) || (state
                    == CTT_STATE_RUNDATA_PREPARED) || (state
                    == CTT_STATE_WAIT_QUERYCONTRACT))
          {
               return string("1");
          }
          if (state == CTT_STATE_WAIT)
          {
               return string("2");
          }
          //已放款
          if ((state == CTT_STATE_PASS) || (state == CTT_STATE_CONTRACT_SIGN)
                    || (state == CTT_STATE_CONTRACT_SENDBANK))
          {
               return string("4");
          }
          //审核未通过
          if ((state == CTT_STATE_CANCEL) || (state == CTT_STATE_ABOLISH)
                    || (state == CTT_STATE_REFUSE) || (state == CTT_STATE_QUIT))
          {
               return string("3");
          }
     }

     //其它原值返回
     return state;
}

/*获取配置文件的3DES密钥 cre_3des_key 失败抛出异常*/
string CIdentPub::GetIdentDesKey()
{
     //string sDesKey("42B43D098214ED23984D38D9E64668D5");
     string sDesKey("1234567887654321");

     return sDesKey;
}
/*获取配置操作部门和操作员，并进行非空判断,失败抛出异常*/
void CIdentPub::GetDeptAndTeller(string& dept, string& teller)
{
     dept = g_allVar.GetValue("jifen_oper", "dept");
     teller = g_allVar.GetValue("jifen_oper", "teller");

     if (dept.empty())
     {
          WarnLog("未配置虚拟操作部门 [%s]", "dept");
          throw(CTrsExp(ERR_GET_DEPT, "获取虚拟操作部门错误"));
     }
     if (teller.empty())
     {
          WarnLog("未配置虚拟操作员 [%s]", "teller");
          throw(CTrsExp(ERR_GET_TELLER, "获取虚拟操作员错误"));
     }

}
string CIdentPub::undes3(const string& src_str)
{
    char outstr[1024];
    char key[48];
    char srcbuf_hex[1024];
    char srcbuf[512];
 
     if (src_str.empty())
     {
          return string("");
     }
    memset(srcbuf,0x00,sizeof(srcbuf));
    memset(srcbuf_hex,0x00,sizeof(srcbuf));
    memcpy(srcbuf_hex,src_str.c_str(),src_str.length());
    ads_hextoasc(srcbuf_hex,(unsigned char *)srcbuf);

     //获取3DES密钥，获取失败抛出异常
    memset(key,0x00,sizeof(key));
     string sDesKey = CIdentPub::GetIdentDesKey();
    strcpy(key,sDesKey.c_str());

    memset(outstr,0x00,sizeof(outstr));
    ads_3des_string((char*)"D",(unsigned char*)key,srcbuf,(src_str.length())/2,outstr);
     ErrorLog("outstr:[%d] [%s]",(src_str.length())/2,outstr);
     return outstr;
}

string CIdentPub::undes3_mobile(const string& src_str)
{
    char outstr[1024];
    char key[48];
    char srcbuf_hex[1024];
    char srcbuf[512];


    if (src_str.empty())
    {
            return string("");
    }
    memset(srcbuf,0x00,sizeof(srcbuf));
    memset(srcbuf_hex,0x00,sizeof(srcbuf));
    memcpy(srcbuf_hex,src_str.c_str(),src_str.length());
    ads_hextoasc(srcbuf_hex,(unsigned char *)srcbuf);

    //设置3DES密钥
    memset(key,0x00,sizeof(key));
    strcpy(key,"8765567887655678");

    memset(outstr,0x00,sizeof(outstr));
    ads_3des_string((char*)"D",(unsigned char*)key,srcbuf,(src_str.length())/2,outstr);
        ErrorLog("outstr:[%d] [%s]",(src_str.length())/2,outstr);
        return outstr;
}

string CIdentPub::des3(const string& src_str)
{
     if (src_str.empty())
     {
          return string("");
     }

     //加密后的字符串
     string des_str;

     //获取3DES密钥，获取失败抛出异常 
     string sDesKey = CIdentPub::GetIdentDesKey();

     return des_str;
}

//DES/ECB/PKCS5PADDING java缺多少补多少去进行加密
string CIdentPub::des3_java(const string& des3Key,const string& srcStr)
{
    char srcData[4096];
    int  srcLen;
    int  paddNum;
    int  i;
    char desData[4096];
    char desDataHex[2*4096];

    memset(srcData,0x00,sizeof(srcData));
    memset(desData,0x00,sizeof(desData));
    memset(desDataHex,0x00,sizeof(desDataHex));

    srcLen=srcStr.length();
    if(srcLen > 4080 || srcLen <1)
    {
        InfoLog("3des-待加密内容长度错误Len=[%d]", srcLen);
        return "";
    }

    strcpy(srcData,srcStr.c_str());

    //PKCS#5
    paddNum = 8 - srcLen % 8;
    for (i=0; i < paddNum; ++i) 
        srcData[srcLen + i] = paddNum;

    srcLen=srcLen + paddNum;

    srcLen = ads_3des_string((char *)"E",(unsigned char*)des3Key.c_str(),srcData,srcLen,desData); 
    if(0 >= srcLen)
    {
        InfoLog("3des-加密后长度错误Len=[%d]",srcLen);
        return "";
    }
    ads_asctohex(desData,srcLen,desDataHex,sizeof(desDataHex));

    return desDataHex;
}

string CIdentPub::des3_base64(const string& des3Key,const string& srcStr)
{
	char srcData[4096];
	int  srcLen;
	int  paddNum;
	int  i;
	char desData[4096];
	char desDataHex[2*4096];
	char base64Encrypt[4096];

	memset(srcData,0x00,sizeof(srcData));
	memset(desData,0x00,sizeof(desData));
	memset(desDataHex,0x00,sizeof(desDataHex));
	memset(base64Encrypt,0x00,sizeof(base64Encrypt));

	srcLen=srcStr.length();
	if(srcLen > 4080 || srcLen <1)
	{
		InfoLog("3des-待加密内容长度错误Len=[%d]", srcLen);
		return "";
	}

	strcpy(srcData,srcStr.c_str());

	//PKCS#5
	paddNum = 8 - srcLen % 8;
	for (i=0; i < paddNum; ++i) 
		srcData[srcLen + i] = paddNum;

	srcLen=srcLen + paddNum;

	srcLen = ads_3des_string((char *)"E",(unsigned char*)des3Key.c_str(),srcData,srcLen,desData); 
	if(0 >= srcLen)
	{
		InfoLog("3des-加密后长度错误Len=[%d]",srcLen);
		return "";
	}
	//ads_asctohex(desData,srcLen,desDataHex,sizeof(desDataHex));
	ads_base64_enc(desData,srcLen,base64Encrypt,srcLen*3);

	InfoLog("3des-desData=[%s]",desData);
	InfoLog("3des-base64Encrypt=[%s]",base64Encrypt);

	return base64Encrypt;
}

string CIdentPub::GetDateAfterXYear(const string& start_date, int nYear)
{
       if(start_date.empty())
           return string("");

        stringstream ss;

        //获取目标年份
        int nyear =
                 atoi(start_date.substr(0,4).c_str()) + nYear;
        //字符串长度
        int sSize = start_date.length() ;

        //获取年份后面的月和日字符串
        string monthDay = start_date.substr(5, sSize - 4);

        //如果当前日期为2月29号，且目标年为平年 ,取2月28号值
        if(monthDay == "02-29" &&
              !(0==nyear%4 && (0!=nyear%100 || 0==nyear%400)))
        {
             monthDay = "02-28";
        }
        ss << nyear << "-" << monthDay;
        return ss.str();
}

string CIdentPub::BankIdMask(const string& bank_id)
{
     if(bank_id.empty())
          return string("");
     /*
      *如果银行卡号小于4，则输出原串
      */
        if(bank_id.size() <=4 )
        {
          return bank_id;
     }
     //得到**********1134格式,默认返回
     return Tools::SubNumWithChar(bank_id,0,bank_id.size()-4,'*');
}

string CIdentPub::GetTransConfigNotEmpty(const string & tid,const string & attrName)
{
        string sRet = g_mTransactions[tid].m_mVars[attrName];
        if (sRet.empty())
        {
                string sErrMsg("关键配置为空");
                sErrMsg = sErrMsg + " attrName:" + attrName;
                throw(CTrsExp(ERR_KEY_CONFIG_EMPTY, sErrMsg.c_str()));
        }
        return sRet;
}

string CIdentPub::GetVarConfigNotEmpty(const string & name,const string & attrName)
{
        string sRet;

        if (attrName == "value")
                sRet = g_allVar.GetValue(name);
        else
                sRet = g_allVar.GetValue(name, attrName);

        if (sRet.empty())
        {
                string sErrMsg("关键配置为空");
                sErrMsg = sErrMsg + " attrName:" + attrName;
                throw(CTrsExp(ERR_KEY_CONFIG_EMPTY, sErrMsg.c_str()));
        }
        return sRet;
}

int CIdentPub::OpenFile(const string & filename, ofstream & os,const ios_base::openmode openmode) 
{
     //错误信息
     char errMsg[256] =
     { 0 };

     //目标文件名不可以用/结尾
     if ('/' == filename[filename.length() - 1])
     {
          snprintf(errMsg, 256, "error file name :%s ", filename.c_str());
          throw CTrsExp(ERR_MKDIR_UPLOAD, errMsg);
     }
     for (unsigned int index = 1; index < filename.length(); index++)
     {
          if ('/' == filename[index])
          {
               //得到了一个目录路径
               if (-1 == access(filename.substr(0, index).c_str(), F_OK))
               {
                    //目录不存在，则尝试新建
                    int retcode = mkdir(filename.substr(0, index).c_str(), S_IRWXU
                              | S_IRWXG | S_IRWXO);
                    if (0 != retcode && errno!=EEXIST)
                    {
                         //创建失败，抛出异常
                         ErrorLog("mkdir [%s]", filename.substr(0, index).c_str());
                         snprintf(errMsg, 256, "openfile:%s failed %s,errcode:%d",
                                   filename.substr(0, index).c_str(), strerror(errno),retcode);
                         throw CTrsExp(ERR_MKDIR_UPLOAD, errMsg);
                    }
               }
          }
     }
     //循环结束，根据传入的openmode来操作文件
     os.open(filename.c_str(), openmode);
     if (!os.is_open())
     {
          //创建失败，抛出异常
          snprintf(errMsg, 256, "openfile:%s failed,is_open():false",
                    filename.c_str());
          throw CTrsExp(ERR_MKDIR_UPLOAD, errMsg);
     }
     return 0;
}

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
int CIdentPub::ads_des_idstr_see(char *option,int id,char *instr,int instrlen,char *outstr)
{
    int ilen;
    char tmpstr[1025];

    memset(tmpstr,0x00,sizeof(tmpstr));
    if ((*option) == 'E')
    {
        if(instrlen > 512)
            return -1;
        ilen=ads_des_idstr((char*)"E",id,instr,instrlen,outstr);
        ads_asctohex(outstr,ilen, tmpstr, sizeof(tmpstr));

        strcpy(outstr,tmpstr);
        ilen= strlen(outstr);
    }
    else
    {
        ilen=instrlen/2;
        if(ilen > 512)
            return -1;

        ads_hextoasc(instr,(unsigned char*)tmpstr);
        ilen=ads_des_idstr((char*)"D",id,tmpstr,ilen,outstr);
    }
    return ilen;
}


/*------------------------------------------------------------------------
 *Function Name: Convertrate
 *       Desc: 费率转换，将传入的数字除以10万
 *      Input: 费率，如千分之3.8，则值为380
 *     Return: 返回小数点表示的数字 0.038
 *-----------------------------------------------------------------------*/
string CIdentPub::Convertrate(const string& sNum)
{
    string result = sNum  ;
    int iLen = sNum.length() ;
    int ik=0;
    char numstr[32];

    if(iLen == 1)
    {
        result = "0.0000" + result ;
    }
    if(iLen == 2)
    {
        result = "0.000" + result ;
    }
    if(iLen == 3)
    {
        result = "0.00" + result ;
    }
    if(iLen == 4)
    {
        result = "0.0" + result ;
        
    }
    if(iLen == 5)
    {
        result = "0." + result ;
    }
    if(iLen > 5)
    {
        string subnum = result.substr(0,iLen-5) ;
        subnum +=  "." ;
        result = subnum + result.substr(iLen - 5,5) ;
    }

    strcpy(numstr,result.c_str());
    ik=strlen(numstr); 
    for (int i = ik - 1; i > 0; --i)  
    {  
        if ('\0' == numstr[i])  
        {  
            continue;  
        }  
        else if ('0' == numstr[i])  
        {  
            numstr[i] = '\0';  
        }  
        else if ('.' == numstr[i])// 小数点之后全为零  
        {  
            numstr[i] = '\0';  
            break;  
        }  
        else// 小数点后有非零数字  
        {  
            break;  
        }  
    }  
    result=numstr; 
    return result ;
}

std::string CIdentPub::HEX_TO_ASCII(char* ascii, int asc_len) 
{
    int len = asc_len / 2;
    char* bcd = new char[len+1];
    memset(bcd,0x00,len+1);
    int i=0,j = 0;
    for (i = 0; i < (asc_len + 1) / 2; i++) {
        bcd[i] = hex_to_asc(ascii[j++]);
        bcd[i] = (char) (((j >= asc_len) ? 0x00 : hex_to_asc(ascii[j++])) + (bcd[i] << 4));
    }
    //InfoLog("ascii=%d %d len =%d : %d i=%d\n",strlen(ascii),len,strlen(bcd),sizeof(bcd),i);
    std::string acsii = std::string(bcd,len);
    delete[] bcd;
    
    return acsii;
}

char CIdentPub::hex_to_asc(char asc) 
{
    char bcd;

    if ((asc >= '0') && (asc <= '9'))
        bcd = (char) (asc - '0');
    else if ((asc >= 'A') && (asc <= 'F'))
        bcd = (char) (asc - 'A' + 10);
    else if ((asc >= 'a') && (asc <= 'f'))
        bcd = (char) (asc - 'a' + 10);
    else
        bcd = (char) (asc - 48);
    return bcd;
}

std::string CIdentPub::ASCII_TO_HEX(char *bytes,int len) 
{
    char val;
    char temp[2048]={"\0"};
    for (int i = 0; i < len; i++) {
      val = (char) (((bytes[i] & 0xf0) >> 4) & 0x0f);
      temp[i * 2] = (char) (val > 9 ? val + 'A' - 10 : val + '0');

      val = (char) (bytes[i] & 0x0f);
      temp[i * 2 + 1] = (char) (val > 9 ? val + 'A' - 10 : val + '0');
    }
    return std::string(temp);
}


RSA* createRSA(unsigned char * key,int isPublic)
{
    RSA *rsa= NULL;
    BIO *keybio ;
    keybio = BIO_new_mem_buf(key, -1);
    if (keybio==NULL)
    {
        ErrorLog( "Failed to create key BIO");
        return 0;
    }
    if(isPublic)
    {
        rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa,NULL, NULL);
    }
    else
    {
        rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa,NULL, NULL);
    }
    if(rsa == NULL)
    {
        ErrorLog( "Failed to create RSA");
    }

    return rsa;
}

int CIdentPub::RsaPublicEncrypt(unsigned char * data,int data_len,unsigned char * key, std::string& encrypted)
{
    RSA * rsa = createRSA(key,1);
    if(rsa==NULL) return -1;
    unsigned char encrypt[256]={"\0"};

    int offSet = 0;
    int result = -1;
    int total_len = 0;
    int i = 0;
    // 对数据分段解密
    while (data_len - offSet > 0) {
        if (data_len - offSet > MAX_ENCRYPT_BLOCK) {
            result = RSA_public_encrypt(MAX_ENCRYPT_BLOCK,data+offSet,encrypt,rsa,RSA_PKCS1_PADDING);
        } else {        
            result = RSA_public_encrypt(data_len - offSet,data+offSet,encrypt,rsa,RSA_PKCS1_PADDING);
        }
        if(result== -1 ) return result;
        
        encrypted += std::string((char *)encrypt,result);
        i++;
        offSet = i * MAX_ENCRYPT_BLOCK;
        total_len += result;
    }
    
    return total_len;
}

int CIdentPub::RsaPrivateEncrypt(unsigned char * data,int data_len,unsigned char * key, std::string& encrypted)
{
    RSA * rsa = createRSA(key,0);
    if(rsa==NULL) return -1;

    unsigned char encrypt[256]={"\0"};

    int offSet = 0;
    int result = -1;
    int total_len = 0;
    int i = 0;
    // 对数据分段解密
    while (data_len - offSet > 0) {
        if (data_len - offSet > MAX_ENCRYPT_BLOCK) {
            result = RSA_private_encrypt(MAX_ENCRYPT_BLOCK,data+offSet,encrypt,rsa,RSA_PKCS1_PADDING);
        } else {        
            result = RSA_private_encrypt(data_len - offSet,data+offSet,encrypt,rsa,RSA_PKCS1_PADDING);
        }
        if(result== -1 ) return result;
        
        encrypted += std::string((char *)encrypt,result);
        i++;
        offSet = i * MAX_ENCRYPT_BLOCK;
        total_len += result;
    }
    
    return total_len;
}

int CIdentPub::RsaPrivateDecrypt(unsigned char * enc_data,int data_len,unsigned char * key, std::string& decrypted)
{
    RSA * rsa = createRSA(key,0);
    if(rsa==NULL) return -1;

    unsigned char decrypt[256]={"\0"};

    int offSet = 0;
    int result = -1;
    int total_len = 0;
    int i = 0;
    // 对数据分段解密
    while (data_len - offSet > 0) {
        if (data_len - offSet > MAX_DECRYPT_BLOCK) {
            result = RSA_private_decrypt(MAX_DECRYPT_BLOCK,enc_data+offSet,decrypt,rsa,RSA_PKCS1_PADDING);
        } else {        
            result = RSA_private_decrypt(data_len - offSet,enc_data+offSet,decrypt,rsa,RSA_PKCS1_PADDING);
        }
        if(result== -1 ) return result;
        
        decrypted += std::string((char *)decrypt,result);
        i++;
        offSet = i * MAX_DECRYPT_BLOCK;
        total_len += result;
    }
    
    return total_len;
}

int CIdentPub::RsaPublicDecrypt(unsigned char * enc_data,int data_len,unsigned char * key, std::string& decrypted)
{
    unsigned char decrypt[256]={"\0"};
    RSA * rsa = createRSA(key,1);
    if(rsa==NULL) return -1;
    
    int offSet = 0;
    int result = -1;
    int total_len = 0;
    int i = 0;
    // 对数据分段解密
    while (data_len - offSet > 0) {
        if (data_len - offSet > MAX_DECRYPT_BLOCK) {
            result = RSA_public_decrypt(MAX_DECRYPT_BLOCK,enc_data+offSet,decrypt,rsa,RSA_PKCS1_PADDING);
        } else {        
            result = RSA_public_decrypt(data_len - offSet,enc_data+offSet,decrypt,rsa,RSA_PKCS1_PADDING);
        }
        if(result== -1 ) return result;
        
        decrypted += std::string((char *)decrypt,result);
        i++;
        offSet = i * MAX_DECRYPT_BLOCK;
        total_len += result;
    }
    
    return total_len;
}


static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)  
{  
    std::string* str = dynamic_cast<std::string*>((std::string *)lpVoid);  
    if( NULL == str || NULL == buffer )  
    {  
        return -1;  
    }  
  
    char* pData = (char*)buffer;  
    str->append(pData, size * nmemb);  
    return (size * nmemb);  
} 

int CIdentPub::HttpPost(const std::string & strUrl, const std::string & strPost, std::string & strResponse)  
{  
    CURLcode res;  
    CURL* curl = curl_easy_init();  
    if(NULL == curl)  
    {     
        InfoLog("curl_easy_init fail"); 
        return -1;  
    }  

    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());  
    curl_easy_setopt(curl, CURLOPT_POST, 1);  
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());  
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);  
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);  
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);  
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30);  
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60);  
    res = curl_easy_perform(curl);  
    if(res > 0) 
    {
        InfoLog("curl_easy_perform fail res=%d", res); 
        curl_easy_cleanup(curl);  
        return -1;
    }
    
    curl_easy_cleanup(curl);     
    return res;  
}  

int CIdentPub::HttpESPost(const std::string & strUrl, const std::string & strPost, std::string & strResponse)  
{  
    CURLcode res;  
    CURL* curl = curl_easy_init();  
    if(NULL == curl)  
    {     
        InfoLog("curl_easy_init fail"); 
        return -1;  
    }  
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());  
    curl_easy_setopt(curl, CURLOPT_POST, 1);  
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());  
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);  
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);  
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);  
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30);  
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60);  
    res = curl_easy_perform(curl);  
    if(res > 0) 
    {
        InfoLog("curl_easy_perform fail res=%d", res); 
        curl_easy_cleanup(curl);  
        return -1;
    }
    
    curl_easy_cleanup(curl);     
    return res;  
}  

int CIdentPub::HttpERPtoken(const std::string &strUrl, const std::string &strName, const std::string &strPasswd, std::string &strToken) 
{
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    // 初始化 libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) 
    {
        // 设置请求 URL
        curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());

        // 设置请求方法为 POST
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        // 设置请求头
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // 构建 POST 数据
        char* escapedUsername = curl_easy_escape(curl, strName.c_str(), strName.length());
        char* escapedPassword = curl_easy_escape(curl, strPasswd.c_str(), strPasswd.length());
        
        // 使用 std::string 进行连接
        std::string postData = "username=" + std::string(escapedUsername) + "&password=" + std::string(escapedPassword);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());

        // 设置返回内容到字符串
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);  
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // 执行请求
        res = curl_easy_perform(curl);

        // 检查请求是否成功
        if (res != CURLE_OK) 
        {
            ErrorLog("curl_easy_perform fail res=%d", res); 
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return -1; // 失败
        }

        InfoLog("readBuffer = [%s]",readBuffer.c_str()); 
        // 解析返回的 JSON 数据，假设返回格式为 {"accessToken": "your_token"}
        if (parseRespJsonERP(readBuffer, strToken) == -1)
        {
            ErrorLog("parseRespJsonERP fail"); 
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return -1; // 失败
        }

        // 清理
        curl_free(escapedUsername); // 释放 curl_easy_escape 分配的内存
        curl_free(escapedPassword);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        curl_global_cleanup();

        return 0; // 成功
    }

    return -1; // 初始化失败
}

int CIdentPub::HttpPostERP(const std::string & strUrl, const std::string & strPost, std::string & strResponse,const string &strToken)
{
    CURL* curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1);  
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);  
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse); 

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json"); 
        std::string authHeader = "Authorization: Bearer " + strToken;
        headers = curl_slist_append(headers, authHeader.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers); 

        res = curl_easy_perform(curl);
        if(res != CURLE_OK) 
        {
            ErrorLog("curl_easy_perform fail res=%d", res); 
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return -1; // 请求失败
        }
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return 0;
}

int CIdentPub::HttpPostXml(const std::string & strUrl, const std::string & strPost, std::string & strResponse)  
{  
    CURLcode res;  
    CURL* curl = curl_easy_init();  
    if(NULL == curl)  
    {     
        InfoLog("curl_easy_init fail"); 
        return -1;  
    }  
    struct curl_slist *headers = NULL;
    curl_slist_append(headers, "Content-Type: text/xml");
    curl_slist_append(headers, "charset: GBK");
    curl_slist_append(headers, "Accept: text/xml");

    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str()); 
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POST, 1);  
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());  
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);  
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);  
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);  
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30);  
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60);  
    res = curl_easy_perform(curl);  
    if(res > 0) 
    {
        InfoLog("curl_easy_perform fail res=%d", res); 
        curl_easy_cleanup(curl);  
        return -1;
    }
    
    curl_easy_cleanup(curl);     
    return res;  
}  

int CIdentPub::HttpPostCUC(const std::string & strUrl, const std::string & strPost, std::string & strResponse)  
{  
	CURLcode res;  
	CURL* curl = curl_easy_init();  
	if(NULL == curl)  
	{     
		InfoLog("curl_easy_init fail"); 
		return -1;  
	}  
	char cLength[20];
	int nLen=0;
	nLen = strPost.length();
	memset(cLength,0x00,sizeof(cLength));
	sprintf(cLength,"Content-Length:%d",nLen);
	InfoLog("cLength=%s", cLength); 
	struct curl_slist *headers = NULL;

	//curl_slist_append(headers, "Host:125.77.22.226");
	//curl_slist_append(headers, "Accept: text/xml");
	headers = curl_slist_append(headers, "Content-Type:application/xml;charset=UTF-8");
    headers = curl_slist_append(headers, "Accept: */*");
	//headers = curl_slist_append(headers, cLength);
	headers = curl_slist_append(headers, "Connection:Keep-Alive");
	
	//curl_slist_append(headers, "application/json;charset=UTF-8");
	//curl_slist_append(headers, "Content-Type: text/xml");
	
	

	curl_easy_setopt(curl, CURLOPT_POST, 1); 
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str()); 
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	 
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());  
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);  
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);  
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);  
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30);  
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60);  
	res = curl_easy_perform(curl);  
	if(res > 0) 
	{
		InfoLog("curl_easy_perform fail res=%d", res); 
		curl_easy_cleanup(curl);  
		return -1;
	}

	curl_easy_cleanup(curl);     
	return res;  
}  

int CIdentPub::HttpPostBusiness(const std::string & strUrl, const std::string & strPost, std::string & strResponse)  
{  
	CURLcode res;  
	CURL* curl = curl_easy_init();  
	if(NULL == curl)  
	{     
		InfoLog("curl_easy_init fail"); 
		return -1;  
	}  
	char cLength[20];
	int nLen=0;
	nLen = strPost.length();
	memset(cLength,0x00,sizeof(cLength));
	sprintf(cLength,"Content-Length:%d",nLen);
	InfoLog("cLength=%s", cLength); 
	struct curl_slist *headers = NULL;

	//curl_slist_append(headers, "Host:125.77.22.226");
	//curl_slist_append(headers, "Accept: text/xml");
	headers = curl_slist_append(headers,"Content-Type:application/x-www-form-urlencoded");

	headers = curl_slist_append(headers, "Accept: */*");



	curl_easy_setopt(curl, CURLOPT_POST, 1); 
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str()); 
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());  
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);  
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);  
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);  
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30);  
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60);  
	res = curl_easy_perform(curl);  
	if(res > 0) 
	{
		InfoLog("curl_easy_perform fail res=%d", res); 
		curl_easy_cleanup(curl);  
		return -1;
	}
	curl_easy_cleanup(curl);     
	return res;  
}  

int CIdentPub::HttpGet(const std::string & strUrl, std::string & strResponse)  
{  
    CURLcode res;  
    CURL* curl = curl_easy_init();  
    if(NULL == curl)  
    {  
        InfoLog("curl_easy_init fail"); 
        return -1;  
    }  

    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());  
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);  
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);  
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);  


    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);  
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30);  
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60);  
    res = curl_easy_perform(curl);  
    if(res > 0) 
    {
        InfoLog("curl_easy_perform fail res=%d", res); 
        curl_easy_cleanup(curl);  
        return -1;
    }
    curl_easy_cleanup(curl);  

    return res;  
}  


int CIdentPub::HttpsPost(const std::string & strUrl, const std::string & strPost, std::string & strResponse, const char * pCaPath)  
{  
    CURLcode res;  
    CURL* curl = curl_easy_init();  
    if(NULL == curl)  
    {  
        InfoLog("curl_easy_init fail"); 
        return -1;  
    }  

    curl_easy_setopt(curl, CURLOPT_URL,           strUrl.c_str());  
    curl_easy_setopt(curl, CURLOPT_POST,          1);  
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS,    strPost.c_str());  
    curl_easy_setopt(curl, CURLOPT_READFUNCTION,  NULL);  
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);  
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,     (void *)&strResponse);  
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL,     1);  
    if(NULL == pCaPath)  
    {  
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);  
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);  
    }  
    else  
    {  
        //缺省情况就是PEM，所以无需设置，另外支持DER  
        //curl_easy_setopt(curl,CURLOPT_SSLCERTTYPE,"PEM");  
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);  
        curl_easy_setopt(curl, CURLOPT_CAINFO, pCaPath);  
    }  
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30);  
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60);  
    res = curl_easy_perform(curl);  
    
    if(res > 0) 
    {
        InfoLog("curl_easy_perform fail res=%d", res); 
        curl_easy_cleanup(curl);  
        return -1;
    }
    curl_easy_cleanup(curl);  
    return res;  
}  
  
int CIdentPub::HttpsGet(const std::string & strUrl, std::string & strResponse, const char * pCaPath)  
{  
    CURLcode res;  
    CURL* curl = curl_easy_init();  
    if(NULL == curl)  
    {  
        InfoLog("curl_easy_init fail"); 
        return -1;  
    }  

    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());  
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);  
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);  
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);  
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);  
    if(NULL == pCaPath)  
    {  
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);  
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);  
    }  
    else  
    {  
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);  
        curl_easy_setopt(curl, CURLOPT_CAINFO, pCaPath);  
    }  
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30);  
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60);  
    res = curl_easy_perform(curl);  
    if(res > 0) 
    {
        InfoLog("curl_easy_perform fail res=%d", res); 
        curl_easy_cleanup(curl);  
        return -1;
    }

    curl_easy_cleanup(curl);  
    return res;  
}  



int CIdentPub::RSA_MD5_DEC(string &strRsaPubKey,string &strMD5Key,string &signature,string &instr)
{
    std::string strReqRsaSig; //保存请求过来的rsa校验值，16进制转为asc之后的
    std::string strReqMd5Sig; //保存请求过来的MD5值，16进制的16位大写
    std::string strRspMd5Sig; //保存CGI生成的MD5值，16进制的32位大写

    //将16进制表示的signature还原
    strReqRsaSig = CIdentPub::HEX_TO_ASCII((char *)signature.c_str(),signature.length());

    //用公钥对signature数据解密
    int decrypted_length = CIdentPub::RsaPublicDecrypt((unsigned char * )strReqRsaSig.c_str(),
        strReqRsaSig.length(),(unsigned char *)strRsaPubKey.c_str(),strReqMd5Sig);   
    if(decrypted_length == -1)
    {
        ErrorLog("RSA解密失败");
        //throw CTrsExp(ERR_SIGNATURE_INCORRECT,"解密失败");
        return -1;
    }
    
    InfoLog("RSA解密后:[%s] MD5秘钥:[%s]", strReqMd5Sig.c_str(),strMD5Key.c_str());  

    strRspMd5Sig = Tools::MD5(instr);

    //MD5比对
    if ( memcmp (strRspMd5Sig.c_str(), strReqMd5Sig.c_str(), 16 ) != 0 )
    {
        ErrorLog("MD5不符 平台方-[%s] CGI-[%s]",  strReqMd5Sig.c_str(),strRspMd5Sig.c_str());
        //throw CTrsExp(ERR_SIGNATURE_INCORRECT,"签名验证不通过");
        return -1;
    }

    return 0;
}

int CIdentPub::RSA_MD5_ENC(string &strRsaPubKey,string &strMD5Key,string &instr,string &signature)
{
    std::string strRspMd5Sig; //保存CGI生成的MD5值，16进制的32位大写
    std::string strRspRsaSig; //保存CGI生成的RSA值，原ASC值

    //MD5加密，并取前16位
    strRspMd5Sig = Tools::MD5(instr).substr(0,16);

    //RSA加密，公钥加密
    int encrypted_length = CIdentPub::RsaPublicEncrypt((unsigned char *)strRspMd5Sig.c_str(),strRspMd5Sig.length(),
        (unsigned char *)strRsaPubKey.c_str(),strRspRsaSig);
    if(encrypted_length == -1)
    {
        ErrorLog("RSA公钥加密失败");
        //throw CTrsExp(ERR_SIGNATURE_INCORRECT,"RSA加密失败");
        return -1;
    }  
    //将加密结果转为16进制
    signature = CIdentPub::ASCII_TO_HEX((char *)strRspRsaSig.c_str(),strRspRsaSig.length());
    
    return 0;
}

int CIdentPub::CheckSyscode(std::string &syscode)
{
    if(syscode.length()!=8)
        return -1;

    if (syscode.compare(0,5,"20000") != 0)
        return -1;
    
    if(!ads_isnum((char *)syscode.c_str()))
        return -1;

    return 0;
}

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
int CIdentPub::ads_creid_check( char *creid)
{
     int result=0;
     int i;
     int flag;
     int xishu[17] = {7,9,10,5,8,4,2,1,6,3,7,9,10,5,8,4,2};
     char last[12] = {"10X98765432"};

     for(i = 0;i < 17;i++)
     {
         result += (creid[i]-48)*xishu[i];
     }

     flag = result % 11;

     if(creid[17]=='x')
     {
         if(last[flag] == 'X')
             return 0;
         else
             return -1;

     }
     else
     {
         if(last[flag] == creid[17])
             return 0;
         else
             return -1;
     }
}



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
*   Date: 2017-01-08                                                   
********************************************************************************/
void CIdentPub::ads_datetime_check(string &time_beg,string &time_end,int nday,int div_day)
{
    time_t mcheck_time;

    //1 判断是否有空值，有空值则按nday规则定义默认值
    if(time_beg.empty() || time_end.empty())
    {
        string datenow = CIdentPub::GetDateNow();
        string dateture = CIdentPub::GetDivDay(datenow.c_str(), nday);
        time_beg = dateture + " 00:00:00";
        time_end = dateture + " 23:59:59";
        return ;
    }
        
    //2 判断time_beg格式，根据后台格式增加时分秒
    if(time_beg.length() == 10)
        time_beg = time_beg + " 00:00:00";
    mcheck_time = Tools::genTimeStamp(time_beg.c_str());
    if(mcheck_time == -1)
    {
            ErrorLog("日期格式错误: time_beg=[%s]" ,  time_beg.c_str());
            throw CTrsExp(ERR_DATA_FORMAT,"起始日期格式错误");
    }

    //3 判断time_end格式，根据后台格式增加时分秒
    if(time_end.length() == 10)
        time_end= time_end + " 23:59:59";
    mcheck_time = Tools::genTimeStamp(time_end.c_str());
    if(mcheck_time == -1)
    {
            ErrorLog("日期格式错误: time_beg=[%s]" ,  time_beg.c_str());
            throw CTrsExp(ERR_DATA_FORMAT,"截止日期格式错误");
    }

    //4 判断起始日期不能超过截止时间
    if(strcmp(time_beg.c_str(),time_end.c_str())>=0)
    {
        ErrorLog("起始时间不能超过截止时间: time_beg=[%s],time_end=[%s]",
                       time_beg.c_str(),time_end.c_str());
        throw CTrsExp(ERR_DATA_FORMAT,"起始时间不能超过截止时间");
    }

    //5 判断时间跨度不能超过divday
    string datetmp = CIdentPub::GetDivDay(time_beg.substr(0,10).c_str(), div_day);
    if(memcmp(datetmp.c_str(),time_end.c_str(),10)<0)
    {
        ErrorLog("日期范围不能超过%d天: time_beg=[%s],time_end=[%s]",
                       div_day,time_beg.c_str(),time_end.c_str());
        char tmpbuf[60];
        memset(tmpbuf,0x00,sizeof(tmpbuf));
        sprintf(tmpbuf,"日期范围不能超过%d天",div_day);
        throw CTrsExp(ERR_DATA_FORMAT,tmpbuf);
    }
    return ;
}

//trans_id    16    Y    返回的交流流水号，唯一
//result      2     Y    1：成功  2：失败
//amount      10    Y    单位：分
//app_id      30    C    商户的流水号或者应用号，须保证流水唯一
//signature   16    Y    MD5({trans_id }|{result}|{amount}|{密钥})，取上MD5前16位，大写。然后对16位MD5做RSA加密。

int CIdentPub::PayNotify(CStr2Map& paramap)
{
    CStr2Map contextMap;
    std::string strData;
    std::string  strResponse;

    std::string strSigKey;
    
    std::string strNotifyUrl = paramap["notify_url"];
    //std::string strNotifyUrl = "http://192.168.1.112:8088/gydemo/pay/resultNotify";

    if(CIdentPub::GetSignatureKey(paramap["syscode"],strSigKey))
    {
        ErrorLog("获取签名失败 syscode[%s]", paramap["syscode"].c_str());
        return -1;
    }

    InfoLog("account:%s  trans_id:%s  amount:%s notify_url:%s", 
        (char *)paramap["account"].c_str(),(char *)paramap["trans_id"].c_str(),(char *)paramap["amount"].c_str(),
        (char *)paramap["notify_url"].c_str()); 
    
    /*
    //兼容老接口，华茂通、点点通、摇一摇
    if(0==strcmp("13207494170",(char *)paramap["account"].c_str()) 
        || 0==strcmp("18811844583",(char *)paramap["account"].c_str()) 
        || 0==strcmp("18684667777",(char *)paramap["account"].c_str()) )
    */

    //兼容老接口，华茂通、点点通、国有支付测试
    if(0==strcmp("9205000001",(char *)paramap["account"].c_str())
        || 0==strcmp("9200000002",(char *)paramap["account"].c_str())
        || 0==strcmp("9100000025",(char *)paramap["account"].c_str()))
    {
        //"orderCode=200001&orderId=2016122700432259&respCode=000000&respInfo=交易成功&signature=af5dcc5cc4e9ae75e71a5b25d8d3b964";
        contextMap["orderCode"]="200001";
        contextMap["orderId"]=paramap["trans_id"];
        contextMap["respCode"]="000000";
        contextMap["respInfo"]="success";
        strData = "{" + contextMap["orderCode"]   + "}|{" + 
                     contextMap["orderId"]     + "}|{" + 
                     contextMap["respCode"]     + "}|{" +
                     strSigKey + 
                   "}";

        char md5[33]={"\0"};
        strcpy(md5,Tools::MD5(strData).c_str());
        ads_tolower(md5);
        contextMap["signature"] = md5;   
        std::string postData;
        Tools::MapToStrNoEncode(contextMap,postData);  
        
        InfoLog("md5:[%s] post:[%s][%s]", strData.c_str(),strNotifyUrl.c_str(),postData.c_str()); 
        //发送post通知
        if(-1 == CIdentPub::HttpPost(strNotifyUrl,postData,strResponse))
        {          
            InfoLog("http post fail %s", strNotifyUrl.c_str()); 
            return -1;
        }
    }
    else
    {
         contextMap["trans_id"]=paramap["trans_id"];
         contextMap["result"]=paramap["result"];
         contextMap["amount"]=paramap["amount"];
         contextMap["app_id"]=paramap["app_id"];    
         // 返回数据签名
         strData = "{" + contextMap["trans_id"]   + "}|{" + 
                         contextMap["result"]     + "}|{" + 
                         contextMap["amount"]     + "}|{" +
                         strSigKey + 
                   "}";
                     
         //RSA公钥加密
         std::string strRspRsaSig;
         std::string strRspSig = Tools::MD5(strData).substr(0,16);
         std::string strRsaPubKey;
         if(CIdentPub::GetRsaPublicKey(paramap["syscode"],strRsaPubKey))
         {
             ErrorLog("获取rsa秘钥失败 syscode[%s]", paramap["syscode"].c_str());
             return -1;
         }         
         int encrypted_length = CIdentPub::RsaPublicEncrypt((unsigned char *)strRspSig.c_str(),strRspSig.length(),
             (unsigned char *)strRsaPubKey.c_str(),strRspRsaSig);
         if(encrypted_length == -1)
         {
             InfoLog("RSA公钥加密失败");
             return -1;
         }
         std::string strRspBCDSig = CIdentPub::ASCII_TO_HEX((char *)strRspRsaSig.c_str(),strRspRsaSig.length());
         InfoLog("RSA公钥加密后:%s", strRspBCDSig.c_str()); 
         contextMap["signature"] = strRspBCDSig;
         
         string res_context;
         Tools::MapToStrNoEncode(contextMap,res_context);
         char out_context[10240]= {"\0"};
         ads_base64_enc((char *)res_context.c_str(),strlen(res_context.c_str()),out_context,sizeof(out_context));
         //返回数据json格式
         std::string postData = std::string("{\"context\":\"") + std::string(out_context) +std::string("\"}");

         InfoLog("md5:[%s] post:[%s][%s]", strData.c_str(),strNotifyUrl.c_str(),postData.c_str()); 

         //发送post通知
         if(-1 == CIdentPub::HttpPost(strNotifyUrl,postData,strResponse)){          
            InfoLog("http post fail %s", strNotifyUrl.c_str()); 
            return -1;
         }
    }

    //检查返回内容
    
    InfoLog("http response %s", strResponse.c_str()); 
    if(0!=strncasecmp(strResponse.c_str(),"ok",2) && 0!=strncasecmp(strResponse.c_str(),"success",7))
    {
        //InfoLog("http response %s", strResponse.c_str()); 
        return -1; 
    }       
 
    return 0;   
}



int CIdentPub::parseApiJson(std::string &data, CStr2Map& outMap)
{
    Document vRoot;
    vRoot.Parse<rapidjson::kParseDefaultFlags>((char*)data.c_str());
    if (vRoot.HasParseError())
    {
        InfoLog("json parse fail %s", data.c_str()); 
        return -1;
    }
    
    outMap["syscode"]   = vRoot["syscode"].GetString();
    outMap["version"]   = vRoot["version"].GetString();
    outMap["signature"] = vRoot["signature"].GetString();
    outMap["context"]   = vRoot["context"].GetString();

    return 0;
}


int CIdentPub::OPayNotify(CStr2Map& paramap)
{
    CStr2Map contextMap;
    std::string strData;
    std::string  strResponse;

    std::string strSigKey;
    std::string strRsaPubKey;
    std::string strReqRsaSig;
    std::string strDesKey;
    
    std::string strNotifyUrl = paramap["opay_notify_url"];
    if(CIdentPub::GetSignatureKey(paramap["syscode"],strSigKey))
    {
        ErrorLog("获取签名失败 syscode[%s]", paramap["syscode"].c_str());
        return -1;
    }
    if(CIdentPub::GetRsaPublicKey(paramap["syscode"],strRsaPubKey))
    {
        ErrorLog("获取rsa秘钥失败 syscode[%s]", paramap["syscode"].c_str());
        return -1;
    }   
    if(CIdentPub::GetDESKey(paramap["syscode"],strDesKey))
    {
        ErrorLog("获取des秘钥失败 syscode[%s]", paramap["syscode"].c_str());
        return -1;
    }   

    
    InfoLog("trans_id:%s  amount:%s notify_url:%s", 
        (char *)paramap["trans_id"].c_str(),(char *)paramap["amount"].c_str(),
        (char *)paramap["opay_notify_url"].c_str()); 
    
     contextMap["trans_id"]=paramap["trans_id"];
     contextMap["result"]=paramap["result"];
     contextMap["amount"]=paramap["amount"];
     contextMap["app_id"]=paramap["app_id"];    
     // 返回数据签名
     strData = "{" + contextMap["trans_id"]   + "}|{" + 
                     contextMap["app_id"]     + "}|{" +                     
                     contextMap["result"]     + "}|{" + 
                     contextMap["amount"]     + "}|{" +
                     strSigKey + 
               "}";

     int iRet = CIdentPub::RSA_MD5_ENC(strRsaPubKey,strSigKey,strData,strReqRsaSig);
     if(iRet == -1)
     {
        ErrorLog("签名失败:[%s]", contextMap["trans_id"].c_str());
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"签名失败");
     }
     InfoLog("RSA公钥加密后:%s", strReqRsaSig.c_str()); 

     //加密
     std::string resContext;    
     std::string resContextEnc;
     Tools::MapToStrNoEncode(contextMap, resContext);
     CIdentPub::EncryptDESContext(resContext, strDesKey, resContextEnc);    
     
     //返回数据json格式
     std::string postData = std::string("{\"context\":\"") + resContextEnc + std::string("\",\"signature\":\"") + strReqRsaSig  + std::string("\"}");
     
     InfoLog("md5:[%s] post:[%s][%s]", strData.c_str(),strNotifyUrl.c_str(),postData.c_str()); 

     //发送post通知
     if(-1 == CIdentPub::HttpPost(strNotifyUrl,postData,strResponse)){          
        InfoLog("http post fail %s", strNotifyUrl.c_str()); 
        return -1;
     }



    //检查返回内容
    InfoLog("http response %s", strResponse.c_str()); 
    CStr2Map resMap;
    parseApiJson(strResponse,resMap);
    if(resMap["context"].empty() || resMap["signature"].empty())
    {
        InfoLog("http response fail"); 
        return -1; 
    } 

    std::string strContext;
    if(-1==CIdentPub::DecryptDESContext(resMap["context"], strDesKey, strContext))
    {
        ErrorLog("解密失败");
        return -1;
    }
    Tools::StrToMap(resMap,strContext);
    InfoLog("http response 解密后%s", strContext.c_str());

    //MD5({trans_id}|{app_id}|{success}|{密钥})
    strData = "{" + resMap["trans_id"]       + "}|{" +
                    resMap["app_id"]         + "}|{" +
                    resMap["success"]      + "}|{" +
                    strSigKey +
              "}";
    //signature解密校验
    iRet = CIdentPub::RSA_MD5_DEC(strRsaPubKey,strSigKey,resMap["signature"],strData);
    if(iRet == -1)
    {
        ErrorLog("签名验证不通过");
        return -1;
    }
  
    if(0 != resMap["success"].compare("0000")
        && 0 != resMap["trans_id"].compare(contextMap["trans_id"]))
    {
        InfoLog("http response fail"); 
        return -1; 
    }       
    InfoLog("http response success"); 
 
    return 0;   
}
 


 
void CIdentPub::DelMapF(CStr2Map& dataMap,CStr2Map& outMap)
{
    CStr2Map::const_iterator it = dataMap.begin();
    while(it != dataMap.end())
    {
         string::const_iterator s = it->first.begin();
         if(*s == 'F')
         {
             outMap[it->first.substr(1,it->first.length() - 1)] = it->second;
             ++it;
             continue;
         }
         outMap[it->first] = it->second;
         ++it;
    }
}


/*----------------------------------------------------------------------
*  Function Name: CheckPhone
*         Desc:校验手机号的有效性
*  Input:                                                               *
*    std::string phone    : 手机号                                   *
*  Return: -1 失败  0 成功                                              *
*-----------------------------------------------------------------------*/  
int CIdentPub::CheckPhone(string phone)
{
    if(!ads_isnum((char *)phone.c_str()) || phone.length() != 11)
    {
        ErrorLog("手机号不正确: phone=[%s]",phone.c_str());
        return (-1);
    }
    return 0;
}


/*----------------------------------------------------------------------
*  Function Name: CheckUid
*         Desc:校验手机号的有效性
*  Input:                                                               *
*    std::string uid    : 商户号                                   *
*  Return: -1 失败  0 成功                                              *
*-----------------------------------------------------------------------*/  
int CIdentPub::CheckUid(string uid)
{
    if(!ads_isnum((char *)uid.c_str()) || uid.length() != 10)
    {
        ErrorLog("商户号不正确: uid=[%s]",uid.c_str());
        return (-1);
    }
    return 0;
}


/*----------------------------------------------------------------------
*  Function Name: CheckTransID
*         Desc:校验流水号的有效性
*  Input:                                                               *
*    std::string trans_id    : 流水号                                   *
*  Return: -1 失败  0 成功                                              *
*-----------------------------------------------------------------------*/  
void CIdentPub::CheckTransID(string trans_id)
{
    if(!ads_isnum((char *)trans_id.c_str()) || trans_id.length() != 16)
    {
        ErrorLog("流水号不正确: trans_id=[%s]",trans_id.c_str());
        throw(CTrsExp(ERR_GET_DEPT, "流水号不正确"));
        //return (-1);
    }

    int year = atoi(trans_id.substr(0,4).c_str());
    int month = atoi(trans_id.substr(4,2).c_str());
    int day = atoi(trans_id.substr(6,2).c_str());

    if( year < 2017 || month > 12 || month < 1 || day > 31 || day < 1 )
    {
         ErrorLog("流水号不正确: trans_id=[%s]",trans_id.c_str());
         throw(CTrsExp(ERR_GET_DEPT, "流水号不正确"));
         //return (-1);
    }
}


/*----------------------------------------------------------------------
*  Function Name: CheckContractID
*         Desc:校验签约号的有效性
*  Input:                                                               *
*    std::string contract_id    : 签约号                                   *
*  Return: -1 失败  0 成功                                              *
*-----------------------------------------------------------------------*/  
int CIdentPub::CheckContractID(string contract_id)
{
    if(!ads_isnum((char *)contract_id.c_str()) || contract_id.length() != 16)
    {
        ErrorLog("签约号非法: contract_id=[%s]",contract_id.c_str());
        return (-1);
    }

    int year = atoi(contract_id.substr(0,4).c_str());
    int month = atoi(contract_id.substr(4,2).c_str());
    int day = atoi(contract_id.substr(6,2).c_str());

    if( year < 2016 || month > 12 || month < 1 || day > 31 || day < 1 )
    {
         ErrorLog("签约号非法: contract_id=[%s]",contract_id.c_str());
         return (-1);
    }

    return 0;
}



/*----------------------------------------------------------------------
*  Function Name: CheckIDCard
*         Desc:校验身份证号的有效性
*  Input:                                                               *
*    std::string id_card    : 身份证号                                   *
*  Return: -1 失败  0 成功                                              *
*-----------------------------------------------------------------------*/ 
int CIdentPub::CheckIDCard(string id_card)
{
    if(id_card.length() != 18)                   //先判断长度一定要是18位，前17位是数字，最后一位是x或者X
    {
        ErrorLog("身份证号码必须是18位,且此身份证号码=[%s]",id_card.c_str());
        return (-1);
    }
    if(!ads_isnum(const_cast<char*>(id_card.substr(0,17).c_str())))
    {
        ErrorLog("身份证号码的前17位必须均是数字,且此身份证号码的前17位=[%s]",id_card.substr(0,17).c_str());
        return (-1);
    }
    if(!ads_isnum(const_cast<char*>(id_card.substr(17,1).c_str())) && id_card.substr(17,1) != "X" && id_card.substr(17,1) != "x")
    {
        ErrorLog("身份证号码的第18位必须是数字或X或x,且此身份证号码=[%s]",id_card.c_str());
        return (-1);
    }
    if(id_card.substr(17,1) == "x")
    {
        id_card= id_card.substr(0,17) + "X";
        DebugLog("小写的x转化为X后的身份证号码=[%s]",id_card.c_str());
    }
    if(CIdentPub::ads_creid_check(const_cast<char*>(id_card.c_str())) == -1)    //判断输入身份证号是否正确
    {
        ErrorLog("身份证号码格式有错误,且此身份证号码=[%s]",id_card.c_str());
        return (-1);
    }

    return 0;
}




/*----------------------------------------------------------------------
*  Function Name: CheckBankno
*         Desc:校验银行卡号的有效性
*  Input:                                                               *
*    std::string bank_no    : 银行卡号                                   *
*  Return: -1 失败  0 成功                                              *
*-----------------------------------------------------------------------*/   
int CIdentPub::CheckBankno(string bank_no)
{
    if(!ads_isnum((char *)bank_no.c_str()) || bank_no.length() < 10)
    {
        ErrorLog("银行卡号不正确: bank_no=[%s]",bank_no.c_str());
        return (-1);
    }
    
    return 0;
}


/*----------------------------------------------------------------------
 * *  Function Name: CheckAmount
 * *         Desc:校验金额的有效性
 * *  Input:                                                               *
 * *    std::string amount    : 金额                                   *
 * *  Return: -1 失败  0 成功                                              *
 * *-----------------------------------------------------------------------*/
int CIdentPub::CheckAmount(string amount)
{
    if(!ads_isnum((char *)amount.c_str()) || amount.length() > 10)
    {
        ErrorLog("金额不正确: amount=[%s]",amount.c_str());
        return (-1);
    }
    
    return 0;
}


/*----------------------------------------------------------------------
*  Function Name: CheckNumDot
*         Desc:校验金额是包含数字和小数点的字符
*  Input:                                                               *
*    std::string amount    : 金额                                   *
*  Return: -1 失败  0 成功                                              *
*-----------------------------------------------------------------------*/   
int CIdentPub::CheckNumDot(string amount)
{
    int j = 1;
    if(amount.length() > 10)
    {
        ErrorLog("金额非法: amount=[%s]",amount.c_str());
        return (-1);       
    }
 
    std::string::iterator it=amount.begin();

    if(!(*it>='0'&&*it<='9'))
    {
        ErrorLog("金额非法: amount=[%s]",amount.c_str());
        return (-1);
    }

    ++it;

    for ( ; it!=amount.end(); ++it)
    {
        j++;
        if(*it!='.'&&!(*it>='0'&&*it<='9'))
        {
            ErrorLog("金额非法: amount=[%s]",amount.c_str());
            return (-1);
        }
        if(*it=='.'&&(amount.length()-j>2))
        {
            ErrorLog("金额非法: amount=[%s]",amount.c_str());
            return (-1);
        }
    }
 
    return 0;
}


/*----------------------------------------------------------------------
*  Function Name: ContractDecode
*         Desc:签约号解密 
*  Input:                                                               *
*    std::string contract    : 签约号                                   *
*  Return: 解密后的签约号                                              *
*-----------------------------------------------------------------------*/   
std::string CIdentPub::ContractDecode(std::string &contract)
{
    char contract_id[16+1]={"\0"};
    CIdentPub::ads_des_idstr_see((char*)"D",2,(char*)contract.c_str(),contract.length(),contract_id);
    return string(contract_id);
}

/*----------------------------------------------------------------------
*  Function Name: ContractEncode
*         Desc:签约号加密 
*  Input:                                                               *
*    std::string contract    : 签约号                                   *
*  Return: 加密后的签约号                                              *
*-----------------------------------------------------------------------*/   
std::string CIdentPub::ContractEncode(std::string &contract)
{
    char contract_id[32+1]={"\0"};
    CIdentPub::ads_des_idstr_see((char*)"E",2,(char*)contract.c_str(),contract.length(),contract_id);
    return string(contract_id);
}


/********************************************************************************
 * *  Func: CheckAppId(string &app_id)
 * *  Input:                                                                       
 * *    app_id: 输入app_id
 * *  Description:                                                         
 * *    (1)判断用户的流水号是否合法
 * *  Return:                                                              
 * *    正确返回: 0
 *      错误返回: -1
 * *   Auth: BaoMingKe                                                     
 * *   Date: 2017-04-21                                                   
 * ********************************************************************************/
int CIdentPub::CheckAppId(std::string app_id)
{
    if(!ads_isnum((char *)app_id.substr(0,8).c_str()) || app_id.length()>30)
    {
        ErrorLog("商户流水号非法: app_id=[%s]",app_id.c_str());
        return (-1);
    }

    char localDate[8+1];
    memset(localDate,0x00,sizeof(localDate));
    ads_getdate(localDate);
    if(ads_diff_day((char *)app_id.substr(0,8).c_str(),localDate) > 30)
    {
        ErrorLog("商户流水号非法: app_id=[%s]",app_id.c_str());
        return (-1);
    }

    return 0;
}


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
int CIdentPub::DecryptDESContext(std::string ciphertext, std::string deskey, std::string& plaintext)
{
    int data_len = ciphertext.length()*2;
    
    char *base64Decrypt = new char[data_len+1];
    if( NULL == base64Decrypt )
    {
        InfoLog("base64Decrypt malloc fail ");
        return -1;
    }
    memset(base64Decrypt,0x00,data_len+1);
    
    char *desDecrypt = new char[data_len+1];
    if( NULL == desDecrypt )
    {
        InfoLog("desDecrypt malloc fail ");
        delete[] base64Decrypt;
        return -1;
    }
    memset(desDecrypt,0x00,data_len+1);

    int len = ads_base64_dec((char *)ciphertext.c_str(),ciphertext.length(),base64Decrypt,data_len+1);    
    if(0 == len)
    {
        InfoLog("base64 fail len=%d,data_len=%d,ciphertext=%s", len,data_len,ciphertext.c_str());
        delete[] base64Decrypt;
        delete[] desDecrypt;    
        return -1;
    }   

    int des_len = ads_3des_string((char *)"D",(unsigned char*)deskey.c_str(),base64Decrypt,len,desDecrypt);  
    if(0 >= des_len)
    {
        InfoLog("3des fail des_len=%d,len=%d ", des_len,len);
        delete[] base64Decrypt;
        delete[] desDecrypt;    
        return -1;
    }   

    plaintext = desDecrypt;

    delete[] base64Decrypt;
    delete[] desDecrypt;    
    return 0;
}

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
int CIdentPub::EncryptDESContext(std::string plaintext, std::string deskey, std::string& ciphertext)
{
	int data_len = (1+plaintext.length()/8)*8*2;

	char *desEncrypt = new char[data_len+1];
	if(NULL == desEncrypt)
	{
		InfoLog("desEncrypt malloc fail ");
		return -1;
	}

	char *base64Encrypt = new char[data_len+1];
	if(NULL == base64Encrypt)
	{
		InfoLog("base64Encrypt malloc fail ");
		delete[] desEncrypt;
		return -1;
	}
	memset(desEncrypt,0x00,data_len+1);
	memset(base64Encrypt,0x00,data_len+1);

	int len = ads_3des_string((char *)"E",(unsigned char*)deskey.c_str(),(char *)plaintext.c_str(),plaintext.length(),desEncrypt); 
	if(0 >= len)
	{
		InfoLog("3des fail len=%d ", len);
		delete[] base64Encrypt;
		delete[] desEncrypt;    
		return -1;
	}   

	ads_base64_enc(desEncrypt,len,base64Encrypt,data_len);
	ciphertext = base64Encrypt;

	delete[] base64Encrypt;
	delete[] desEncrypt; 
	return 0;
}


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
/*
&#x0000;
&#00000;
这种编码称为 HTML 编码。HTML 编码有两种形式：
第一种是 Unicode 代码点的十六进制表示，后一种是 Unicode 代码点的十进制表示。
*/
int CIdentPub::unicode2gbk(char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
    unsigned short gbKey;
    unsigned short uniKey;
    size_t i,j;
    char stemp[50];

    if(inlen > 2040)
        return (-1);
    if(inlen > outlen)
        return (-1);

    i=0;
    j=0;
    for(;i<inlen;)
    {
       if(inbuf[i] != 0x5c)
       {
           outbuf[j]=inbuf[i];
           j++;
           i++;
       }
       else
       {
           i++;
           if(inbuf[i] == 'u')
           {
               i++;
               memcpy(stemp,"0x",2);
               memcpy(stemp,&inbuf[i],4);
               stemp[6]='\0';

               uniKey = strtol(stemp, NULL, 16);
               gbKey=SearchCodeTable( uniKey );
               gbKey = (gbKey >> 8) | (gbKey << 8);
               memcpy(&outbuf[j],&gbKey,2);
               i=i+4;
               j=j+2;

           }
           else if(inbuf[i] == '/')
           {
               outbuf[j]=inbuf[i];
               j++;
               i++;
           }
           else 
           {
               outbuf[j]= 0x5c;
               j++;
               outbuf[j]=inbuf[i];
               j++;
               i++;
           }
       }
    }
   
    outbuf[j]= 0x00;
    return j;
}

/*------------------------------------------------------------------------
 *Function Name: gbk2unicode
 *       Desc: 
 *             转码函数 ---  GBK 转为 "\u5bb6\u7528\u7535\u5668" 
 *      Input: 待转码串，待转码串长度，转码结果空间，转码结果空间大小
 *     Return: 转码后长度
 *       Auth: JiaYeHui
 *       Date: 2018-06-08
 *-----------------------------------------------------------------------*/
int CIdentPub::gbk2unicode(char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
    int byteCount1 = 0;
    size_t i1 = 0;
    size_t j1 = 0;
    char buf[8];

    unsigned short unicodeKey1 = 0;
    unsigned short gbKey1 = 0;
    memset(buf,0x00,sizeof(buf));

    if(inlen > 2040)
        return (-1);
    if(inlen > outlen)
        return (-1);
       
    while(i1 < inlen)
    {  
        if(j1 > (outlen-7))
            return (-1);
        if ((unsigned char)inbuf[i1]<0x80)
        {
             outbuf[j1] = inbuf[i1];
             byteCount1 = 1;
        }
        else
        {
            byteCount1 = 2;
            outbuf[j1]   = inbuf[i1+1];
            outbuf[j1+1] = inbuf[i1];
             
            memcpy(&gbKey1, (outbuf + j1), 2);
            unicodeKey1 = SearchCodeTable1(gbKey1);

            if (unicodeKey1 != 0)
            {
                sprintf(outbuf+j1, "\\u%04X", unicodeKey1);  
                byteCount1 = 6;
            }
        }

        j1 += byteCount1;
        if(byteCount1 == 1)
            i1++;
        else
            i1 += 2;
    }

    return j1;
}
/*------------------------------------------------------------------------
 *Function Name: json2http
 *       Desc: 
 *        将":" json串转成&=这样的格式     
 *      Input: 
 *        待转码串 {"respDesc":"订单号重复","returnUrl":"http://www.jfbill.com/cgi-bin/cdh_in_amount.cgi","transAmt":1000,"commodityName":"电器"}
 *        待转码串长度
 *        转码结果空间 respDesc=订单号重复&returnUrl=http://www.jfbill.com/cgi-bin/cdh_in_amount.cgi&transAmt=1000&commodityName=电器
 *        转码结果空间大小
 *     Return: 转码后长度
 *       Auth: JiaYeHui
 *       Date: 2017-04-28                                                   
 *-----------------------------------------------------------------------*/
int CIdentPub::json2http(char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
    size_t i,j;

    if(inlen > 2040)
        return (-1);
    if(inlen > outlen)
        return (-1);

    i=0;
    j=0;
    for(;i<inlen;) //每个字段一个for循环
    {
       if(inbuf[i] != '"') 
       {
           i++;
       }
       else //开始处理 1个字段
       {
           i++;
           if(j > 0)
           {
              outbuf[j]='&';
              j++;
           }
           //第一列
           while ( inbuf[i]!= '"' && i<inlen )
           {
              outbuf[j]=inbuf[i];
              j++;
              i++;
          }
          if(i<inlen);
               i++;
           //第二列
          if(inbuf[i] == ':' && i<inlen)
          {
               i++;
               outbuf[j]='=';
               j++;
               if(inbuf[i] == '"' && i<inlen)
               {
                   i++;
                   while ( inbuf[i]!= '"' && i<inlen )
                   {
                      outbuf[j]=inbuf[i];
                      j++;
                      i++;
                   }
                   i++;
               }
               else if(i<inlen)
               {
                   outbuf[j]=inbuf[i];
                   j++;
                   i++;
                   while ( inbuf[i] != ',' && i<inlen )
                   {
                      if(inbuf[i] != '}')
                      {
                          outbuf[j]=inbuf[i];
                          j++;
                          i++;
                      }
                   }
                   i++;
                }
            }
       }
   } //for

   outbuf[j]= 0x00;
   return j;
}

/*------------------------------------------------------------------------
 *Function Name: httpjson2map
 *       Desc: 
 *        将":" json串转成map
 *      Input: 
 *        待转码串 {"respDesc":"订单号重复","returnUrl":"http://www.jfbill.com/cgi-bin/cdh_in_amount.cgi","transAmt":1000,"commodityName":"电器"}
 *        待转码串长度
 *        转码结果Map
 *     Return: -1失败 0成功
 *       Auth: JiaYeHui
 *       Date: 2017-04-28                                                   
 *-----------------------------------------------------------------------*/
int CIdentPub::httpjson2map(char *inbuf, size_t inlen, CStr2Map& outMap)
{
    size_t i,j,k,l_1,l_2;
    char stemp_1[128];
    char stemp_2[2048];

    if(inlen > 2040)
        return (-1);

    i=0;
    for(;i<inlen;) //每个字段一个for循环
    {
       j=0;
       k=0;
       l_1=0;
       l_2=0;
       if(inbuf[i] != '"') 
       {
           i++;
       }
       else //开始处理 1个字段
       {
           i++;
           //第一列
           while ( inbuf[i]!= '"' && i<inlen )
           {
              stemp_1[j]=inbuf[i];
              j++;
              i++;
              l_1=1;
          }
          stemp_1[j]=0x00;

          if(i<inlen);
               i++;

          while((i<inlen)&&(inbuf[i] == ' '))
          {
             i++;
          }

           //第二列
          if(inbuf[i] == ':' && i<inlen)
          {
               i++;
               while((i<inlen)&&(inbuf[i] == ' '))
               {
                  i++;
               }
               if(inbuf[i] == '"' && i<inlen)
               {
                   i++;
                   while ( inbuf[i]!= '"' && i<inlen )
                   {
                      l_2=1;
                      stemp_2[k]=inbuf[i];
                      k++;
                      i++;
                   }
                   stemp_2[k]=0x00;
                   i++;
               }
               else if(i<inlen)
               {
                   l_2=1;
                   stemp_2[k]=inbuf[i];
                   k++;
                   i++;
                   while ( inbuf[i] != ',' && i<inlen )
                   {
                      if(inbuf[i] != '}')
                      {
                          stemp_2[k]=inbuf[i];
                          k++;
                      }
                      i++;
                   }
                   stemp_2[k]=0x00;
                   i++;
                }
           }
           if((l_1==1) && (l_2==1))
           {
               outMap[stemp_1]=stemp_2;
           }
       }
   } //for

   stemp_1[j]= 0x00;
   return 0;
}

//报文统一解析  只在接口使用
int CIdentPub::parseRespJson(std::string &data, CStr2Map& outMap)
{
    Document vRoot;
    vRoot.Parse<rapidjson::kParseDefaultFlags>((char*)data.c_str());
    if (vRoot.HasParseError())
    {
        InfoLog("json parse fail %s", data.c_str());
        return -1;
    }

    outMap["syscode"]   = vRoot["syscode"].GetString();
    outMap["version"]   = vRoot["version"].GetString();
    outMap["signature"] = vRoot["signature"].GetString();
    outMap["context"]   = vRoot["context"].GetString();

    return 0;
}

int CIdentPub::parseRespJsonERP(std::string &data, string& outToken)
{
    rapidjson::Document document;
    // 解析JSON
    if (document.Parse(data.c_str()).HasParseError())
    {
        ErrorLog( "JSON parse error: %s" ,data.c_str());
        return -1; // 解析错误
    }
    if (document.HasMember("result") && document["result"].IsBool() && document["result"].GetBool())
    {
        if (document.HasMember("data") && document["data"].IsObject())
        {
            if (document["data"].HasMember("accessToken") && document["data"]["accessToken"].IsString())
            {
                outToken = document["data"]["accessToken"].GetString();
                return 0;
            }
            else
            {
                ErrorLog( "accessToken not found in data" );
                return -1; // 失败
            }
        }
        else
        {
            ErrorLog( "result is false or not found" );
            return -1;
        }
    }
    return -1;

}

int CIdentPub::OPayNotifyM(CStr2Map& paramap)
{
    CStr2Map contextMap;
    CStr2Map contextReqMap,contextResMap;
    std::string strData;
    std::string  strResponse;

    std::string strSigKey;
    std::string strRsaPubKey;
    std::string strReqRsaSig;
    std::string strDesKey;
    
    std::string strNotifyUrl = paramap["opay_notify_url"];
    if(CIdentPub::GetSignatureKey(paramap["syscode"],strSigKey))
    {
        ErrorLog("获取签名失败 syscode[%s]", paramap["syscode"].c_str());
        return -1;
    }
    if(CIdentPub::GetDESKey(paramap["syscode"],strDesKey))
    {
        ErrorLog("获取des秘钥失败 syscode[%s]", paramap["syscode"].c_str());
        return -1;
    }   

    InfoLog("trans_id:[%s] amount:[%s] notify_url:[%s]", 
        (char *)paramap["trans_id"].c_str(),(char *)paramap["amount"].c_str(), (char *)paramap["opay_notify_url"].c_str()); 
    
    //对返回数据做DES加密,放入context字段中
    contextReqMap["trans_id"]      = paramap["trans_id"];
    contextReqMap["app_id"]        = paramap["app_id"];
    contextReqMap["result"]        = paramap["result"];
    contextReqMap["amount"]        = paramap["amount"];
    
    //返回
    std::string resContext;    
    std::string resContextEnc;
    Tools::MapToStrNoEncode(contextReqMap, resContext);
    CIdentPub::EncryptDesUrlbase64(resContext, strDesKey, resContextEnc);    

    strData = "{" + contextReqMap["trans_id"]     + "}|{" + 
                    contextReqMap["app_id"]       + "}|{" + 
                    contextReqMap["result"]       + "}|{" +
                    contextReqMap["amount"]       + "}|{" +
                    strSigKey + 
              "}";
                 
    std::string strReqSignature;
    strReqSignature = Tools::MD5(strData).substr(0,16);
    InfoLog("代付回调通知-MD5-strData[%s] MD5-result[%s]",strData.c_str(),strReqSignature.c_str()); 
    InfoLog("代付回调通知-context[%s]",resContext.c_str()); 
    //构造http 请求json
    std::string strUrl;
    std::string postData;

    CStr2Map reqMap;
    std::string strPostData;
    reqMap["syscode"]   = paramap["syscode"];
    reqMap["version"]   = "002";
    reqMap["signature"] = strReqSignature;
    reqMap["context"]   = resContextEnc;
    //makeReqJson(reqMap, strPostData);
    strPostData =  "syscode=" + reqMap["syscode"] +"&version=" + reqMap["version"] + "&context=" + reqMap["context"] + "&signature=" + reqMap["signature"];
    InfoLog("代付回调通知-post:[%s][%s]",strNotifyUrl.c_str(),strPostData.c_str()); 

    //发送post通知
    if(-1 == CIdentPub::HttpPost(strNotifyUrl,strPostData,strResponse))
    {          
        InfoLog("代付回调通知-http post fail %s", strNotifyUrl.c_str()); 
        return -1;
    }
    InfoLog("代付回调通知-http response[%s]", strResponse.c_str()); 
    
    //判断长度
    if(strResponse.length() < 30)
    {
        ErrorLog("代付回调通知-返回异常 长度[%d]", strResponse.length());
        return(-1);
    }

    //处理响应
    CStr2Map respMap,respConextMap;     
    if(-1 == CIdentPub::parseRespJson(strResponse,respMap))
    {
        ErrorLog("代付回调通知-返回数据格式有误");
        return (-1);
    }
    
    //解密 
    string strContext;
    if(-1==CIdentPub::DecryptDesUrlbase64(respMap["context"], strDesKey, strContext))
    {
        ErrorLog("代付回调通知-返回报文解密失败");
        return (-1);
    }
    InfoLog("代付回调通知-urlbase-des解密结果:[%d] [%s]",strContext.length(),strContext.c_str());
    Tools::StrToMap(contextResMap,strContext);
    
    strData = "{" + contextResMap["trans_id"]     + "}|{" + 
                    contextResMap["app_id"]       + "}|{" + 
                    contextResMap["return_code"]  + "}|{" +
                    strSigKey + 
              "}";

    //signature解密校验
    string strRspSig;
    strRspSig = Tools::MD5(strData);

    if(respMap["signature"].empty())
    {
        if( memcmp(strRspSig.c_str(), contextResMap["signature"].c_str(), 16 ) != 0 )
        {
            ErrorLog("代付回调通知-签名验证不通过 计算-[%s] 传入-[%s] md5data-[%s]",
                      strRspSig.c_str(),contextResMap["signature"].c_str(),strData.c_str());
            return (-1);
        }
    }
    else if( memcmp(strRspSig.c_str(), respMap["signature"].c_str(), 16 ) != 0 )
    {
        ErrorLog("代付回调通知-签名验证不通过 计算-[%s] 传入-[%s] md5data-[%s]",
                  strRspSig.c_str(),contextResMap["signature"].c_str(),strData.c_str());
        return (-1);
    }

    if((contextResMap["return_code"].compare("0000") != 0) && (0 != contextResMap["trans_id"].compare(contextMap["trans_id"])))
    {
        ErrorLog("代付回调通知-商户接收通知失败 return_code=[%s]",contextResMap["return_code"].c_str());
        return (-1);
    }
        
    InfoLog("代付回调通知-http response success"); 
 
    return 0;   
}
 

//trans_id    16    Y    返回的交流流水号，唯一
//result      2     Y    1：成功  2：失败
//amount      10    Y    单位：分
//app_id      30    C    商户的流水号或者应用号，须保证流水唯一
//signature   16    Y    MD5({trans_id }|{result}|{amount}|{密钥})，取上MD5前16位，大写。然后对16位MD5做RSA加密。
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
int CIdentPub::JfpNotifyMd5(CStr2Map& paramap)
{
    CStr2Map contextMap;
    std::string strData;
    std::string  strResponse;

    std::string strSigKey;
    
    std::string strNotifyUrl = paramap["notify_url"];
    //std::string strNotifyUrl = "http://192.168.1.112:8088/gydemo/pay/resultNotify";

    if(CIdentPub::GetSignatureKey(paramap["syscode"],strSigKey))
    {
        ErrorLog("获取签名失败 syscode[%s]", paramap["syscode"].c_str());
        return -1;
    }

    InfoLog("account:%s  trans_id:%s  amount:%s notify_url:%s", 
        (char *)paramap["account"].c_str(),(char *)paramap["trans_id"].c_str(),
        (char *)paramap["amount"].c_str(),(char *)paramap["notify_url"].c_str()); 
    
     contextMap["trans_id"]=paramap["trans_id"];
     contextMap["result"]=paramap["result"];
     contextMap["amount"]=paramap["amount"];
     contextMap["app_id"]=paramap["app_id"];    
     // 返回数据签名
     strData = "{" + contextMap["trans_id"]   + "}|{" + 
                     contextMap["result"]     + "}|{" + 
                     contextMap["amount"]     + "}|{" +
                     strSigKey + 
               "}";

     //MD5加密
     std::string strRspSig = Tools::MD5(strData).substr(0,16);
     contextMap["signature"] = strRspSig;
     
     string res_context;
     Tools::MapToStrNoEncode(contextMap,res_context);
     InfoLog("context base64 pre:[%s]", res_context.c_str()); 
     char out_context[10240]= {"\0"};
     ads_base64_enc((char *)res_context.c_str(),strlen(res_context.c_str()),out_context,sizeof(out_context));
     //返回数据json格式
     //std::string postData = std::string("{\"context\":\"") + std::string(out_context) +std::string("\"}");
     std::string postData = std::string("context=") + std::string(out_context);

     InfoLog("md5:[%s] post:[%s][%s]", strData.c_str(),strNotifyUrl.c_str(),postData.c_str()); 

     //发送post通知
     if(-1 == CIdentPub::HttpPost(strNotifyUrl,postData,strResponse)){          
        InfoLog("http post fail %s", strNotifyUrl.c_str()); 
        return -1;
     }

    //检查返回内容
    InfoLog("http response %s", strResponse.c_str()); 
    if(0!=strncasecmp(strResponse.c_str(),"ok",2) && 0!=strncasecmp(strResponse.c_str(),"success",7))
    {
        //InfoLog("http response %s", strResponse.c_str()); 
        return -1; 
    }       
 
    return 0;   
}

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
int CIdentPub::AesEncrypt(unsigned char *key,unsigned char* in, int len, unsigned char* out)
{
    unsigned char iv[AES_BLOCK_SIZE];//加密的初始化向量
    AES_KEY aes;
    int i;

    if(!in || !key || !out)
        return 0;
    if(len <1)
        return 0;

    //iv一般设置为全0,可以设置其他，但是加密解密要一样就行
          for(i=0; i<AES_BLOCK_SIZE; ++i)
        iv[i]=key[i];
    if(AES_set_encrypt_key((unsigned char*)key, 128, &aes) < 0)
        return 0;

    AES_cbc_encrypt((unsigned char*)in, (unsigned char*)out, len, &aes, iv, AES_ENCRYPT);
    return 1;
}

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
int CIdentPub::AesDecrypt(unsigned char* key,unsigned char* in,int len,unsigned char* out)
{
    unsigned char iv[AES_BLOCK_SIZE];//加密的初始化向量
    AES_KEY aes;
    int i;

    if(!in || !key || !out)
        return 0;

    //iv一般设置为全0,可以设置其他，但是加密解密要一样就行
    for(i=0; i<AES_BLOCK_SIZE; ++i)
        iv[i]=key[i];
    if(AES_set_decrypt_key((unsigned char*)key, 128, &aes) < 0)
    {
        return 0;
    }
    AES_cbc_encrypt((unsigned char*)in, (unsigned char*)out, len, &aes, iv, AES_DECRYPT);
    return 1;
}


/********************************************************************************
 * *  Func: DecryptDesUrlbase64(std::string ciphertext, std::string deskey, std::string& plaintext)
 * *  Input:                                                                       
 * *    ciphertext: 密文
 * *    deskey: des秘钥 
 * *    plaintext: 解密结果明文
 * *  Description:                                                         
 * *    (1)3des解密
 * *  Return:                                                              
 * *    正确返回: 0
 *      错误返回: -1
 * *   Auth: JiaYeHui
 * *   Date: 2017-08-18                                                  
 * ********************************************************************************/
int CIdentPub::DecryptDesUrlbase64(std::string ciphertext, std::string deskey, std::string& plaintext)
{
    int data_len = ciphertext.length()*2;
    
    char *base64Decrypt = new char[data_len+1];
    if( NULL == base64Decrypt )
    {
        InfoLog("base64Decrypt malloc fail ");
        return -1;
    }
    memset(base64Decrypt,0x00,data_len+1);
    
    char *desDecrypt = new char[data_len+1];
    if( NULL == desDecrypt )
    {
        InfoLog("desDecrypt malloc fail ");
        delete[] base64Decrypt;
        return -1;
    }
    memset(desDecrypt,0x00,data_len+1);

    int len = ads_urlsafe_base64_dec((char *)ciphertext.c_str(),ciphertext.length(),base64Decrypt,data_len+1);    
    if(0 == len)
    {
        InfoLog("base64 fail len=%d,data_len=%d,ciphertext=%s", len,data_len,ciphertext.c_str());
        delete[] base64Decrypt;
        delete[] desDecrypt;    
        return -1;
    }   

    int des_len = ads_3des_string((char *)"D",(unsigned char*)deskey.c_str(),base64Decrypt,len,desDecrypt);  
    if(0 >= des_len)
    {
        InfoLog("3des fail des_len=%d,len=%d ", des_len,len);
        delete[] base64Decrypt;
        delete[] desDecrypt;    
        return -1;
    }   

    plaintext = desDecrypt;

    delete[] base64Decrypt;
    delete[] desDecrypt;    
    return 0;
}
/********************************************************************************
 * *  Func: EncryptDesUrlbase64(std::string plaintext, std::string deskey, std::string& ciphertext)
 * *  Input:  
 * *    plaintext: 明文
 * *    deskey: des秘钥 
 * *    ciphertext: 加密结果密文
 * *  Description:                                                         
 * *    (1)3des加密
 * *  Return:                                                              
 * *    正确返回: 0
 *      错误返回: -1
 * *   Auth: JiaYeHui
 * *   Date: 2017-08-18                                                  
 * ********************************************************************************/
int CIdentPub::EncryptDesUrlbase64(std::string plaintext, std::string deskey, std::string& ciphertext)
{
    int data_len = (1+plaintext.length()/8)*8*2;
    
    char *desEncrypt = new char[data_len+1];
    if(NULL == desEncrypt)
    {
        InfoLog("desEncrypt malloc fail ");
        return -1;
    }
    
    char *base64Encrypt = new char[data_len+1];
    if(NULL == base64Encrypt)
    {
        InfoLog("base64Encrypt malloc fail ");
        delete[] desEncrypt;
        return -1;
    }
    memset(desEncrypt,0x00,data_len+1);
    memset(base64Encrypt,0x00,data_len+1);

    int len = ads_3des_string((char *)"E",(unsigned char*)deskey.c_str(),(char *)plaintext.c_str(),plaintext.length(),desEncrypt); 
    if(0 >= len)
    {
        InfoLog("3des fail len=%d ", len);
        delete[] base64Encrypt;
        delete[] desEncrypt;    
        return -1;
    }   

    ads_urlsafe_base64_enc(desEncrypt,len,base64Encrypt,data_len);
    ciphertext = base64Encrypt;
    
    delete[] base64Encrypt;
    delete[] desEncrypt; 
    return 0;
}

int CIdentPub::JfpNotifyDesUrlBase(CStr2Map& paramap)
{
    CStr2Map contextMap;
    std::string strData;
    std::string  strResponse;

    std::string strSigKey;
    std::string strDESKey;
    
    std::string strNotifyUrl = paramap["notify_url"];
    //std::string strNotifyUrl = "http://192.168.1.112:8088/gydemo/pay/resultNotify";

    if(CIdentPub::GetSignatureKey(paramap["syscode"],strSigKey))
    {
        ErrorLog("获取签名失败 syscode[%s]", paramap["syscode"].c_str());
        return -1;
    }

    if(CIdentPub::GetDESKey(paramap["syscode"],strDESKey))
    {
        ErrorLog("获取des秘钥失败 syscode[%s]", paramap["syscode"].c_str());
        return -1;
    }   

    InfoLog("account:[%s] trans_id:[%s] amount:[%s] notify_url:[%s]", 
        (char *)paramap["account"].c_str(),(char *)paramap["trans_id"].c_str(),
        (char *)paramap["amount"].c_str(),(char *)paramap["notify_url"].c_str()); 
    
     contextMap["trans_id"]=paramap["trans_id"];
     contextMap["result"]=paramap["result"];
     contextMap["amount"]=paramap["amount"];
     contextMap["app_id"]=paramap["app_id"];    
     // 返回数据签名
     strData = "{" + contextMap["trans_id"]   + "}|{" + 
                     contextMap["result"]     + "}|{" + 
                     contextMap["amount"]     + "}|{" +
                     strSigKey + 
               "}";

     //MD5加密
     std::string strRspSig = Tools::MD5(strData).substr(0,16);
     contextMap["signature"] = strRspSig;
     
     string res_context;
     Tools::MapToStrNoEncode(contextMap,res_context);
     InfoLog("返回商户通知内容context-base64前:[%s]", res_context.c_str()); 
     //char out_context[10240]= {"\0"};
     //ads_base64_enc((char *)res_context.c_str(),strlen(res_context.c_str()),out_context,sizeof(out_context));
     char outsrc[32];
     memset(outsrc,0x00,sizeof(outsrc));
     //std::string strDesKey_16 = "E45988F49D01FCC6BEBDFC3F5037CE72";
     //ads_hextoasc2( (char *)strDesKey_16.c_str(), 32, outsrc, sizeof(outsrc));
     //std::string strDESKey = outsrc;
     std::string cOutContext;
     CIdentPub::EncryptDesUrlbase64(res_context,strDESKey,cOutContext);

     //返回数据json格式
     //std::string postData = std::string("context=") + std::string(out_context);
     std::string postData = std::string("context=") + cOutContext;

     InfoLog("md5:[%s] post:[%s][%s]", strData.c_str(),strNotifyUrl.c_str(),postData.c_str()); 

     //发送post通知
     if(-1 == CIdentPub::HttpPost(strNotifyUrl,postData,strResponse)){          
        InfoLog("http post fail %s", strNotifyUrl.c_str()); 
        return -1;
     }

    //检查返回内容
    InfoLog("商户接收通知后返回[%s]", strResponse.c_str()); 
    if(0 != strncasecmp(strResponse.c_str(),"SUCCESS",7))
    {
        //InfoLog("http response %s", strResponse.c_str()); 
        return -1; 
    }       
 
    return 0;   
}

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
int CIdentPub::html_unicode2gbk(char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
    unsigned short gbKey;
    unsigned short uniKey;
    size_t i,j;
    char stemp[50];

    if(inlen > 2040)
        return (-1);
    if(inlen > outlen)
        return (-1);

    i=0;
    j=0;
    for(;i<inlen;)
    {
       if(inbuf[i] != '&')
       {
           outbuf[j]=inbuf[i];
           j++;
           i++;
       }
       else
       {
           i++;
           if(inbuf[i] == '#')
           {
               i++;
               if(inbuf[i] == 'x')
               {
                   i++;
                   memcpy(stemp,"0x",2);
                   memcpy(stemp,&inbuf[i],4);
                   stemp[6]='\0';
                   uniKey = strtol(stemp, NULL, 16);
                   i=i+4;
               }
               else
               {
                   memcpy(stemp,&inbuf[i],5);
                   stemp[5]='\0';
                   uniKey = strtol(stemp, NULL, 10);
                   i=i+5;
               }
               gbKey=SearchCodeTable( uniKey );
               gbKey = (gbKey >> 8) | (gbKey << 8);
               memcpy(&outbuf[j],&gbKey,2);
               j=j+2;
               i++; // 有个;号,要过滤掉
           }
           else 
           {
               outbuf[j]= '&';
               j++;
               outbuf[j]=inbuf[i];
               j++;
               i++;
           }
       }
    }
   
    outbuf[j]= 0x00;
    return j;
}

/*------------------------------------------------------------------------
 *Function Name:CheckTransTime 
 *       Desc: 
 *             交易时间误差在300秒内
 *      Input: YYYYMMDDHHMMSS
 *       Auth: JiaYeHui
 *       Date: 2017-11-18                                                 
 *-----------------------------------------------------------------------*/
void CIdentPub::CheckTransTime(string &trans_time,const int num_sec)
{
    char now_time[20];
    int  div_sec;
    memset(now_time,0x00,sizeof(now_time));
    ads_get_date_time((char *)"%y%m%d%h%n%s",now_time);
    div_sec=ads_diff_sec(now_time,(char *)(trans_time.c_str()));
    if(abs(div_sec) > num_sec)
    {
        ErrorLog("交易失败55 发送时间[%s]-接收时间[%s] 间隔[%d] 只能[%d]",trans_time.c_str(),now_time,div_sec,num_sec);
        throw CTrsExp(ERR_TRANS_TIME,"交易失败55");
    }
}


//trans_id    16    Y    返回的交流流水号，唯一
//result      2     Y    1：成功  2：失败
//amount      10    Y    单位：分
//app_id      30    C    商户的流水号或者应用号，须保证流水唯一
//signature   16    Y    MD5({trans_id }|{result}|{amount}|{密钥})，取上MD5前16位，大写。然后对16位MD5做RSA加密。
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
 *             memo        memo
 *     Return: -1失败 0成功
 *       Auth: JiaYeHui
 *       Date: 2017-08- 09                                                  
 *-----------------------------------------------------------------------*/
int CIdentPub::JfpNotifyMd5Urlbase(CStr2Map& paramap)
{
    CStr2Map contextMap;
    std::string strData;
    std::string  strResponse;

    std::string strSigKey;
    
    std::string strNotifyUrl = paramap["notify_url"];
    //std::string strNotifyUrl = "http://192.168.1.112:8088/gydemo/pay/resultNotify";

    if(CIdentPub::GetSignatureKey(paramap["syscode"],strSigKey))
    {
        ErrorLog("获取签名失败 syscode[%s]", paramap["syscode"].c_str());
        return -1;
    }

    InfoLog("account:[%s]trans_id:[%s] amount:[%s]notify_url:[%s]", 
        (char *)paramap["account"].c_str(),(char *)paramap["trans_id"].c_str(),
        (char *)paramap["amount"].c_str(),(char *)paramap["notify_url"].c_str()); 
    
     contextMap["trans_id"]=paramap["trans_id"];
     contextMap["result"]=paramap["result"];
     contextMap["amount"]=paramap["amount"];
     contextMap["app_id"]=paramap["app_id"];    
     //if(paramap["account"].compare("9202000499") == 0)
     //{
     //   contextMap["memo"]=paramap["memo"];
     //}
         contextMap["memo"]=paramap["memo"];
     // 返回数据签名
     strData = "{" + contextMap["trans_id"]   + "}|{" + 
                     contextMap["result"]     + "}|{" + 
                     contextMap["amount"]     + "}|{" +
                     strSigKey + 
               "}";

     //MD5加密
     contextMap["signature"] = Tools::MD5(strData).substr(0,16);
     
     string res_context;
     Tools::MapToStrNoEncode(contextMap,res_context);
     InfoLog("context urlbase64 pre:[%s]", res_context.c_str()); 
     char out_context[10240]= {"\0"};
     ads_urlsafe_base64_enc((char *)res_context.c_str(),res_context.length(),out_context,sizeof(out_context));
     //返回数据json格式
     //std::string postData = std::string("{\"context\":\"") + std::string(out_context) +std::string("\"}");
     std::string postData = std::string("context=") + std::string(out_context);

     InfoLog("md5:[%s] post:[%s][%s]", strData.c_str(),strNotifyUrl.c_str(),postData.c_str()); 

     //发送post通知
     if(-1 == CIdentPub::HttpPost(strNotifyUrl,postData,strResponse)){          
        InfoLog("http post fail %s", strNotifyUrl.c_str()); 
        return -1;
     }

    //检查返回内容
    InfoLog("http response-[%s]", strResponse.c_str()); 
    if(0!=strncasecmp(strResponse.c_str(),"SUCCESS",7))
    {
        //InfoLog("http response %s", strResponse.c_str()); 
        return -1; 
    }       
 
    return 0;   
}

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
int CIdentPub::CheckAppIdNoDate(std::string app_id)
{
    if(app_id.length()>30)
    {
        ErrorLog("商户流水号非法: app_id=[%s]",app_id.c_str());
        return (-1);
    }

    return 0;
}

int CIdentPub::GetRsaPublicKey(string& syscode, string& rsa_key)
{
    static string rsa_pubkey_array[] =
    {
        //syscode=20000001
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDSOaIHfCXWeLP1OqEiDg4lj7Zd\n"\
        "u39OqB+iwf9DRx6QlNAErEAXrfKARj0wvCjUtKtr1JiAqRT4JFdz24jEotFUC/c/\n"\
        "AIrZ4JpVAoXZj78GgZ3mI6jJYM0XHp/3fsrq5Xx+lJfft518deLPKqtZemtOOlCR\n"\
        "fFBm9gr+vBwz/fNvcwIDAQAB\n"\
        "-----END PUBLIC KEY-----\n",
        
        //syscode=20000002
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCjut+tYS5uQ5IdPQrIeILEVumu\n"\
        "c+RkXvcDc2fMQaBG60IAV76JOAkEOWKA/EnhuOGPntvI7l2X/R58b0MqM2Ey92Xo\n"\
        "jBzlQnZPYFWBTjyC4BQRH69v2DiFZe9pL1DGXdhlW+QmBfvXPigU7Gp0/89p4NX0\n"\
        "KNiTyUnn0CZQ2llQxQIDAQAB\n"\
        "-----END PUBLIC KEY-----\n",
        
        //syscode=20000003
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDMexfkFE9klU//iHBGxA2fENwl\n"\
        "EX2hGk8at9p2vV83lTcLRR9+qdjTsu3j7pt3BNANecCuCI1SW5dYlKI926i/OF40\n"\
        "PbYB/dFhhDLD0a55+VT2baDvsF/n3ROm6WSZY4easImnWh+U7/+pD7DPeB1O0bm4\n"\
        "EAAQ09oa2bArcUP+qwIDAQAB\n"\
        "-----END PUBLIC KEY-----\n",
        
        //syscode=20000004
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCrbukDVa8qBC+zCeZtZZIUb49q\n"\
        "andFHB3VXkhsUyiYIPaKwowTg8RLe03nJyfDnIVS+GA4W/RTMO0AeN/Bya4nw/TA\n"\
        "7n20pj4obgXjVtO3QxHRVEKECoZewp1GtoAjYPSHiSHL+8FEqpw0JMyWrl1eoypz\n"\
        "ROQCocXszoowXgxlSQIDAQAB\n"\
        "-----END PUBLIC KEY-----\n",
        
        //syscode=20000005
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC9o0mM5dhwAhFrVbOufNAyMoMB\n"\
        "FxGo7O0lTox9BdBHDFdS0CJ+wspOuBc0B4pJO4+uo2v1UZ8Eg12RZHdqbJqMCrwt\n"\
        "vUtfWUnC0cyN6ButiRgo4eJSBKXF6TkKRHZewoRerULIVql4D572jBKKqI7aLHoo\n"\
        "8n/WWyqajVYOv+XZ/QIDAQAB\n"\
        "-----END PUBLIC KEY-----\n",
        
        //syscode=20000006
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDQYO/UlOi4aBtXc0RJsOk/1ntd\n"\
        "h2SSxRSTpvsIiNhVtyJ6MTaC2bKQqA/9FvvTGL0ZfcgV4CC4n+vzJIRfSBJP2c6z\n"\
        "WRSMcRDP3wN4PaFNk5yaOhCrk+nHaIBqxJRtIMC9hU0xqM3wOhC5D196gpB64EBv\n"\
        "4FAtN5VQMF3D0fVeFwIDAQAB\n"\
        "-----END PUBLIC KEY-----\n",
        
        //syscode=20000007
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDcio8JGq8ngQBsUOtqkKAPHIVC\n"\
        "O8aXDYs2ZF/oaRL+v9W23BjO3M6fOOnZaigYpNUk/Jv6eY+2uj8VB/WM8NeWxQn3\n"\
        "TTgP/YKsNCClGUMMTSvK2RsOXakiiVldIsAiut2CSbwDEsMhunqJo1rb4CKGEmGB\n"\
        "/C6HM/6uKw/4Seko8wIDAQAB\n"\
        "-----END PUBLIC KEY-----\n",
        
        //syscode=20000008
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDhQEDuBEsFGjjLGAdibaq7Umnh\n"\
        "2+MKp9EhSA9KDLNG060AKTWhycUp5imKzkMIOnixXFbVwKEKplfUtxYFYQHidhhA\n"\
        "43zEVhy3BAyKtMjd/3a/qz1EXbCUtSX4yshysYaX97gmKZf+k2s1e96E2Ufh6HVv\n"\
        "71KXFMld8iFg95qwewIDAQAB\n"\
        "-----END PUBLIC KEY-----\n",
        
        //syscode=20000009
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC4TBEgSkR268MUXVgvFt0MHt6F\n"\
        "lywMBmDElx9UMtLnJQipj7f+xulN4uwBxKkWDeU3To2udUCazgwEuE42bCjGUo7f\n"\
        "7K/3FmcUTkwcluuZ2N5XdEkwsS3jii9QF2qb4Cs2cNyoMSHQ1ash3TNAz+L164c4\n"\
        "WhMd8dWcg8AeKUxrHQIDAQAB\n"\
        "-----END PUBLIC KEY-----\n",
        
        //syscode=20000010
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC4j8mQhtvs5+5dPIRlEwIkVGJr\n"\
        "/ij/rkY4+qtphwRjS3nxoRuSez8e5860jWa5FGa2OTAG39623R14cOTLFn//Wdsk\n"\
        "d+0rP0QiAerov3XSApo0+qEhmwDfGNCOYQikV/kL3jXAx3WaPdRW2NGpFov+al4K\n"\
        "ikrf9FDbBVQjiFBeDwIDAQAB\n"\
        "-----END PUBLIC KEY-----\n",
        
        //syscode=20000011
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDKRK3V+Mc27cey3qV4XSul36Du\n"\
        "f2Hog9bAMqLRaWQdwCOz5ulH5q+1WxfkbxPZkmjghuhaM9v/4Kg5FamJ2B74Kp4x\n"\
        "nHd9LAfhySRgFPyTimA4li3pO5VcL+E7ZWghcoTReRVGzGUOcF7MtCULCJHLhYK6\n"\
        "vAEfjm9UM02pdCcoPQIDAQAB\n"\
        "-----END PUBLIC KEY-----\n",
        
        //syscode=20000012
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDWIvA4hcQJoWJGraAu7kQLpEZq\n"\
        "aXccZA6Dkdd8YpFo5Z1D8UBwrGjFgDsxjDVkI2Z6nl+c2zwjJO+nj+WwNSXsAQ2C\n"\
        "woG28yXSUogk5aNoUSmI6kciZk2eFY9BQvGTZNAZNa3rU4e+ujXlpUVaSs4+iAtO\n"\
        "B9FvB093GOW8saI09QIDAQAB\n"\
        "-----END PUBLIC KEY-----\n",
        
        //syscode=20000013
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDSud3TNGfjLWjbm6ryWqQ5Vw8w\n"\
        "4OSL4yanH37OOg347mdnjHFtdcZ+Sp/NLcGKUelFVjj3M+rR1ZCaw422w2zW5KkK\n"\
        "4J3hgRvKk805csJ/DrY4zJ6xWH6vaKMClPPp5afZukXE9a3kZ6ZioLfTcMu+bZKr\n"\
        "JjNHRtZU76qnNj0ubwIDAQAB\n"\
        "-----END PUBLIC KEY-----\n",
        
        //syscode=20000014
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDFPJY8LnV9ys0KNscs2b5V7pZz\n"\
        "5VldTuvbgecFQ1JwfMnclKTATP2IJgOXokGrCpNPpQ2FkpU4WJM4QzBs3KfJWg+2\n"\
        "5o/06jvtIJF3qbiR1GVtgvL8bsfja6R9tVpISTF23DKCr85b+BCEPnpNlFLRF+2S\n"\
        "4LtWT9z4ZMUviRXL6wIDAQAB\n"\
        "-----END PUBLIC KEY-----\n",
        
        //syscode=20000015
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQD9M5oZHuL2SrljBiQGIvvNPv8b\n"\
        "CeNs6kwXgrTbZM6yIS78rpb01Hrbn0ZfQAAyLB64hATOn1Y68b1eyfwaRAu+JfCt\n"\
        "zCs+yQ8bWPpq8+65jceqCC01+obGl3wFZTmaVBaApI1RCMMvEDpGLJNmwwxACK92\n"\
        "VY3r5vbO3ABYgobYswIDAQAB\n"\
        "-----END PUBLIC KEY-----\n",
        
        //syscode=20000016
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCcer0FnSHpWzKG0/5qb2D7gFEQ\n"\
        "9AhSSHXWbr1CAo4D64bOo41q5tgKSFp7QgxeKUBcyhaPADOlHGgUHYx+fEAwpibJ\n"\
        "RcHjNKQuRk6FUNyqtJ2EETpvERVq4woZJMgoHUSbYQJFxHO7QDxhSpCdJM5QsUk5\n"\
        "qrQAv2NTuUEM/ngNlQIDAQAB\n"\
        "-----END PUBLIC KEY-----\n",
        
        //syscode=20000017
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC4H7bBERx+mzqkp+LBKyMBmDV0\n"\
        "W8MciDiT1WKF/PR7cv56EHXXzIbbR23NBDJbpipNV4zlKcuRi92eRZohnm8MQEYo\n"\
        "xhkNlPNpQqh2e4vqZUOZZUkduwaVQMeDsGSU5s3tnnmIckWLmocaqhfNdarce9oF\n"\
        "d3Qt5fqTQzAy7tM/yQIDAQAB\n"\
        "-----END PUBLIC KEY-----\n",
        
        //syscode=20000018
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDTk9W1/+r+BHlQQIgqHaER2MHD\n"\
        "HYjWk5cW4RUB5wGotOM3xjNwKOnnXSc767UMIQse4a6/S0V7ewQnrH1DqYRAuiat\n"\
        "wBpujzs7SJsZLqKYnWmoB5H+u7vQ6LHpxI7sDufQ7qHyx7HKKodKXxxwek9qRwkI\n"\
        "FXkNsZ2I7DitVl+bswIDAQAB\n"\
        "-----END PUBLIC KEY-----\n",
        
        //syscode=20000019
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDrvujEzywI9EIHwM4EHPM1+aZc\n"\
        "/gSHnPURzCQwsVXgOGyQzCRnkht1G7TlseTNF2J2Wh7t1axjghhE16LIeIEkKIPD\n"\
        "EEbZIBfUg2MEXII2GKWQwXzfuo2vWalBI3oQ7NNrwkYpQ6eaCRrNVAdJM2dGZbFJ\n"\
        "LbScdom+D68SybSKNQIDAQAB\n"\
        "-----END PUBLIC KEY-----\n",
        
        //syscode=20000020
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDCLxOGRrQqyEL55yPJow1APvaR\n"\
        "76UCwp4oaDltDCU2hx85rLxyuvu+kgO2j2VfzHOsa8OsZYS9dJ47ztUSUBf4aRdz\n"\
        "A7bz1m6H55M6lhGjhyJl/rG/AUJ3xY1Ijqtrm31L4Ue7of87v16RH7saLXDE9b3P\n"\
        "6uNKkKW5RnMaffG47QIDAQAB\n"\
        "-----END PUBLIC KEY-----\n",
        
        //syscode=20000021
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDXjSXa6sjAxL760BIuHETK7bdO\n"\
        "jJNm+b2dY+SUgA0TQhMdxKKD+tuGg5oD3MoImPP7a5m7tSc4UwngyEWRSOICCJ9s\n"\
        "FJdlSPLpcnxl9OLoEyGl6plGuyBPzE+WSjQwAGb1Xz4srJz2ym9BEioJJHMKu3ZX\n"\
        "JyYIwmNj1U8U2Vr9ZwIDAQAB\n"\
        "-----END PUBLIC KEY-----\n",
        
        //syscode=20000022
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDExU8PdP+0Zy0OM+sJAvOvKIFF\n"\
        "gnH79zNgr3DYWdM0aqdQEeRCjcJKgYh5kFMQ2t+jWNDmnbkhLIlFpgfVMB0Oy67d\n"\
        "pqQfOXB3/TtYlCDvnzLm+szzyHbhK/mOdAMExv0DD1C7PDKCefnCL6ebfSr8gUv9\n"\
        "qknOVEt1mlJ5tlsipQIDAQAB\n"\
        "-----END PUBLIC KEY-----\n",
        
        //syscode=20000023
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDTZ67OVssCDxH6zYgM5kPNbJdf\n"\
        "ya8akbBAzQzsBPcwv/TThiDjGPk6vzJIrnQ253CaIUDz00LB0fOaZ33wb/PEWitp\n"\
        "AQdLr9XAuhIeDRgUG5v1SOYPIPPSfjVv/ctdOv+piwVklHQGvcWE/sjTPWGJJq3o\n"\
        "IJnw2SwZAY/a87XB2wIDAQAB\n"\
        "-----END PUBLIC KEY-----\n",
        
        //syscode=20000024
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDGowrYuzF1nqPw/RC1ahVM3vCJ\n"\
        "M4zR8hw9OE0DwmC38AzCTa45ZKmWKwnaFAFB7IdcpVJUyF1l1VQMa6Nl5qfYvuxR\n"\
        "TZmmHy2bgBJf6hBARwfYccK+s/xCnfjAAW8RyMFdGsSm8ZwVWltAN9UpjTzwHYLw\n"\
        "zYIEDKrwDMtvwyZFmwIDAQAB\n"\
        "-----END PUBLIC KEY-----\n",
        
        //syscode=20000025
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDYOhmtUyO6TOOwbK5TyLF8pS/6\n"\
        "yCe3kaFlfvhk31PPhWz3P7VTqlX3UKxdpA9UKZ7VVWj/vcUiUfT5tZk9a2Xsxnaj\n"\
        "/xCHX0W+62yW9rYyL7HkJb3ehqA2mLPCXaNxs+AEQVnHxf1YHUAF/y/h4Kn/JezE\n"\
        "zspjGJOf76HlFABXQQIDAQAB\n"\
        "-----END PUBLIC KEY-----\n",
        
        //syscode=20000026
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDA44lfJbl5Ubp8abpjoBQOf54s\n"\
        "jJVXkPmzbLThiD0/11ZGW8jk1kOAj7OORyaD6vNDxKXU+bVx+ii0QSdtyuelNK5S\n"\
        "2pcAV1gQ8kDuw+Vj70DHaNU0soBgoHvxWA4kXe1TImJLYDNw5aZ6ipVR1Tk8Lg8o\n"\
        "S8ze7PQsYi8qTlw1vwIDAQAB\n"\
        "-----END PUBLIC KEY-----\n",
        
        //syscode=20000027
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC8txoTzGFip/E+8at2rGaxKmfz\n"\
        "rLx9LCGmhEXWWckI6N6CRepY5TpCBMVACNrAWqWTNebTUhR02O0A9NDCchw4Od2B\n"\
        "p8Tihg0Lz2V5ICjTfwWJIOJe4i0cHzFbJUKd6pJLafjzxPt0gBE28Wrjwmibeu3U\n"\
        "Fs0avcfF3YWDIDpy5wIDAQAB\n"\
        "-----END PUBLIC KEY-----\n",
        
        //syscode=20000028
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDOpDhO4cuiC4PJPWwBCsD82rlp\n"\
        "h39WZuW2Dqh1GMtRu1yaIIeKuge80yNC44jFzIYen5Cw1VnBVluXot8TU7nkVOow\n"\
        "00eEBaaDK0v/x88KWorMXsZP6KRhF7lVqAx2ELA7Xmn6i1KynrSn5fjN/9zHw09q\n"\
        "z8dSwKjrk9PP3BZbbwIDAQAB\n"\
        "-----END PUBLIC KEY-----\n",
        
        //syscode=20000029
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCvyZDzAmjlWBE8n5nsVd3NO3xD\n"\
        "WAzkP5jV1b7Ia9p3YFjlKgCDMoFy6zqf931CQphtuber8lfMX2Zlknzn6JQF4P2t\n"\
        "2tlmguDN7QsJfVtzqfonXPllHGzd8n3mYsD1LqcRT+6ilwS+p3bcNYH6i1R+jQko\n"\
        "JDwy8V0M0LrOx+6h0wIDAQAB\n"\
        "-----END PUBLIC KEY-----\n"\
        
        //syscode=20000030
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDMfkCkTsfpgkbfvQGSWlstjB8b\n"\
        "xqbdjoUm8RMiEu3SDM7N5RQg60dXQIprTth0k/VTlD+QFIqaXH+641nRdnHKeptB\n"\
        "qID80tSf6l+QOw6mNGPJiSyF3/0ogr+6dBAlAokH+TpxiwLnuHchSts5iqwfYwd5\n"\
        "EEx7PCwm/nY6exd7BQIDAQAB\n"\
        "-----END PUBLIC KEY-----\n"
    };

    /*syscode长度要求是8，内容是数字*/
    if ((syscode.length() != 8))
    {
         return -1;
    }
    if(!ads_isnum((char *)syscode.c_str()))
    {
        return -1;
    }
    int sys = atoi((char*)syscode.c_str());
    int sub = sys - 20000000;
    if(sub<1 || sub>30)
    {
         return -1;
    }
   
    rsa_key = rsa_pubkey_array[sub-1]; 
    
    //InfoLog("KKKKKKKKKK:%s %s", syscode.c_str(),rsa_key.c_str());  
    return 0;
}

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
int CIdentPub::parsePubRespJson(string &data, CStr2Map& outMap)
{
    Document vRoot;
    vRoot.Parse<rapidjson::kParseDefaultFlags>((char*)data.c_str());
    if (vRoot.HasParseError())
    {
        ErrorLog("json parse fail %s",data.c_str());
        return -1;
    }

    for(Value::ConstMemberIterator itr = vRoot.MemberBegin(); itr != vRoot.MemberEnd(); itr++)
    {
        Value jKey,jValue;
        Document::AllocatorType allocator;
        jKey.CopyFrom(itr->name, allocator);
        jValue.CopyFrom(itr->value,allocator);
        if(jKey.IsString())
        {
            outMap[jKey.GetString()]=jValue.GetString();
        }
    }

    return 0;
}

int CIdentPub::parsePubReqJsonList1(std::string &data, CStr2Map& outMap, std::vector<CStr2Map>& vectmapArray)
{
    Document document;
    document.Parse(data.c_str());

    if (document.HasParseError()) {
        return -1; // 返回错误代码
    }

    if (document.IsObject()) {
        for (auto& m : document.GetObject()) {
            const char* key = m.name.GetString();
            const Value& value = m.value;

            // 处理数组
            if (value.IsArray()) {
                for (SizeType i = 0; i < value.Size(); i++) {
                    const Value& tmpValue = value[i]; // 使用 const 引用
                    CStr2Map arrayMap; // 为每个数组元素创建新的映射

                    // 处理数组元素
                    if (tmpValue.IsObject()) {
                        for (auto& obj : tmpValue.GetObject()) {
                            arrayMap[obj.name.GetString()] = obj.value.GetString(); // 将对象的键值对存入 arrayMap
                        }
                    }

                    vectmapArray.push_back(arrayMap); // 将每个数组元素的映射添加到 vectmapArray
                }
            } else {
                // 处理其他类型
                if (value.IsString()) {
                    outMap[key] = value.GetString();
                } else if (value.IsNumber()) {
                    outMap[key] = std::to_string(value.GetInt());
                } else if (value.IsBool()) {
                    outMap[key] = value.GetBool() ? "true" : "false";
                }
            }
        }
    }

    return 0; 
}

int CIdentPub::parsePubReqJsonList(string &data, CStr2Map& outMap,vector<CStr2Map>& vectmapArray)
{
    Document document;
    document.Parse(data.c_str());

    if (document.HasParseError()) {
        return -1; // 返回错误代码
    }

    if (document.IsObject()) {
        for (auto& m : document.GetObject()) {
            const char* key = m.name.GetString();
            const Value& value = m.value;

            if (value.IsArray()) {
                for (SizeType i = 0; i < value.Size(); i++) {
                    const Value& tmpValue = value[i]; // 使用 const 引用
                    CStr2Map arrayMap; // 为每个数组元素创建新的映射

                    string arrayKey = string(key) + "[" + to_string(i) + "]"; // 构造键

                    if (tmpValue.IsString()) {
                        arrayMap[arrayKey] = tmpValue.GetString();
                    } else if (tmpValue.IsNumber()) {
                        arrayMap[arrayKey] = to_string(tmpValue.GetInt());
                    } else if (tmpValue.IsBool()) {
                        arrayMap[arrayKey] = tmpValue.GetBool() ? "true" : "false";
                    } else if (tmpValue.IsObject()) {
                        StringBuffer sbBuffer;
                        Writer<StringBuffer> jWriter(sbBuffer);
                        tmpValue.Accept(jWriter);
                        string nestedJson = sbBuffer.GetString();
                        CStr2Map inMap;
                        parsePubReqJsonList(nestedJson, inMap, vectmapArray); // 递归解析嵌套对象
                        vectmapArray.push_back(inMap);
                    }

                    vectmapArray.push_back(arrayMap); // 将每个数组元素的映射添加到 vectmapArray
                }
            } else {
                // 处理其他类型
                if (value.IsString()) {
                    outMap[key] = value.GetString();
                } else if (value.IsNumber()) {
                    outMap[key] = to_string(value.GetInt());
                } else if (value.IsBool()) {
                    outMap[key] = value.GetBool() ? "true" : "false";
                }
            }
        }
    }


    return 0; 
}

/*------------------------------------------------------------------------
 *Function Name: parsePubRespJsonList
 *       Desc: 
 *        将":" json串转成map 多层输出
 *      Input: 
 *        待转码串data
 *        转码结果Map
 *     Return: -1失败 0成功
 *       Auth: JiaYeHui
 *       Date: 2018-01-28                                                   
 *-----------------------------------------------------------------------*/
int CIdentPub::parsePubRespJsonList(string &data, CStr2Map& outMap,vector<CStr2Map>& vectmapArray)
{
    Document vRoot;
    vRoot.Parse<rapidjson::kParseDefaultFlags>((char*)data.c_str());
    if (vRoot.HasParseError())
    {
        ErrorLog("json parse fail %s",data.c_str());
        return -1;
    }

    for(Value::ConstMemberIterator itr = vRoot.MemberBegin(); itr != vRoot.MemberEnd(); itr++)
    {
        Value jKey,jValue;
        Document::AllocatorType allocator;
        jKey.CopyFrom(itr->name, allocator);
        jValue.CopyFrom(itr->value,allocator);
        if(jValue.IsString())
        {
            outMap[jKey.GetString()]=jValue.GetString();
        }
		else if(jValue.IsNumber())
		{
            outMap[jKey.GetString()]=Tools::IntToStr(jValue.GetInt());
		}
		else if(jValue.IsBool())
		{
       	 	outMap[jKey.GetString()]=jValue.GetBool() ? "true" : "false";
		}
		else if(jValue.IsObject())
		{
			StringBuffer sbBuffer;
			Writer<StringBuffer> jWriter(sbBuffer);
			jValue.Accept(jWriter);  
			string jsonData = sbBuffer.GetString();
			parsePubRespJsonList(jsonData,outMap,vectmapArray);
		}
		else if(jValue.IsArray())
		{
			Value tmpValue;
			CStr2Map inMap;
			for(SizeType i = 0; i < jValue.Size(); i++) 
			{
				tmpValue  = jValue[i];
				if(tmpValue.IsObject())
				{
					StringBuffer sbBuffer;
					Writer<StringBuffer> jWriter(sbBuffer);
					tmpValue.Accept(jWriter);  
					string jsonData = sbBuffer.GetString();
					parsePubRespJsonList(jsonData,inMap,vectmapArray);

					if(inMap.size() > 0)
						vectmapArray.push_back(inMap);
				}
            }
		}
    }

    return 0;
}

/********************************************************************************
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
 * ********************************************************************************/
int CIdentPub::CheckDebitCard(string card_no)
{
    char card[36];
    int  i_len;
    int  sum=0;
    int  i;
    int  temp;
    bool even=0;

    i_len=card_no.length();
    if(i_len > 30 || i_len < 10)
    {
        ErrorLog("借记卡号长度不对: card_no=[%s]",card_no.c_str());
        return -1;
    }
    if(!ads_isnum((char *)card_no.c_str()))
    {
        ErrorLog("借记卡号非全数字: bank_no=[%s]",card_no.c_str());
        return (-1);
    }

    memset(card,0x00,sizeof(card));
    strcpy(card,card_no.c_str());

   /* 
   从最右一位数开始向左，把每个数字交替乘1或2， 
   如果结果大于 9就减9。如果把各位数的计算结果加起来， 
   最后得到的总和能被10 整除，那这个卡号就是有效卡号。 
   */
   for(i=i_len-1;i>=0;i--)
   {
       if(even)
           temp=(card[i]-'0')*2;
       else
           temp=(card[i]-'0');

       even=!even;

       if(temp>9)
          temp-=9;
       sum+=temp;
   }

   if(sum%10 == 0)
      return 0;
   else
   {
      ErrorLog("借记卡号规则不对: card_no=[%s]",card_no.c_str());
      return -1;
   }
}

//配置秘钥，给通讯用的，只有1-8个秘钥 
//第1个给收银台用
//第2个给CGI通讯使用
string CIdentPub::GetConDESKey(int sub)
{
    static string con_deskey_array[]={
        "8AFF66B621F86A1B55CF4DB179099AB9",
        "ECB346DDD7686A1B5A81B15E1272911D",
        "C445699282886A1B5C4C4BC6723C753D",
        "879D630C7E286A1B5C1F1E6302A1BA8D",
        "263E58CB6D786A1B55A3BBBF02DD6038",
        "5C0FE70F2F886A1B5F74950C6A35BB66",
        "882F4E5047486A1B5C2ED19982EC898D",
        "9296AE5B63186A1B55D9C2AA59F22D61"
     };

     if(sub<1 || sub>8)
     {
          return "";
     }

     char outsrc[32];
     memset(outsrc,0x00,sizeof(outsrc));
     ads_hextoasc2( (char *)con_deskey_array[sub-1].c_str(), 32, outsrc, sizeof(outsrc));

     return string(outsrc);
}

//配置秘钥，给通讯用的，只有1-8个秘钥
//第1个给收银台用
//第2个给CGI通讯使用
string CIdentPub::GetConMD5Key(int sub)
{
    static string con_md5key_array[]={
          "ak3dszpay50tq98",
          "ak3jszpay500riu",
          "alfaszpay5tyt0",
          "ljalszpay5g0a9",
          "8775szpay5kjal",
          "lfajszpay50270",
          "ljglszpay51012",
          "ak3jszpay58u0ru"
     };

     if(sub<1 || sub>8)
          return "";

     //md5_key=con_md5key_array[sub-1];
     //return 0;
     return con_md5key_array[sub-1];
}

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
string CIdentPub::WebrsaToAdsdes(int n_web,string &web_pwd,int n_ads,const int num_sec)
{
    char ascbuf[1024];
    char rsabuf[256];
    char des_open[512];
    char des_enc[512];

    //将web_pwd的内容base64解析
    memset(ascbuf,0x00,sizeof(ascbuf));
    int len=web_pwd.length();
    len=ads_urlsafe_base64_dec((char *)web_pwd.c_str(),len,ascbuf, sizeof(ascbuf));

    string rsa_pri_key;
    memset(rsabuf,0x00,sizeof(rsabuf));
    memset(des_enc,0x00,sizeof(des_enc));

    string rsa_key;
    CRsaHandler::GetCommRsaPrivateKey(n_web, rsa_key);
    int dec_len=CRsaHandler::private_decrypt((unsigned char *)ascbuf,len,
            (unsigned char *)rsa_key.c_str(),(unsigned char *)rsabuf);
    if(dec_len == -1)
    {
        ErrorLog("系统维护中-解密失败");
        throw CTrsExp(ERR_WRITE_SESSION,"系统维护中");
        //return(-1);
    }

    //解密内容14位时间+密码
    if(dec_len < 20)
    {
        ErrorLog("系统维护中-RSA解密后长度<20,buf=[%s]",rsabuf);
        throw CTrsExp(ERR_WRITE_SESSION,"系统维护中");
        //return(-1);
    }
    if(dec_len > 60)
    {
        ErrorLog("系统维护中-RSA解密后长度>20,buf=[%s]",rsabuf);
        //return(-1);
        throw CTrsExp(ERR_WRITE_SESSION,"系统维护中");
    }
    if(n_ads == 1) //交易密码是6位数字
    {
        if(dec_len != 20)
        {
            ErrorLog("系统维护中-RSA解密后长度不等于20,buf=[%s]",rsabuf);
            throw CTrsExp(ERR_WRITE_SESSION,"系统维护中");
            //return(-1);
        }
    }

    //时间验证 20秒有效期
    memset(des_open,0x00,sizeof(des_open));
    memcpy(des_open,rsabuf,14);
    InfoLog("kkkkkkk-rsabuf=[%s] des_open=[%s]",rsabuf,des_open);
    string tm=des_open;
    CIdentPub::CheckTransTime(tm,num_sec);
    memset(des_open,0x00,sizeof(des_open));
    strcpy(des_open,rsabuf+14);
    InfoLog("kkkkkkk-rsabuf=[%s] des_open=[%s]",rsabuf,des_open);

    char Mode[]="E";
    CIdentPub::ads_des_idstr_see(Mode,n_ads,des_open,strlen(des_open),des_enc);

    return string(des_enc);
}

/*------------------------------------------------------------------------
*Function Name:WebrsaDec
*       Desc: 将前端的RSA加密信息解密为明文
*      Input: 
*            web_pwd:前端密码 urlbase64
*              n_web:前端第几个秘钥,目前只有2个
*              n_ads:后台第几个秘钥,目前只有40个[0-39] 0登录密码 1交易密码
*            num_sec:rsa精确到几秒，默认20秒
*     Return:ADS后台DES密码
*       Auth: JiaYeHui
*       Date: 2018-03-30                                                
*-----------------------------------------------------------------------*/
string CIdentPub::WebrsaDec(int n_web,string &web_pwd,int n_ads,const int num_sec)
{
	char ascbuf[1024];
	char rsabuf[256];
	char des_open[512];
	char des_enc[512];

	//将web_pwd的内容base64解析
	memset(ascbuf,0x00,sizeof(ascbuf));
	int len=web_pwd.length();
	len=ads_urlsafe_base64_dec((char *)web_pwd.c_str(),len,ascbuf, sizeof(ascbuf));

	string rsa_pri_key;
	memset(rsabuf,0x00,sizeof(rsabuf));
	memset(des_enc,0x00,sizeof(des_enc));

	string rsa_key;
	CRsaHandler::GetCommRsaPrivateKey(n_web, rsa_key);
	int dec_len=CRsaHandler::private_decrypt((unsigned char *)ascbuf,len,
		(unsigned char *)rsa_key.c_str(),(unsigned char *)rsabuf);
	if(dec_len == -1)
	{
		ErrorLog("系统维护中-解密失败");
		throw CTrsExp(ERR_WRITE_SESSION,"系统维护中");
		//return(-1);
	}

	//解密内容14位时间+密码
	if(dec_len < 20)
	{
		ErrorLog("系统维护中-RSA解密后长度<20,buf=[%s]",rsabuf);
		throw CTrsExp(ERR_WRITE_SESSION,"系统维护中");
		//return(-1);
	}
	if(dec_len > 100)
	{
		ErrorLog("系统维护中-RSA解密后长度>100,buf=[%s]",rsabuf);
		//return(-1);
		throw CTrsExp(ERR_WRITE_SESSION,"系统维护中");
	}
	if(n_ads == 1) //交易密码是6位数字
	{
		if(dec_len != 20)
		{
			ErrorLog("系统维护中-RSA解密后长度不等于20,buf=[%s]",rsabuf);
			throw CTrsExp(ERR_WRITE_SESSION,"系统维护中");
			//return(-1);
		}
	}

	//时间验证 20秒有效期
	memset(des_open,0x00,sizeof(des_open));
	memcpy(des_open,rsabuf,14);
	InfoLog("kkkkkkk-rsabuf=[%s] des_open=[%s]",rsabuf,des_open);
	string tm=des_open;
	CIdentPub::CheckTransTime(tm,num_sec);
	memset(des_open,0x00,sizeof(des_open));
	strcpy(des_open,rsabuf+14);
	InfoLog("kkkkkkk-rsabuf=[%s] des_open=[%s]",rsabuf,des_open);

	
	return string(des_open);
}

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
string CIdentPub::RsaNumEnc(int n_key,string &strData)
{
    char rsabuf[256];
    char urlbuf[512];
    int  len;
    int  enc_len;

    //将strData的内容base64解析
    len=strData.length();
    if(len > 118)
    {
        ErrorLog("系统维护中-加密失败-数据长度大于118-[%d]",len);
        throw CTrsExp(ERR_WRITE_SESSION,"系统维护中");
    }

    memset(rsabuf,0x00,sizeof(rsabuf));

    string rsa_key;
    CRsaHandler::GetCommRsaPublicKey(n_key, rsa_key);

    enc_len=CRsaHandler::public_encrypt((unsigned char *)strData.c_str(),len,
            (unsigned char *)rsa_key.c_str(),(unsigned char *)rsabuf);
    if(enc_len == -1)
    {
        ErrorLog("系统维护中-加密失败");
        throw CTrsExp(ERR_WRITE_SESSION,"系统维护中");
    }

    //将加密结果转为urlbase64
    memset(urlbuf,0x00,sizeof(urlbuf));
    ads_urlsafe_base64_enc((char *)rsabuf,enc_len,urlbuf,sizeof(urlbuf));

    return string(urlbuf);
}

/*------------------------------------------------------------------------
*Function Name:RsaEnc
*       Desc: 将cgi的字符串数据rsa加密
*      Input: 
*              n_key:第几个秘钥,目前只有2个
*           cgi_data:明文字符串
*     Return: urlbase64的rsa密文
*       Auth: leize
*       Date: 2018-10-17                                               
*-----------------------------------------------------------------------*/
string CIdentPub::RsaEnc(string &strData)
{
	char rsabuf[4096];
	char rsabuftmp[117+1];
	char urlbuf[2048];
	int  len;
	int  enc_len;
	int  offSet = 0;
	int  i = 0;
	int  enc_sumlen = 0;

	//将strData的内容base64解析
	len=strData.length();
	if(len > 2048)
	{
		ErrorLog("系统维护中-加密失败-数据长度大于2048-[%d]",len);
		throw CTrsExp(ERR_WRITE_SESSION,"系统维护中");
	}

	memset(rsabuftmp,0x00,sizeof(rsabuftmp));
	memset(rsabuf,0x00,sizeof(rsabuf));

	string rsa_key;
	string privateKey = "-----BEGIN PRIVATE KEY-----\n";
	privateKey +="MIICdQIBADANBgkqhkiG9w0BAQEFAASCAl8wggJbAgEAAoGBAInJN8sPGfbN6eS7\n";
	privateKey +="ygS2CEra1ivSrp6bt2GEcx5syAbM3cb7a1S1TCfhiHivkbvHodYcC1PzEUQUr48d\n";
	privateKey +="fGbxuM6+nx3ecpAOnEqyQNGGfAG5EOen0Oey9jjSOYvIUtLGY5H9SLU10qw0aJe5\n";
	privateKey +="8ZYHk61OWhdI5FeP0DPIZfQjQjJvAgMBAAECgYAJ/ImMsCWDm90N9QZpXQAw3LeV\n";
	privateKey +="KYn6ePLLoJvvYpcE1yhj2akn1JQWd+Q6Sw9W+tsh95pilUV1F8K5rrtrgq2QQ0HR\n";
	privateKey +="tdA/NwrkOJTd3aTvmD2Y/JUFF7rjpIfiJvUzXfWv0oJprjz886by5zAI58tFurdk\n";
	privateKey +="vdWi6J/tuGvOZlWegQJBAMXAGBhT3Rwd4IAMoMQJPEiKvIV5ebrkKaQ4rFmxK0r4\n";
	privateKey +="BMXh7Iv2vMUE1uyK+DIKCmEDxKauAvItE0T4A9noGg8CQQCyX1kB1my08LYbuZwX\n";
	privateKey +="IK07Njr4MgtNHg7HoQLZfFdwlZnszGtIYaNNHjLxrOPoEV57PERW2NZFvmEPeTZX\n";
	privateKey +="lEGhAkB2VGwWopg8quQbu3K3247nGZ2VgQsGemEwk3kOcqWlRqQUhQw29H4gprS0\n";
	privateKey +="9rNtvfRX+RlDY/z/TVmqe35SdkChAkBdgcZzRCkwoY+V8TN2nFaz17YKLpHmF3+/\n";
	privateKey +="/xQzVw+voX1TucXz59tnrhEeyHehTJmvGOTqcjnBzg+rwOtP9hSBAkAVs1MYsdAx\n";
	privateKey +="6ogfFCYDJqYx1p45LAknFK4VhZYyiMEOK41Aqx0xtslvo/8CIYdERJPRKa9kYEbx\n";
	privateKey +="yd9UXp0FLcYJ\n";
	privateKey +="-----END PRIVATE KEY-----";

	while(len - offSet > 0)
	{
		if (len - offSet > 117)
		{
			enc_len=CRsaHandler::private_encrypt((unsigned char *)strData.c_str()+i*117,117,
				(unsigned char *)privateKey.c_str(),(unsigned char *)rsabuftmp);
			if(enc_len == -1)
			{
				ErrorLog("系统维护中-加密失败");
				throw CTrsExp(ERR_WRITE_SESSION,"系统维护中");
			}

		}
		else
		{
			enc_len=CRsaHandler::private_encrypt((unsigned char *)strData.c_str()+i*117,len - offSet,
				(unsigned char *)privateKey.c_str(),(unsigned char *)rsabuftmp);
			if(enc_len == -1)
			{
				ErrorLog("系统维护中-加密失败");
				throw CTrsExp(ERR_WRITE_SESSION,"系统维护中");
			}
			
		}
		memcpy(rsabuf+enc_len*i,rsabuftmp,enc_len);
		enc_sumlen += enc_len;
		i++;
		offSet = i * 117;
	}
	memset(urlbuf,0x00,enc_sumlen);
	ads_base64_enc((char *)rsabuf,enc_sumlen,urlbuf,sizeof(urlbuf));

	return string(urlbuf);
}

string CIdentPub::RsaDESenc(string &srcStr)
{
	char srcData[4096];
	char rsabuf[4096];
	char rsabuftmp[117+1];
	char base64desEncrypt[4096];
	int  len = 0;
	int srcLen;
	int  enc_len;
	int  offSet = 0;
	int  i = 0;
	int  enc_sumlen = 0;
	char szHexBuf[4096];

	InfoLog("开始解密...");
	memset(base64desEncrypt,0x00,sizeof(base64desEncrypt));
	memset(srcData,0x00,sizeof(srcData));
	memset(rsabuftmp,0x00,sizeof(rsabuftmp));
	memset(rsabuf,0x00,sizeof(rsabuf));
	memset(szHexBuf,0x00,sizeof(szHexBuf));

	len=srcStr.length();
	strcpy(srcData,srcStr.c_str());

	srcLen = ads_base64_dec(srcData,len,base64desEncrypt,sizeof(base64desEncrypt));
	InfoLog("srcLen=[%d]",srcLen);

	//ads_asctohex(base64desEncrypt,srcLen,szHexBuf,sizeof(szHexBuf));
	string strDESdate = base64desEncrypt;

	InfoLog("strDESdate=[%s]",strDESdate.c_str());
	//InfoLog("strDESdate=[%s]",strDESdate.c_str());
	string publicKey = "-----BEGIN PUBLIC KEY-----\n";
	publicKey +="MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCbowBDWWZw71okCHdn4/mvNvDI\n";
	publicKey +="NFd7CVZM4pQGajdu0IcALcGmp0OSbslEY5gJNInlOFGY+2mYV1796Ebp3KlSm/c2\n";
	publicKey +="57hzGibs6ApyfFxs/FB8T0zv4syBTf9e8fAVqyoBplc9ghAw3bFusalnsNFyh1eu\n";
	publicKey +="opRUWtHNN0rm7rjpcwIDAQAB\n";
	publicKey +="-----END PUBLIC KEY-----";

	while(srcLen - offSet > 0)
	{
		if (srcLen - offSet > 128)
		{
			enc_len=CRsaHandler::public_decrypt((unsigned char *)strDESdate.c_str()+i*128,128,
				(unsigned char *)publicKey.c_str(),(unsigned char *)rsabuftmp);
			if(enc_len == -1)
			{
				ErrorLog("系统维护中-解密失败");
				throw CTrsExp(ERR_WRITE_SESSION,"系统维护中");
			}

		}
		else
		{
			enc_len=CRsaHandler::private_encrypt((unsigned char *)strDESdate.c_str()+i*128,srcLen - offSet,
				(unsigned char *)publicKey.c_str(),(unsigned char *)rsabuftmp);
			if(enc_len == -1)
			{
				ErrorLog("系统维护中-解密失败");
				throw CTrsExp(ERR_WRITE_SESSION,"系统维护中");
			}

		}
		memcpy(rsabuf+enc_len*i,rsabuftmp,enc_len);
		InfoLog("解密后内容rsabuf=[%s]",rsabuf);
		enc_sumlen += enc_len;
		i++;
		offSet = i * 128;
	}
	return string(rsabuf);
}


int CIdentPub::RsaPayDecrypt(unsigned char * enc_data,int data_len,std::string& decrypted)
{
	unsigned char decrypt[2048]={"\0"};
	char base64desEncrypt[4096];

	int offSet = 0;
	int result = -1;
	int total_len = 0;
	int i = 0;

	memset(base64desEncrypt,0x00,sizeof(base64desEncrypt));
	string key = "-----BEGIN PUBLIC KEY-----\n";
	key       +="MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCbowBDWWZw71okCHdn4/mvNvDI\n";
	key       +="NFd7CVZM4pQGajdu0IcALcGmp0OSbslEY5gJNInlOFGY+2mYV1796Ebp3KlSm/c2\n";
	key       +="57hzGibs6ApyfFxs/FB8T0zv4syBTf9e8fAVqyoBplc9ghAw3bFusalnsNFyh1eu\n";
	key       +="opRUWtHNN0rm7rjpcwIDAQAB\n";
	key       +="-----END PUBLIC KEY-----";

	RSA * rsa = createRSA((unsigned char * )key.c_str(),1);
	if(rsa==NULL) return -1;

	ads_base64_dec((char*)enc_data,data_len,base64desEncrypt,sizeof(base64desEncrypt));
	//InfoLog("base64desEncrypt=[%s]",base64desEncrypt);

	// 对数据分段解密
	while (data_len - offSet > 0) {
		if (data_len - offSet > MAX_DECRYPT_BLOCK) {
			result = RSA_public_decrypt(MAX_DECRYPT_BLOCK,(unsigned char *)enc_data+offSet,decrypt,rsa,RSA_PKCS1_PADDING);
		} else {        
			result = RSA_public_decrypt(data_len - offSet,(unsigned char *)enc_data+offSet,decrypt,rsa,RSA_PKCS1_PADDING);
		}
		if(result== -1 ) return result;

		decrypted += std::string((char *)decrypt,result);
		i++;
		offSet = i * MAX_DECRYPT_BLOCK;
		total_len += result;
	}

	return total_len;
}

/*------------------------------------------------------------------------
 *Function Name:MapToJson
 *       Desc: 将Map转为Json格式
 *      Input: 
 *            map:Map键值对
 *     Return:json
 *       Auth: JiaYeHui
 *       Date: 2018-04-04                                                
 *-----------------------------------------------------------------------*/
string CIdentPub::MapToJson(const map<string, string> &m)
{  
    Document document;    
    Document::AllocatorType& allocator = document.GetAllocator();   
    Value root(kObjectType);    
  
    Value key(kStringType);    
    Value value(kStringType);   
  
    for(map<string, string>::const_iterator it = m.begin(); it != m.end(); ++it)
    {  
        key.SetString(it->first.c_str(), allocator);    
        value.SetString(it->second.c_str(), allocator);    
        root.AddMember(key, value, allocator);  
    }  
  
    StringBuffer buffer;    
    Writer<StringBuffer> writer(buffer);    
    root.Accept(writer);    
    return buffer.GetString();    
}

//JiaYeHui -- 换掉以下的两个函数，这行以下都删除
//mm_cgi-5
// 秘钥不同，每次拷贝程序，都应该用这2个秘钥函数覆盖
int CIdentPub::GetDESKey(string& syscode, string& des_key)
{
    static string des_key_array[]={
    /*20000001-20000010*/
    "88FF66B621F86A1B55CF4DB179099AB9",
    "EC0346DDD7686A1B5A81B15E1272911D",
    "0445699282886A1B5C4C4BC6723C753D",
    "869D630C7E286A1B5C1F1E6302A1BA8D",
    "163E58CB6D786A1B55A3BBBF02DD6038",
    "FC0FE70F2F886A1B5F74950C6A35BB66",
    "02A8578D23D86A1B53A99D03170DD782",
    "165367BE53D86A1B567918A4609E8710",
    "FFDD2EF24F086A1B53A9A0BC3D3DBA28",
    "820E170F5E386A1B5111B0A89E2B7CC5",
    /*20000011-20000020*/
    "C935C94C94E86A1B52AE7068C7FCB72A",
    "9B78B72ACAB86A1B5236620ED3A68CAA",
    "85B4E81963686A1B59166E3A3A133C2F",
    "FAB49C4899F86A1B5A249C8A5E8314C1",
    "E370D29CFFB86A1B52966BC7D17F9A3D",
    "4A0EF7D238B86A1B58AB2A3A21703373",
    "76C1D0267EA86A1B56694377E4CD4D5A",
    "28DBE0EAA8B86A1B5C9740162F0B847D",
    "21B6662279B86A1B544A5D9E993C1737",
    "44DEAF82111DB2B04882B12D77C3BE34",
    /*20000021-20000030*/
    "B890EE6A03986A1B5A37DC32A91F4713",
    "602C0EDEC5386A1B573762D89EB54051",
    "5AE4EE73EE386A1B536DA1C925F5A3B8",
    "711C4E0750586A1B5AECF976456E9585",
    "E4598EF49D086A1B5EBDFC3F5037CE72",
    "2454CE2E0CB86A1B5654DE19A8544692",
    "1A6C2E5EFD586A1B5B6D46278E4A311F",
    "2535DE6EF4886A1B58D39CF39919F288",
    "732BAE3855786A1B55A77DA253137BA2",
    "64A30EE1C0486A1B514371D3D30725A3",
    /*20000031-20000040*/
    "CCF31E4205E86A1B5898F0799959C8DD",
    "20632EA8EB486A1B5E93494CD7BB4BBC",
    "225FBE945F686A1B526F4CF982949DDC",
    "28436EFE60886A1B5986A8702B9CB7CF",
    "39780EF2DEC86A1B5B88B9669F6F656A",
    "B49FEE1B4CE86A1B5AE8803B0B0B4290",
    "CED31E4AFB486A1B50E205B05ECA96B3",
    "558E4EEA77686A1B54BC19A429ABDFCB",
    "C823DEC865686A1B5F38C62898544920",
    "1E0C0E4F62386A1B54E75AED3B703432",
    /*20000041-20000050*/
    "6940AE292CA86A1B5D16AECB228270B6",
    "C935C94C94E86A1B52AE7068C7FCB72A",
    "4B821E7CBE886A1B5934AD6E4B1A2280",
    "4115BE3A8AC86A1B561A937744D893CB",
    "43715E09E1886A1B5FE2129B1E3F4C0B",
    "1A312EB2AFD86A1B54DC9AB19B73AE43",
    "907B3E08E9A86A1B517D44BD459D48B9",
    "83ECFEF204486A1B52F922D96A7C8630",
    "B15D7E0485886A1B5257410D6FC8F7C9",
    "54AC8E1EC3A86A1B5ABA80D7D92B0F6C",
    /*20000051-20000060*/
    "E2EE9E04A6786A1B5FB28B375FC52107",
    "FAB2DE91E7786A1B5A4544DA8AAC8B62",
    "2C89EE211CB86A1B5C80F867C8CC4413",
    "D9C98EDB1A586A1B574CD98224124DF7",
    "E635CEEA1A286A1B579E520C1397B6ED",
    "5E6CBE25D8F86A1B5EF66C7044A142B2",
    "776D4E07CBD86A1B5876F88EFC2F49F0",
    "694E4EB726386A1B5AE6C818CD9C11C8",
    "6BE21E5042C86A1B51B27BA7B247C302",
    "0B1CEE0F40486A1B51C943854C9313DE",
    /*20000061-20000070*/
    "868BDE0230186A1B533A5538E5D88B77",
    "1C72DE115C686A1B5A6C64AD4FEE4067",
    "7CAB7E56D0A86A1B526B50C0D9DD6804",
    "1C399E734CA86A1B5D17B779C81143D9",
    "589E5E6FDF086A1B526F61FDA529718B",
    "F5CDBED4D3686A1B567B408780FCF598",
    "AA4EDEF5DBC86A1B54629FAB172976DC",
    "C7447EC542086A1B580FB5229B170493",
    "C5ED6E1CB9F86A1B5F6903E59CFDCF2B",
    "F9218EF73C286A1B5FAB63F880DA5167",
    /*20000071-20000080*/
    "94B26E6D67386A1B55938ECAB6F9FEDC",
    "F7D62E55C6C86A1B5414DFBFE18D11D6",
    "E6EA8E2D98D86A1B57AFE5B43AAD0232",
    "5ED42E7DA4986A1B5BEF93E4C70F16B5",
    "279F5E69D7D86A1B5948E1AB62A048C2",
    "3F580E28FE986A1B5DE0F46EAA74928A",
    "2FF65EAB65986A1B5AE673507DEDE16B",
    "89DAAE3AF1486A1B50AA01E25346BF59",
    "DB964E13B5786A1B54A681D6437ADE04",
    "7A036E4AF6986A1B51CBE2C9C72EF021",
    /*20000081-20000090*/
    "4C166E7896986A1B5D7088A151241F09",
    "92E46ED63B786A1B5B7A32D535BAF928",
    "61663E7B47886A1B52FDE5BDB8CB3B2E",
    "D44F1EF63EF86A1B5193D2B794ABCA6C",
    "CAB88E1DF5C86A1B51F2F957B9AE35DB",
    "E9845E24BF886A1B5F20F90B01EB6316",
    "A33E3EAECEC86A1B598FB06E8FA409FC",
    "3A741E165A586A1B56876906AB7CACA8",
    "7B2F0EDE57886A1B58879EEB4C20EB20",
    "FD61CE1570986A1B57A1DA6E8DF99F44",
    /*20000091-20000100*/
    "BFB37E70E1586A1B5DDA3027242A0125",
    "9CA69E0BE9486A1B57156CC5FA88BDA2",
    "40AC8E976AE86A1B59585EA9BB8F4B6A",
    "05AF9E8F34F86A1B542C0E618601C27F",
    "C11FCEABA6186A1B503D9F7C28D9E1A4",
    "2A15AEF1AA186A1B5F079DB4EB80CF9D",
    "8D7C0E59E0986A1B536EEEFFE2ED49E6",
    "61656E9BC3786A1B54473DF0D1AEA105",
    "382F4E5047486A1B5C2ED2F382EC898D",
    "C296AE5B63186A1B55D9CCB759F22D61",
    /*20000101-20000200*/
    "88FF66B621F86A1B55CF41A179099AB9",
    "EC0346DDD7686A1B5A81B1A21272911D",
    "0445699282886A1B5C4C41A3723C753D",
    "869D630C7E286A1B5C1F11A402A1BA8D",
    "163E58CB6D786A1B55A3B1A502DD6038",
    "FC0FE70F2F886A1B5F7491A66A35BB66",
    "02A8578D23D86A1B53A991A7170DD782",
    "165367BE53D86A1B567911A8609E8710",
    "FFDD2EF24F086A1B53A9A1A93D3DBA28",
    "820E170F5E386A1B5111B11A9E2B7CC5",
    "C935C94C94E86A1B52AE7111C7FCB72A",
    "9B78B72ACAB86A1B52366112D3A68CAA",
    "85B4E81963686A1B591661133A133C2F",
    "FAB49C4899F86A1B5A2491145E8314C1",
    "E370D29CFFB86A1B52966115D17F9A3D",
    "4A0EF7D238B86A1B58AB211621703373",
    "76C1D0267EA86A1B56694117E4CD4D5A",
    "28DBE0EAA8B86A1B5C9741182F0B847D",
    "21B6662279B86A1B544A5119993C1737",
    "44DEAF8211186A1B5882B12A77C3BE34",
    "B890EE6A03986A1B5A37D121A91F4713",
    "602C0EDEC5386A1B573761229EB54051",
    "5AE4EE73EE386A1B536DA12325F5A3B8",
    "711C4E0750586A1B5AECF124456E9585",
    "E4598EF49D086A1B5EBDF1255037CE72",
    "2454CE2E0CB86A1B5654D126A8544692",
    "1A6C2E5EFD586A1B5B6D41278E4A311F",
    "2535DE6EF4886A1B58D391289919F288",
    "732BAE3855786A1B55A7712953137BA2",
    "64A30EE1C0486A1B5143713AD30725A3",
    "CCF31E4205E86A1B5898F1319959C8DD",
    "20632EA8EB486A1B5E934132D7BB4BBC",
    "225FBE945F686A1B526F413382949DDC",
    "28436EFE60886A1B5986A1342B9CB7CF",
    "39780EF2DEC86A1B5B88B1359F6F656A",
    "B49FEE1B4CE86A1B5AE881360B0B4290",
    "CED31E4AFB486A1B50E201375ECA96B3",
    "558E4EEA77686A1B54BC113829ABDFCB",
    "C823DEC865686A1B5F38C13998544920",
    "1E0C0E4F62386A1B54E7514A3B703432",
    "6940AE292CA86A1B5D16A141228270B6",
    "C935C94C94E86A1B52AE7142C7FCB72A",
    "4B821E7CBE886A1B5934A1434B1A2280",
    "4115BE3A8AC86A1B561A914444D893CB",
    "43715E09E1886A1B5FE211451E3F4C0B",
    "1A312EB2AFD86A1B54DC91469B73AE43",
    "907B3E08E9A86A1B517D4147459D48B9",
    "83ECFEF204486A1B52F921486A7C8630",
    "B15D7E0485886A1B525741496FC8F7C9",
    "54AC8E1EC3A86A1B5ABA815AD92B0F6C",
    "E2EE9E04A6786A1B5FB281515FC52107",
    "FAB2DE91E7786A1B5A4541528AAC8B62",
    "2C89EE211CB86A1B5C80F153C8CC4413",
    "D9C98EDB1A586A1B574CD15424124DF7",
    "E635CEEA1A286A1B579E51551397B6ED",
    "5E6CBE25D8F86A1B5EF6615644A142B2",
    "776D4E07CBD86A1B5876F157FC2F49F0",
    "694E4EB726386A1B5AE6C158CD9C11C8",
    "6BE21E5042C86A1B51B27159B247C302",
    "0B1CEE0F40486A1B51C9416A4C9313DE",
    "868BDE0230186A1B533A5161E5D88B77",
    "1C72DE115C686A1B5A6C61624FEE4067",
    "7CAB7E56D0A86A1B526B5163D9DD6804",
    "1C399E734CA86A1B5D17B164C81143D9",
    "589E5E6FDF086A1B526F6165A529718B",
    "F5CDBED4D3686A1B567B416680FCF598",
    "AA4EDEF5DBC86A1B54629167172976DC",
    "C7447EC542086A1B580FB1689B170493",
    "C5ED6E1CB9F86A1B5F6901699CFDCF2B",
    "F9218EF73C286A1B5FAB617A80DA5167",
    "94B26E6D67386A1B55938171B6F9FEDC",
    "F7D62E55C6C86A1B5414D172E18D11D6",
    "E6EA8E2D98D86A1B57AFE1733AAD0232",
    "5ED42E7DA4986A1B5BEF9174C70F16B5",
    "279F5E69D7D86A1B5948E17562A048C2",
    "3F580E28FE986A1B5DE0F176AA74928A",
    "2FF65EAB65986A1B5AE671777DEDE16B",
    "89DAAE3AF1486A1B50AA01785346BF59",
    "DB964E13B5786A1B54A68179437ADE04",
    "7A036E4AF6986A1B51CBE18AC72EF021",
    "4C166E7896986A1B5D70818151241F09",
    "92E46ED63B786A1B5B7A318235BAF928",
    "61663E7B47886A1B52FDE183B8CB3B2E",
    "D44F1EF63EF86A1B5193D18494ABCA6C",
    "CAB88E1DF5C86A1B51F2F185B9AE35DB",
    "E9845E24BF886A1B5F20F18601EB6316",
    "A33E3EAECEC86A1B598FB1878FA409FC",
    "3A741E165A586A1B56876188AB7CACA8",
    "7B2F0EDE57886A1B588791894C20EB20",
    "FD61CE1570986A1B57A1D19A8DF99F44",
    "BFB37E70E1586A1B5DDA3191242A0125",
    "9CA69E0BE9486A1B57156192FA88BDA2",
    "40AC8E976AE86A1B59585193BB8F4B6A",
    "05AF9E8F34F86A1B542C01948601C27F",
    "C11FCEABA6186A1B503D919528D9E1A4",
    "2A15AEF1AA186A1B5F079196EB80CF9D",
    "8D7C0E59E0986A1B536EE197E2ED49E6",
    "61656E9BC3786A1B54473198D1AEA105",
    "382F4E5047486A1B5C2ED19982EC898D",
    "C296AE5B63186A1B55D9C2AA59F22D61" 
    };

     /*syscode长度要求是8，内容是数字*/
     if ((syscode.length() != 8))
     {
          return -1;
     }
     if(!ads_isnum((char *)syscode.c_str()))
     {
         return -1;
     }
     int sys = atoi((char*)syscode.c_str());
     int sub = sys - 20000000;
     if(sub<1 || sub>200)
     {
          return -1;
     }

     //des_key = CIdentPub::HEX_TO_ASCII((char *)des_key_array[sub-1].c_str(), 32) ;
     char outsrc[32];
     memset(outsrc,0x00,sizeof(outsrc));
     ads_hextoasc2( (char *)des_key_array[sub-1].c_str(), 32, outsrc, sizeof(outsrc));
     des_key=outsrc;

     return 0;
}

int CIdentPub::GetSignatureKey(string& syscode, string& sig_key)
{
     static string sig_key_array[]={
     /*20000001-20000010*/
          "asdfipkg123",//20000001
          "asdfipkg123",
          "asdfipkg123",
          "asdfipkg123",
          "asdfipkg123",
          "asdfipkg123",
          "asdfipkg123",
          "asdfipkg123",
          "asdfipkg123",
          "asdfipkg123",
     /*20000011-20000020*/
          "asdfipkg123",//20000011
          "asdfipkg123",
          "asdfipkg123",
          "asdfipkg123",
          "asdfipkg123",
          "asdfipkg123",
          "asdfipkg123",
          "asdfipkg123",
          "asdfipkg123",
          "asdfipkg123",
     /*20000021-20000030*/
          "asdfipkg123",//20000021
          "asdfipkg123",
          "asdfipkg123",
          "asdfipkg123",
          "asdfszpay53",
          "asdfszpay523",
          "asdfszpay5g123",
          "asdfszpay53",
          "asdfszpay523",
          "asdfiszpay5123",

     /*20000031-20000040*/
          "ak3dszpay523",//20000031
          "ak30szpay523",
          "ak30szpay523",
          "ak3dszpay523",
          "ak3dszpay523",
          "ak3dszpay523",
          "ak30szpay523",
          "ak3dszpay523",
          "ak3dszpay523",
          "ak3dszpay523",

     /*20000041-20000050*/
          "ak3kszpay5g123",//20000041
          "ak3jszpay5g123",
          "ak3lszpay53",
          "ak3fszpay593",
          "ak3lszpay503",
          "ak3dszpay513",
          "ak3dszpay553",
          "ak3dszpay5f3",
          "ak3dszpay5b3",
          "ak3aszpay523",     

     /*20000051szpay5060*/
          "ak3dszpay523",//20000051
          "ak3dszpay523",
          "ak3dszpay523",
          "ak3dszpay523",
          "ak3dszpay523",
          "ak3dszpay523",
          "ak3dszpay523",
          "ak3dszpay523",
          "ak37szpay523",
          "a1dfszpay53",

     /*20000061-20000070*/
          "ak3dszpay523",//20000061
          "ak3dszpay523",
          "ak3dszpay523",
          "ak3dszpay523",
          "ak3lszpay598",
          "ahfaszpay57",
          "ak3dszpay5tup",
          "ak3yszpay5lan",
          "ak3dszpay5837",
          "aljfszpay5vn",

     /*20000071-20000080*/
          "ak3dszpay5876jg",//20000071
          "ak3dszpay5gglhh",
          "ak3dszpay50tq98",
          "ak3jszpay500riu",
          "alfaszpay5tyt0",
          "ljalszpay5g0a9",
          "8775szpay5kjal",
          "lfajszpay50270",
          "ljglszpay51012",
          "ak3jszpay58u0ru",

     /*20000081-20000090*/
          "ak3dszpay5lajg",//20000081
          "ak3dszpay5flajg",
          "ak30szpay5flajg",
          "8787szpay5ajg",
          "a088szpay5flajg",
          "ak3lszpay5faflajg",
          "ak3dszpay5jglau98",
          "ak3dszpay5ajglai9",
          "ak3lszpay50q8ljls",
          "ak3fszpay590frur",     

     /*20000091-20000100*/
          "afilszpay5lajg",//20000091
          "adfiszpay5ajg",
          "a0ilszpay5jg",
          "a09iszpay5ajg",
          "ak3fszpay5flajg",
          "ak39szpay5hflajg",
          "ak3dszpay5flajg",
          "ak3dszpay5flajg",
          "ak3dszpay5flajg",
          "ak3dszpay5flajg",
     /*20000101-20000200*/
          "as1a1fip2szpay5k123",
          "as1a2fip3szpay5k123",
          "as1a3fip4szpay5k123",
          "as1a4fip5szpay5k123",
          "as1a5fip6szpay5k123",
          "as1a6fip7szpay5k123",
          "as1a7fip8szpay5k123",
          "as1a8fip9szpay5k123",
          "as1a9fipaszpay5k123",
          "as11afip1szpay5k123",
          "as111fip2szpay5k123",
          "as112fip3szpay5k123",
          "as113fip4szpay5k123",
          "as114fip5szpay5k123",
          "as115fip6szpay5k123",
          "as116fip7szpay5k123",
          "as117fip8szpay5k123",
          "as118fip9szpay5k123",
          "as119fipaszpay5k123",
          "as12afip1szpay5k123",
          "as121fip2szpay5k123",
          "as122fip3szpay5k123",
          "as123fip4szpay5k123",
          "as124fip5szpay5k123",
          "as125fsz6szpay5py53",
          "as126fsz7szpay5py523",
          "as127fsz8szpay5py5g123",
          "as128fsz9szpay5py53",
          "as129fszaszpay5py523",
          "as13afis1szpay5zay5123",
          "ak131dszpszpay5a523",
          "ak1320szpszpay5a523",
          "ak1330szpszpay5a523",
          "ak134dszpszpay5a523",
          "ak135dszpszpay5a523",
          "ak136dszpszpay5a523",
          "ak1370szpszpay5a523",
          "ak138dszpszpay5a523",
          "ak139dszpszpay5a523",
          "ak14adszpszpay5a523",
          "ak141kszpszpay5a5g123",
          "ak142jszpszpay5a5g123",
          "ak143lszpszpay5a53",
          "ak144fszpszpay5a593",
          "ak145lszpszpay5a503",
          "ak146dszpszpay5a513",
          "ak147dszpszpay5a553",
          "ak148dszpszpay5a5f3",
          "ak149dszpszpay5a5b3",
          "ak15aaszpszpay5a523",
          "ak151dszpszpay5a523",
          "ak152dszpszpay5a523",
          "ak153dszpszpay5a523",
          "ak154dszpszpay5a523",
          "ak155dszpszpay5a523",
          "ak156dszpszpay5a523",
          "ak157dszpszpay5a523",
          "ak158dszpszpay5a523",
          "ak1597szpszpay5a523",
          "a116afszpszpay5a53",
          "ak161dszpszpay5a523",
          "ak162dszpszpay5a523",
          "ak163dszpszpay5a523",
          "ak164dszpszpay5a523",
          "ak165lszpszpay5a598",
          "ah166aszpszpay5a57",
          "ak167dszpszpay5a5tup",
          "ak168yszpszpay5a5lan",
          "ak169dszpszpay5a5837",
          "al17afszpszpay5a5vn",
          "ak171dszpay5876jg",
          "ak172dszpay5gglhh",
          "ak173dszpay50tq98",
          "ak174jszpay500riu",
          "al175aszpay5tyt0",
          "lj176lszpay5g0a9",
          "871775szpay5kjal",
          "lf178jszpay50270",
          "lj179lszpay51012",
          "ak18ajszpay58u0ru",
          "ak181dszpay5lajg",
          "ak182dszpay5flajg",
          "ak1830szpay5flajg",
          "871847szpay5ajg",
          "a01858szpay5flajg",
          "ak186lszpay5faflajg",
          "ak187dszpay5jglau98",
          "ak188dszpay5ajglai9",
          "ak189lszpay50q8ljls",
          "ak19afszpay590frur",
          "af191lszpay5lajg",
          "ad192iszpay5ajg",
          "a0193lszpay5jg",
          "a0194iszpay5ajg",
          "ak195fszpay5flajg",
          "ak1969szpay5hflajg",
          "ak197dszpay5flajg",
          "ak198dszpay5flajg",
          "ak199dszpay5flajg",
          "ak2aadszpay5flajg"
     };

     /*syscode长度要求是8，内容是数字*/
     if ((syscode.length() != 8))
     {
          return -1;
     }
     if(!ads_isnum((char *)syscode.c_str()))
     {
         return -1;
     }
     int sys = atoi((char*)syscode.c_str());
     int sub = sys - 20000000;
     if(sub<1 || sub>200)
     {
          return -1;
     }

     sig_key = sig_key_array[sub-1];
     return 0;
}

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
string CIdentPub::WebrsaToOpen(int n_web,string &web_pwd)
{
    char ascbuf[1024];
    char rsabuf[256];

    //将web_pwd的内容base64解析
    memset(ascbuf,0x00,sizeof(ascbuf));
    int len=web_pwd.length();
    len=ads_urlsafe_base64_dec((char *)web_pwd.c_str(),len,ascbuf, sizeof(ascbuf));

    string rsa_pri_key;
    memset(rsabuf,0x00,sizeof(rsabuf));

    string rsa_key;
    CRsaHandler::GetCommRsaPrivateKey(n_web, rsa_key);
    int dec_len=CRsaHandler::private_decrypt((unsigned char *)ascbuf,len,
            (unsigned char *)rsa_key.c_str(),(unsigned char *)rsabuf);
    if(dec_len == -1)
    {
        ErrorLog("系统维护中-解密失败");
        throw CTrsExp(ERR_WRITE_SESSION,"系统维护中");
    }

    if(dec_len < 2)
    {
        ErrorLog("系统维护中-RSA解密后长度<2,buf=[%s]",rsabuf);
        throw CTrsExp(ERR_WRITE_SESSION,"系统维护中");
    }
    if(dec_len > 100)
    {
        ErrorLog("系统维护中-RSA解密后长度>20,buf=[%s]",rsabuf);
        throw CTrsExp(ERR_WRITE_SESSION,"系统维护中");
    }

    return string(rsabuf);
}

void CIdentPub::deleteAllMark(string &s, const string &mark)
{

	size_t nSize = mark.size();
	while(1)
	{
		size_t pos = s.find(mark);    
		if(pos == string::npos)
		{
			return;
		}

		s.erase(pos, nSize);

	}
}

int CIdentPub::Http2MerLink(const std::string & strUrl, CStr2Map & reqMap, CStr2Map & respMap)
{
	const static string MERLINK_DES_KEY  = "326jklojhydhjuiokjmhmjhg";
    const static string MERLINK_MD5_KEY = "123456789";
    const static string ELE_SEP = "\"";
    const static string NV_SEP  = ":";
    const static string LN_SEP  = ",";

				string strResponse,reqbody,sendbody,respdata;
				Tools::MapToStrNoEncode(reqMap,reqbody);
				InfoLog("repbody加密前=[%d][%s]",reqbody.length(),reqbody.c_str());

				string strMd5Data = reqbody+  MERLINK_MD5_KEY;
				InfoLog("strMd5Data=[%d][%s]",strMd5Data.length(),strMd5Data.c_str());
				string newMd5Sig = Tools::lower(Tools::MD5(strMd5Data).substr(0,32));
				InfoLog("newMd5Sig=[%d][%s]",newMd5Sig.length(),newMd5Sig.c_str());

    string strJsonStr = "";
    strJsonStr += "{";

				CStr2Map::iterator iter  = reqMap.begin();
				CStr2Map::iterator iter_end  = reqMap.end();

				if(reqMap.size() == 0)
				{
					return -1;
				}

				while(iter != iter_end)
				{
								if( !(iter->second.empty()) )
								{
													strJsonStr += ELE_SEP + iter->first + ELE_SEP + NV_SEP + 
																																			ELE_SEP + iter->second + ELE_SEP + LN_SEP;
								}
				   	iter++ ;
				}

				strJsonStr += "\"signatureInfo\":\""  + newMd5Sig+  "\"}";

		 	InfoLog("strJsonStr=[%d][%s]",strJsonStr.length(),strJsonStr.c_str());

				string postData = des3_base64_ml(MERLINK_DES_KEY,strJsonStr);

				char* outstrPost = curl_escape(postData.c_str(),postData.length());

				postData = strUrl + "?req=" + outstrPost;
				InfoLog("MERLINK postData=[%d][%s]",postData.length(),postData.c_str());

				if(-1 == CIdentPub::HttpGet(postData,strResponse))
   {          
				  	InfoLog("http post fail %s", strUrl.c_str()); 
				  	return -1;
				}

				int iLenSrc = strResponse.length();

				string safeBase64=Tools::ReplaceStr(strResponse,"\n","");
				InfoLog("返回数据 safeBase64=[%d][%s]",safeBase64.length(),safeBase64.c_str());

				respdata = Decdes3_base64_ml(MERLINK_DES_KEY,safeBase64);
				InfoLog("解密之后respdata=[%d][%s]",respdata.length(),respdata.c_str());
				if(iLenSrc > 2048)
				{
					ErrorLog("渠道响应数据错误[%d][%s]", iLenSrc,(strResponse.substr(0,2000)).c_str());
					throw CTrsExp(ERR_JAVA_RETURN,"渠道响应数据错误");
					return -1;
				}

    std::size_t found = respdata.find('}');
		  if (found == std::string::npos)  
			 {
									ErrorLog("渠道响应数据格式错误-Resp");
									throw CTrsExp(ERR_JAVA_RETURN,"渠道响应数据格式错误");
									return -1;
				}

    respdata = respdata.substr(0,found+1);
    InfoLog("respdata=[%d][%s]",respdata.length(),respdata.c_str());

				if(-1 == parsePubRespJson(respdata,respMap))
				{
					ErrorLog("渠道响应数据格式错误-Resp");
					throw CTrsExp(ERR_JAVA_RETURN,"渠道响应数据格式错误");
     return -1;
				}

    return 0;
}

string CIdentPub::des3_base64_ml(const string& des3Key,const string& srcStr)
{
	char srcData[4096];
	int  srcLen;
	int  paddNum;
	int  i;
	char desData[4096];
	char desDataHex[2*4096];
	char base64Encrypt[4096];

	memset(srcData,0x00,sizeof(srcData));
	memset(desData,0x00,sizeof(desData));
	memset(desDataHex,0x00,sizeof(desDataHex));
	memset(base64Encrypt,0x00,sizeof(base64Encrypt));

	srcLen=srcStr.length();
	if(srcLen > 4080 || srcLen <1)
	{
		InfoLog("3des-待加密内容长度错误Len=[%d]", srcLen);
		return "";
	}

	strcpy(srcData,srcStr.c_str());

	//PKCS#5
	paddNum = 8 - srcLen % 8;
	for (i=0; i < paddNum; ++i) 
		srcData[srcLen + i] = paddNum;

	srcLen=srcLen + paddNum;

	string strDesdata = DES_Encrypt(srcData,des3Key,TRIPLE_ECB,"0");

	ads_base64_enc((char*)strDesdata.c_str(),srcLen,base64Encrypt,srcLen*3);

	InfoLog("3des-base64Encrypt=[%s]",base64Encrypt);

	return base64Encrypt;
}

string CIdentPub::Decdes3_base64_ml(const string& des3Key,const string& srcStr)
{
	char srcData[4096];
	int  srcLen;
	char desData[4096];
	char desDataHex[2*4096];
	char base64Encrypt[4096];
	char szHexBuf[4096];

	memset(srcData,0x00,sizeof(srcData));
	memset(desData,0x00,sizeof(desData));
	memset(desDataHex,0x00,sizeof(desDataHex));
	memset(base64Encrypt,0x00,sizeof(base64Encrypt));
	memset(szHexBuf,0x00,sizeof(szHexBuf));

	strcpy(srcData,srcStr.c_str());
	srcLen=srcStr.length();
	if(srcLen > 4080 || srcLen <1)
	{
		InfoLog("3des-待解密内容长度错误Len=[%d]", srcLen);
		return "";
	}

	int nRet = ads_base64_dec(srcData,srcLen,base64Encrypt,sizeof(base64Encrypt));

	ads_asctohex(base64Encrypt,nRet,szHexBuf, sizeof(szHexBuf));
	string strszHex = szHexBuf;

	string strDesdata = DES_Decrypt(strszHex,des3Key,TRIPLE_ECB,"0");

	//转gbk
    return _U2G(strDesdata);
}


string CIdentPub::des3_tohex(const string& des3Key,const string& srcStr)
{
	char srcData[4096];
	int  srcLen;
	int  paddNum;
	int  i;
	char desData[4096];
	char desDataHex[2*4096];
	char base64Encrypt[4096];

	memset(srcData,0x00,sizeof(srcData));
	memset(desData,0x00,sizeof(desData));
	memset(desDataHex,0x00,sizeof(desDataHex));
	memset(base64Encrypt,0x00,sizeof(base64Encrypt));

	srcLen=srcStr.length();
	if(srcLen > 4080 || srcLen <1)
	{
		InfoLog("3des-待加密内容长度错误Len=[%d]", srcLen);
		return "";
	}

	strcpy(srcData,srcStr.c_str());

	//PKCS#5
	paddNum = 8 - srcLen % 8;
	for (i=0; i < paddNum; ++i) 
		srcData[srcLen + i] = paddNum;

	srcLen=srcLen + paddNum;

	string strDesdata = DES_Encrypt(srcData,des3Key,TRIPLE_ECB,"0");

	ads_asctohex((char*)strDesdata.c_str(),srcLen,base64Encrypt,srcLen*3);

	InfoLog("3desEncrypt=[%s]",base64Encrypt);

	return base64Encrypt;
}

unsigned char* CIdentPub::Str2Hex(const char *str, unsigned char *hex, int sLen)
{
	if(str == NULL || hex == NULL)
    {
        return NULL;
    }
	if(sLen%2 == 1)
	{
        return NULL;
	}

    char buf[3];
	memset(buf, 0, 3);

	for (int i=0; i<sLen; i+=2)
	{
		memcpy(buf, &str[i], 2);
		hex[i/2]=(unsigned char)strtol(buf, NULL, 16);
	}
	return hex;
}

char* CIdentPub::Hex2Str(const unsigned char *hex, char *str, int hLen)
{
	if(hex == NULL || str == NULL)
    {
        return NULL;
    }

	memset(str,0,hLen*2+1);
	for (int i=0; i<hLen; i++)
	{
		sprintf((char *)&str[i*2], "%02X", (unsigned char)hex[i]);
	}
	str[hLen*2]='\0';
	return str;
}

int CIdentPub::hex2bytearray(unsigned char* s,unsigned char bits[])
{
	int i,n = 0;
    for(i = 0; s[i]; i += 2) {
        if(s[i] >= 'a' && s[i] <= 'f')
            bits[n] = s[i] - 'a' + 10;
        else bits[n] = s[i] - '0';
        if(s[i + 1] >= 'a' && s[i + 1] <= 'f')
            bits[n] = (bits[n] << 4) | (s[i + 1] - 'a' + 10);
        else bits[n] = (bits[n] << 4) | (s[i + 1] - '0');
        ++n;
    }
    return n;
}

vector<string> CIdentPub::split(const string& s, const string& delim, const bool keep_empty /*= true*/) 
{
    vector<string> result;
    if (delim.empty()) {
        result.push_back(s);
        return result;
    }
    string::const_iterator substart = s.begin(), subend;
    while (true) {
        subend = search(substart, s.end(), delim.begin(), delim.end());
        string temp(substart, subend);
        if (keep_empty || !temp.empty()) {
            result.push_back(temp);
        }
        if (subend == s.end()) {
            break;
        }
        substart = subend + delim.size();
    }
    return result;
}

unsigned char char2hex(char ch)
{
	static const char *hex="0123456789ABCDEF";
	for(unsigned char i=0;i!=16;++i)
	{
		if(ch == hex[i])
			return i;
	}
	return 0;
}

char* CIdentPub::solve(char* dest,const char*src)
{
	int i = 0;
	int cnt = 0;
	unsigned char* d = (unsigned char*)dest;

	while(*src)
	{
		if(i&1)
		{
			d[cnt++]|=char2hex(*src);
		}
		else
			d[cnt]=char2hex(*src)<<4;
		src++;
		i++;
	}
	return dest;

}


static int GetFileName(const char *path, char *name)
{
	const char *sTmp = strrchr(path, '/');
	if (NULL == sTmp)
	{
		sTmp = strrchr(path, '\\');
	}
	if (NULL != sTmp)
		strcpy(name, sTmp + 1);
	else
        strcpy(name, path);

    return 0;
}

static int MySystem(const char *cmd)
{
    int status = 0;
	// 执行 system 命令
	status = system(cmd);
	
	if (WIFEXITED(status)) 	
	{
		// 命令正常退出
		int exit_status = WEXITSTATUS(status);
		if (exit_status != 0) 
		{
			ErrorLog("system exec cmd[%s] failed: %d", cmd, exit_status);
			return -1;
		}
	}
	else if(WIFSIGNALED(status)) 
	{
		// 命令被信号中断
		ErrorLog("cmd[%s] was interrupted by a signal[%d]", cmd, WTERMSIG(status));
		throw CTrsExp(ERR_JAVA_RETURN,"渠道响应数据格式错误"); 
	}

	return 0;
}

static int GetExeDir(char* dir)
{
    int i;
    int rslt = readlink("/proc/self/exe", dir, 255);
    if (rslt < 0 || (rslt >= 255))
    {
        return -1;
    }
    dir[rslt] = '\0';
    for (i = rslt; i >= 0; i--)
    {
        if (dir[i] == '/')
        {
            dir[i + 1] = '\0';
            break;
        }
    }
    return 0;
}

static bool IsDir(const char *path)
{
    struct stat st;
    if (stat(path, &st) == 0)
        return S_ISDIR(st.st_mode);
    else
        return false;
}

int CIdentPub::UploadFile2MinIO(S3Cfg &cfg, const char *localPath, const char *remotePath, char *objectName)
{
    int ret = 0;
    char cmd[1024] = {0}, fileName[128] = {0}, realPath[256] = {0};

    if (NULL != remotePath)
        strcpy(realPath, remotePath);

    if (NULL != objectName)
    {
        GetFileName(localPath, fileName);
        if (NULL == remotePath)
            strcpy(objectName, fileName);
        else
        {
            if (remotePath[strlen(remotePath)-1] == '/')
                sprintf(objectName, "%s%s", remotePath, fileName);
            else
                strcpy(objectName, remotePath);
        }
    }

    snprintf(cmd, sizeof(cmd), "mc stat \"%s/%s\" >/dev/null && mc cp \"%s\" \"%s/%s/%s\" >/dev/null",\
            cfg.SvrAlias.c_str(), cfg.BucketName.c_str(), localPath, cfg.SvrAlias.c_str(), cfg.BucketName.c_str(), realPath);
    DebugLog("UploadFile2MinIO: %s", cmd);
    ret = MySystem(cmd);
    if (ret)
    {
        ErrorLog( "exec cmd [%s] failed!", cmd);
        return ret;
    }

    return 0;
}


int CIdentPub::DownloadFileFromMinIO(S3Cfg &cfg, const char *objectName, char *localPath)
{
    if (!strcmp(localPath, ""))
        GetExeDir(localPath);

    if (IsDir(localPath)) //路径
	{
        char fileName[128] = {0};
        GetFileName(objectName, fileName);
        strcat(localPath, "/");
        strcat(localPath, fileName);
    }

    char cmd[1024] = {0};
    snprintf(cmd, sizeof(cmd), "mc stat \"%s/%s/%s\" >/dev/null && mc cp \"%s/%s/%s\" \"%s\" >/dev/null",\
            cfg.SvrAlias.c_str(), cfg.BucketName.c_str(), objectName, cfg.SvrAlias.c_str(), cfg.BucketName.c_str(), objectName, localPath);
	InfoLog(" cmd [%s] ", cmd);
    int ret = MySystem(cmd);
    if (ret)
    {
        ErrorLog("exec cmd [%s] failed!", cmd);
        return ret;
    }
    return 0;
}

int CIdentPub::CheckFileFromMinIO(S3Cfg &cfg, const char *objectName)
{
    char cmd[1024] = {0};
    snprintf(cmd, sizeof(cmd), "mc stat \"%s/%s/%s\" >/dev/null",\
            cfg.SvrAlias.c_str(), cfg.BucketName.c_str(), objectName);
	InfoLog(" cmd [%s] ", cmd);
    int ret = MySystem(cmd);
    if (ret)
    {
        ErrorLog("exec cmd [%s] failed!", cmd);
        return ret;
    }
    return 0;
}

bool CIdentPub::exists(const string& path)
{
    return ::access(path.c_str(), F_OK) ? false : true;
}

static int GetExeFullPath(char *path)
{
    int rslt = readlink("/proc/self/exe", path, 255);
    if (rslt < 0 || (rslt >= 255))
    {
        return -1;
    }
    path[rslt] = '\0';
    return 0;
}

static int GetAbsPath(const char *path, char *absPath)
{
    if (NULL == path || !strcmp(path, ""))
        return -1; //路径错误
    
    if (path[0] == '/') //绝对路径
        strcpy(absPath, path);
    else
    {
        char curdir[256] = {0};
        GetExeDir(curdir);
        sprintf(absPath, "%s%s", curdir, path);
    }

    return 0;
}

static string GetIPList()
{
    string iplist;
	struct sockaddr_in *sin = NULL;
	struct ifaddrs *ifa = NULL, *ifList;
	if (getifaddrs(&ifList) < 0)
	{
        ErrorLog("GetIPList->getifaddrs failed!");
		return "";
	}
	for (ifa = ifList; ifa != NULL; ifa = ifa->ifa_next)
	{
		if (ifa->ifa_addr->sa_family == AF_INET && 0 != strcmp(ifa->ifa_name, "lo"))
		{
            sin = (struct sockaddr_in *)ifa->ifa_addr;
            iplist.append(inet_ntoa(sin->sin_addr));
            iplist.append(",");
		}
	}

    freeifaddrs(ifList);
    
    if (!iplist.empty())
        return iplist.substr(0, iplist.size()-1);
    else
        return "";
}

int CIdentPub::SendAlarm2(const char *format, ...)
{
    char errmsg[521] = {0};
    va_list args;
    va_start(args, format);
    vsnprintf(errmsg, sizeof(errmsg)-1, format, args);
    va_end(args);

    char  path[256] = {0}, sendalarm[256] = {0}, cmd[1024] = {0};
    GetExeFullPath(path);
    //发送邮件
    GetAbsPath("/usr/bin/sendalarm.sh", sendalarm);
    snprintf(cmd, sizeof(cmd), "%s \"ip: %s\\nservice: %s\\n%s\" > /dev/null 2>&1", sendalarm, GetIPList().c_str(), path, errmsg);
    int ret = MySystem(cmd);
    return ret;
}

//ST开头是包装PID， WK开头是组装PID,其它情况是SN号
bool CIdentPub::IsPIDNumber(string id)
 {
    if(id.substr(0, 2) == "WK" || id.substr(0, 2) == "ST" || id.substr(0, 2) == "MO")
        return true;
    return false;
 }

 string CIdentPub::calculateFileHash(const std::string& filePath)
 {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    ifstream file(filePath, std::ios::binary);
    if (!file)
    {
        ErrorLog("无法打开文件[%s]",filePath.c_str());
		return "";
    }
    char buffer[8192]; // 增加缓冲区大小以提高效率
    while (file.read(buffer, sizeof(buffer))) {
        SHA256_Update(&sha256, buffer, file.gcount());
    }

    // 处理最后一部分
    if (file.gcount() > 0) {
        SHA256_Update(&sha256, buffer, file.gcount());
    }

    SHA256_Final(hash, &sha256);
    // 转换为十六进制字符串
    std::ostringstream oss;
    for (const auto& byte : hash) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return oss.str();

 }

string CIdentPub::escape_svn_path(const std::string& svn_path) {
    std::ostringstream escaped;
    for (char c : svn_path) {
        // 转义特殊字符
        switch (c) {
            case ' ':
            case '(':
            case ')':
            case '[':
            case ']':
            case '{':
            case '}':
            case '$':
            case '&':
            case ';':
            case '<':
            case '>':
            case '|':
            case '`':
            case '"':
            case '\'':
            case '\\':
                escaped << '\\'; // 添加反斜杠
                // Fall through to add the character itself
            default:
                escaped << c; // 添加当前字符
        }
    }
    return escaped.str();
}

/*
string CIdentPub::TransformJson(const string& rawJson)
{
    // 创建新的 JSON 结构
    Document newDocument;
    newDocument.SetObject();
    Document::AllocatorType& allocator = newDocument.GetAllocator();

    // 设置 errorcode 和 errormessage
    newDocument.AddMember("errorcode", "0000", allocator);
    newDocument.AddMember("errormessage", "OK", allocator);

    // 创建 array
    Value array(kArrayType);
    set<string> uniqueModules; // 用于存储不同模块的集合

    // 如果原始 JSON 字符串为空，直接返回带空数组的 JSON
    if (rawJson.empty()) {
        newDocument.AddMember("array", array, allocator);
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        newDocument.Accept(writer);
        return buffer.GetString();
    }

    // 解析 JSON
    Document document;
    document.Parse(rawJson.c_str());

    // 获取 top_posnames 的 buckets
    if (document.HasMember("aggregations") && document["aggregations"].HasMember("top_posnames")) {
        const Value& buckets = document["aggregations"]["top_posnames"]["buckets"];
        
        for (SizeType i = 0; i < buckets.Size(); ++i) {
            const Value& bucket = buckets[i];

            // pos_name
            std::string posName = bucket["key"].GetString();

            // 创建模块数组
            Value moduleArray(kArrayType);
            const Value& modules = bucket["nested_funclist"]["by_func"]["buckets"];
            for (SizeType j = 0; j < modules.Size(); ++j) {
                const Value& module = modules[j];

                // 模块信息
                Value moduleInfo(kObjectType);
                string moduleKey = module["key"].GetString();
                uniqueModules.insert(moduleKey); // 添加模块到集合中

                moduleInfo.AddMember("module_name", Value(module["description"]["hits"]["hits"][0]["_source"]["description"].GetString(), allocator), allocator);
                moduleInfo.AddMember("total_count", Value(std::to_string(module["total_count"]["value"].GetInt()).c_str(), allocator), allocator);
                moduleInfo.AddMember("one_success_count", Value(std::to_string(module["success_count"]["doc_count"].GetInt()).c_str(), allocator), allocator);
                moduleInfo.AddMember("one_success_rate", Value(module["total_count"]["value"].GetInt() > 0 ? 
                    std::to_string((module["success_count"]["doc_count"].GetInt() * 100.0) / module["total_count"]["value"].GetInt()).c_str() : "0.00", allocator), allocator);

                // 将模块信息加入模块数组
                moduleArray.PushBack(moduleInfo, allocator);
            }

            // 创建包含 pos_name 和模块数组的对象
            Value posInfo(kObjectType);
            posInfo.AddMember("pos_name", Value(posName.c_str(), allocator), allocator);
            posInfo.AddMember("module_arrary", moduleArray, allocator);

            // 将 posInfo 加入 array
            array.PushBack(posInfo, allocator);
        }
    }

    newDocument.AddMember("module_total", Value(std::to_string(uniqueModules.size()).c_str(), allocator), allocator);
    
    // 将 array 加入新的 JSON 结构
    newDocument.AddMember("array", array, allocator);

    // 将新 JSON 转换为字符串
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    newDocument.Accept(writer);

    return buffer.GetString();
}*/

// 根据模块标签或英文名称获取模块描述
static string GetModuleDescription(const string& moduleKey)
{
    // 模块标签/英文名称到描述的映射关系（支持标签和英文名称两种key）
    static map<string, string> moduleMap = {
        // 标签映射
        {"TEST_ITEM_WIFI", "WIFI测试"},
        {"TEST_ITEM_BLUETOOTH", "蓝牙测试"},
        {"TEST_ITEM_PRINT", "打印测试"},
        {"TEST_ITEM_MAGNETIC_CARD", "磁条卡"},
        {"TEST_ITEM_INSERT_CARD", "IC卡"},
        {"TEST_ITEM_TAP_CARD", "非接卡"},
        {"TEST_ITEM_SECURITY_STATUS", "触发测试"},
        {"TEST_ITEM_BATTERY_STATUS", "电池状态"},
        {"TEST_ITEM_TF_CARD", "TF卡测试"},
        {"TEST_ITEM_PSAM_CARD", "PSAM卡测试"},
        {"TEST_ITEM_SIM_CARD", "无线测试"},
        {"TEST_ITEM_BATTERY_LEVEL", "电池电量"},
        {"TEST_ITEM_GPS", "GPS测试"},
        {"TEST_ITEM_ETHERNET", "以太网测试"},
        {"TEST_ITEM_ALGORITHM", "算法测试"},
        {"TEST_ITEM_BUZZER", "蜂鸣器测试"},
        {"TEST_ITEM_USB", "USB测试"},
        {"TEST_ITEM_SERIAL_PORT", "串口测试"},
        {"TEST_ITEM_LED", "LED测试"},
        {"TEST_ITEM_LIGHT_BELT", "灯带测试"},
        {"TEST_ITEM_DISPLAY", "显示测试"},
        {"TEST_ITEM_TOUCH", "触摸测试"},
        {"TEST_ITEM_BACKLIGHT", "背光测试"},
        {"TEST_ITEM_KEYPAD", "按键测试"},
        {"TEST_ITEM_AUDIO", "音频测试"},
        {"TEST_ITEM_SOUND_RECORDING", "录音测试"},
        {"TEST_ITEM_EARPHONE", "耳机测试"},
        {"TEST_ITEM_CAMERA", "摄像头测试"},
        {"TEST_ITEM_SCAN", "扫码测试"},
        {"TEST_ITEM_CHARGE_BASE", "充电底座测试"},
        {"TEST_ITEM_SCANNER", "扫码头测试"},
        {"TEST_ITEM_FINGERPRINT", "指纹测试"},
        {"TEST_ITEM_U_DISK", "U盘测试"},
        {"TEST_ITEM_EX_DISPLAY", "客显屏测试"},
        {"TEST_ITEM_ESIM", "ESIM测试"},
        {"TEST_ITEM_DC", "DC电源"},
        {"TEST_ITEM_RF_PARAMETER_CHECK", "射频参数检查"},
        {"TEST_ITEM_FLASH_LIGHT", "闪光灯测试"},
        {"TEST_ITEM_RF_FIELD_STRENGTH", "非接场强测试"},
        {"TEST_ITEM_VIBRATOR", "振动器测试"},
        {"TEST_ITEM_NFC_READER", "NFC"},
        {"TEST_ITEM_FUNCTIONAL_BASE", "功能底座测试"},
        {"TEST_ITEM_EARPIECE", "听筒测试"},
        {"TEST_ITEM_LIGHT_SENSOR", "光感器测试"},
        {"TEST_ITEM_PROXIMITY_SENSOR", "距离传感器测试"},
        {"TEST_ITEM_G_SENSOR", "重力传感器测试"},
        {"TEST_ITEM_SAR_SENSOR", "SAR传感器测试"},
        {"TEST_ITEM_TAX_CONTROL", "税控测试"},
        {"TEST_ITEM_BUTTON_CELL", "纽扣电池测试"},
        {"TEST_ITEM_MCU", "MCU测试"},
        {"TEST_ITEM_DOCKING_STATION", "扩展坞测试"},
        {"TEST_ITEM_RF_LIGHT", "非接灯测试"},
        {"TEST_ITEM_IC_LIGHT", "IC卡灯测试"},
        {"TEST_ITEM_LOW_POWER", "低功耗测试"},
        {"TEST_ITEM_DEFAULT", "维修测试"},
        // 英文名称映射
        {"Wifi", "WIFI测试"},
        {"Bluetooth", "蓝牙测试"},
        {"Printer", "打印测试"},
        {"MAG", "磁条卡"},
        {"IC", "IC卡"},
        {"RF", "非接卡"},
        {"Trigger", "触发测试"},
        {"BatteryStatus", "电池状态"},
        {"TF", "TF卡测试"},
        {"PSAM", "PSAM卡测试"},
        {"SIM", "无线测试"},
        {"BatteryLevel", "电池电量"},
        {"GPS", "GPS测试"},
        {"Ethernet", "以太网测试"},
        {"Algorithm", "算法测试"},
        {"Beeper", "蜂鸣器测试"},
        {"USB", "USB测试"},
        {"SerialPort", "串口测试"},
        {"LED", "LED测试"},
        {"LightBelt", "灯带测试"},
        {"ScreenDisplay", "显示测试"},
        {"Touch", "触摸测试"},
        {"Backlight", "背光测试"},
        {"Keypad", "按键测试"},
        {"Audio", "音频测试"},
        {"Record", "录音测试"},
        {"Earphone", "耳机测试"},
        {"Camera", "摄像头测试"},
        {"ScanCode", "扫码测试"},
        {"ChargeBase", "充电底座测试"},
        {"Scanner", "扫码头测试"},
        {"Fingerprint", "指纹测试"},
        {"UDisk", "U盘测试"},
        {"ExtendScreenDisplay", "客显屏测试"},
        {"ESIM", "ESIM测试"},
        {"DC", "DC电源"},
        {"RFParameterCheck", "射频参数检查"},
        {"FlashLight", "闪光灯测试"},
        {"RfFieldStrengthSet", "非接场强测试"},
        {"Vibrator", "振动器测试"},
        {"RF1", "NFC"},
        {"FunctionalBase", "功能底座测试"},
        {"Earpiece", "听筒测试"},
        {"LightSensor", "光感器测试"},
        {"ProximitySensor", "距离传感器测试"},
        {"GSensor", "重力传感器测试"},
        {"SARSensor", "SAR传感器测试"},
        {"TaxControl", "税控测试"},
        {"ButtonCell", "纽扣电池测试"},
        {"MCU", "MCU测试"},
        {"DockingStation", "扩展坞测试"},
        {"LightRF", "非接灯测试"},
        {"LightIC", "IC卡灯测试"},
        {"LowPower", "低功耗测试"},
        {"Repair", "维修测试"}
    };
    
    // 查找映射，如果找到返回描述，否则返回原始key
    auto it = moduleMap.find(moduleKey);
    if (it != moduleMap.end()) {
        return it->second;
    }
    return moduleKey;  // 如果找不到映射，返回原始key
}

struct ModuleInfo {
    std::string moduleName;
    int totalCount;
    int oneSuccessCount;
    double oneSuccessRate; // 使用 double 来存储成功率

    // 定义排序规则
    bool operator<(const ModuleInfo& other) const {
        return oneSuccessRate < other.oneSuccessRate; // 按照成功率升序排列
    }
};

string CIdentPub::TransformJson(const string& rawJson)
{
    // 创建新的 JSON 结构
    Document newDocument;
    newDocument.SetObject();
    Document::AllocatorType& allocator = newDocument.GetAllocator();

    // 设置 errorcode 和 errormessage
    newDocument.AddMember("errorcode", "0000", allocator);
    newDocument.AddMember("errormessage", "OK", allocator);

    // 创建 array
    Value array(kArrayType);
    set<string> uniqueModules; // 用于存储不同模块的集合

    // 如果原始 JSON 字符串为空，直接返回带空数组的 JSON
    if (rawJson.empty()) {
        newDocument.AddMember("array", array, allocator);
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        newDocument.Accept(writer);
        return buffer.GetString();
    }

    // 解析 JSON
    Document document;
    document.Parse(rawJson.c_str());

    // 获取 top_posnames 的 buckets
    if (document.HasMember("aggregations") && document["aggregations"].HasMember("top_posnames")) {
        const Value& buckets = document["aggregations"]["top_posnames"]["buckets"];
        
        for (SizeType i = 0; i < buckets.Size(); ++i) {
            const Value& bucket = buckets[i];

            // pos_name
            std::string posName = bucket["key"].GetString();

            // 获取该机型的总文档数
            int posTotalDocs = bucket.HasMember("doc_count") ? bucket["doc_count"].GetInt() : 0;
            // 计算1%的阈值
            double threshold = posTotalDocs * 0.01;

            // 创建模块数组
            std::vector<ModuleInfo> moduleInfos; // 存储模块信息
            const Value& modules = bucket["nested_funclist"]["by_func"]["buckets"];
            for (SizeType j = 0; j < modules.Size(); ++j) {
                const Value& module = modules[j];

                // 获取模块检查次数
                int moduleTotalCount = module["total_count"]["value"].GetInt();
                
                // 如果模块检查次数小于等于机型总文档数的1%，则剔除
                if (moduleTotalCount <= threshold) {
                    continue; // 跳过这个模块
                }

                // 模块信息
                ModuleInfo info;
                // 先获取标签（key），然后根据标签查找对应的描述
                string moduleKey = module["key"].GetString();
                info.moduleName = GetModuleDescription(moduleKey);  // 根据标签获取描述
                info.totalCount = module["total_count"]["value"].GetInt();
                info.oneSuccessCount = module["success_count"]["doc_count"].GetInt();
                info.oneSuccessRate = info.totalCount > 0 ? 
                    (info.oneSuccessCount * 100.0) / info.totalCount : 0.0;

                uniqueModules.insert(moduleKey); // 添加模块标签到集合中
                moduleInfos.push_back(info); // 将模块信息添加到 vector 中
            }

            // 对模块信息按 oneSuccessRate 排序
            std::sort(moduleInfos.begin(), moduleInfos.end());

            // 将排序后的模块信息添加到 moduleArray（每个机型最多返回6个模块）
            Value moduleArray(kArrayType);
            int moduleCount = 0;
            const int maxModulesPerPos = 6; // 每个机型最多返回6个模块
            for (const auto& moduleInfo : moduleInfos) {
                if (moduleCount >= maxModulesPerPos) {
                    break; // 达到最大数量，停止添加
                }
                Value moduleInfoJson(kObjectType);
                moduleInfoJson.AddMember("module_name", Value(moduleInfo.moduleName.c_str(), allocator), allocator);
                moduleInfoJson.AddMember("total_count", Value(std::to_string(moduleInfo.totalCount).c_str(), allocator), allocator);
                moduleInfoJson.AddMember("one_success_count", Value(std::to_string(moduleInfo.oneSuccessCount).c_str(), allocator), allocator);
                moduleInfoJson.AddMember("one_success_rate", Value(std::to_string(moduleInfo.oneSuccessRate).c_str(), allocator), allocator);
                
                // 将模块信息加入模块数组
                moduleArray.PushBack(moduleInfoJson, allocator);
                moduleCount++;
            }

            // 创建包含 pos_name 和模块数组的对象
            Value posInfo(kObjectType);
            posInfo.AddMember("pos_name", Value(posName.c_str(), allocator), allocator);
            posInfo.AddMember("module_arrary", moduleArray, allocator);

            // 将 posInfo 加入 array
            array.PushBack(posInfo, allocator);
        }
    }

    newDocument.AddMember("module_total", Value(std::to_string(uniqueModules.size()).c_str(), allocator), allocator);
    
    // 将 array 加入新的 JSON 结构
    newDocument.AddMember("array", array, allocator);

    // 将新 JSON 转换为字符串
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    newDocument.Accept(writer);

    return buffer.GetString();
}