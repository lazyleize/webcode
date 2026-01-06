/*
 * CAuthPub.cpp
 *
 *  Authated on: 2013-3-11
 *      Author: JiaYeHui
 */


#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "CAuthPub.h"
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/aes.h>
#include "xml/unicode.h"
#include "curl/curl.h"


using namespace rapidjson;
using namespace std;

#define STATE_16 "16"
/*------------------------------------------------------------------------
 *Function Name: GetDivDay
 *       Desc: 根据起始日、间隔天数、方向计算结束日
 *      Input: 日期串(YYYY-MM-DD)
 *     Return: 结束日串(YYYY-MM-DD)
 *-----------------------------------------------------------------------*/
string CAuthPub::GetDivDay(const char * start_date, int nday)
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
string CAuthPub::GetXyyDay(const char * start_date, int nday)
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
int CAuthPub::GetTimeNow(char *str)
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

string CAuthPub::GetCurrentTime()
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

string CAuthPub::GetCurrentDateTime()
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

//取当前日期格式为YYYY-MM-DD
string CAuthPub::GetDateNow()
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
string CAuthPub::GetFormatDateNow()
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
void CAuthPub::GetTime(string & strTime)
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
string CAuthPub::GetDivHourTime(const string& sStartTime, int nHour)
{
        struct tm m1;
        memset(&m1,0x0,sizeof(m1));
        time_t t1;
        /*
         *将日期转换成以秒为单位的整型时间
         */
        m1.tm_year = atoi(sStartTime.substr(0,4).c_str())- 1900;
        m1.tm_mon = atoi(sStartTime.substr(5,2).c_str())- 1;
        m1.tm_mday = atoi(sStartTime.substr(8,2).c_str());
        m1.tm_hour= atoi(sStartTime.substr(11,2).c_str()) + nHour;
        m1.tm_min= atoi(sStartTime.substr(14,2).c_str());
        m1.tm_sec= atoi(sStartTime.substr(17,2).c_str());

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
string CAuthPub::TranslateDate(const string& srcDate, const int flag)
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

int CAuthPub::codeConvert(const char* src_page, const char* dst_page,
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
int CAuthPub::ChangeCharacterSize(string &strBuf, const char * szFrom,
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
	if (CAuthPub::codeConvert(szFrom, szTo, strBuf.c_str(),
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
string CAuthPub::ChangeContractState(const string &state, int strno)
{
	//借款人发起查询
	if (strno == CAuthPub::APPLY_NO_PUB)
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
	if (strno == CAuthPub::ASSURE_APPLY_NO_PUB)
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
	if (strno == CAuthPub::ASSURE_NO_PUB)
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
	if (strno == CAuthPub::LOAN_NO_PUB)
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
string CAuthPub::GetAuthDesKey()
{
	//string sDesKey("42B43D098214ED23984D38D9E64668D5");
	string sDesKey("1234567887654321");

	return sDesKey;
}
/*获取配置操作部门和操作员，并进行非空判断,失败抛出异常*/
void CAuthPub::GetDeptAndTeller(string& dept, string& teller)
{
	dept = g_allVar.GetValue("auth_oper", "dept");
	teller = g_allVar.GetValue("auth_oper", "teller");

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
string CAuthPub::undes3(const string& src_str)
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
	string sDesKey = CAuthPub::GetAuthDesKey();
    strcpy(key,sDesKey.c_str());

    memset(outstr,0x00,sizeof(outstr));
    ads_3des_string("D",(unsigned char*)key,srcbuf,(src_str.length())/2,outstr);
	ErrorLog("[%s],[%d]  outstr:[%d] [%s]",__FILE__,__LINE__,(src_str.length())/2,outstr);
	return outstr;
}

string CAuthPub::undes3_mobile(const string& src_str)
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
    ads_3des_string("D",(unsigned char*)key,srcbuf,(src_str.length())/2,outstr);
	ErrorLog("[%s],[%d]  outstr:[%d] [%s]",__FILE__,__LINE__,(src_str.length())/2,outstr);
	return outstr;
}

string CAuthPub::des3(const string& src_str)
{
	if (src_str.empty())
	{
		return string("");
	}

	//加密后的字符串
	string des_str;

	//获取3DES密钥，获取失败抛出异常 
	string sDesKey = CAuthPub::GetAuthDesKey();

	return des_str;
}
string CAuthPub::GetDateAfterXYear(const string& start_date, int nYear)
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
string CAuthPub::BankIdMask(const string& bank_id)
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
string CAuthPub::GetTransConfigNotEmpty(const string & tid,
                const string & attrName)
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
string CAuthPub::GetVarConfigNotEmpty(const string & name,
                const string & attrName)
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
int CAuthPub::OpenFile(const string & filename, ofstream & os,
		const ios_base::openmode openmode) 
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
*   Auth: JiaYeHui                                                      *
*   Date: 2015-04-09                                                    *
************************************************************************/
int CAuthPub::ads_des_idstr_see(char *option,int id,char *instr,int instrlen,char *outstr)
{
    int ilen;
    char tmpstr[1025];

    memset(tmpstr,0x00,sizeof(tmpstr));
    if ((*option) == 'E')
    {
        if(instrlen > 512)
            return -1;
        ilen=ads_des_idstr("E",id,instr,instrlen,outstr);
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
        ilen=ads_des_idstr("D",id,tmpstr,ilen,outstr);
    }
    return ilen;
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
int CAuthPub::ads_creid_check( char *creid)
{
     int result=0;
     int i;
     int flag;
     char l;
     int xishu[17] = {7,9,10,5,8,4,2,1,6,3,7,9,10,5,8,4,2};
     char last[12] = {"10X98765432"};

     for(i = 0;i < 17;i++)
     {
         result += (creid[i]-48)*xishu[i];
     }

     flag = result % 11;
     l = last[flag];

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
/*------------------------------------------------------------------------
 *Function Name:CheckTransTime 
 *       Desc: 
 *             交易时间误差在300秒内
 *      Input: YYYYMMDDHHMMSS
 *       Auth: JiaYeHui
 *       Date: 2018-03-30                                                
 *-----------------------------------------------------------------------*/
void CAuthPub::CheckTransTime(string &trans_time,const int num_sec)
{
    char now_time[20];
    int  div_sec;
    memset(now_time,0x00,sizeof(now_time));
    ads_get_date_time((char *)"%y%m%d%h%n%s",now_time);
    div_sec=ads_diff_sec(now_time,(char *)(trans_time.c_str()));
    /*if(abs(div_sec) > num_sec)
    {
        ErrorLog("交易失败55 发送时间[%s]-接收时间[%s] 间隔[%d] 只能[%d]",trans_time.c_str(),now_time,div_sec,num_sec);
        throw CTrsExp(ERR_TRANS_TIME,"交易失败55");
    }*/
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
string CAuthPub::WebrsaToAdsdes(int n_web,string &web_pwd,int n_ads,const int num_sec)
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
    int dec_len=CRsaHandler::private_decrypt((unsigned char *)ascbuf,len,(unsigned char *)rsa_key.c_str(),(unsigned char *)rsabuf);
    if(dec_len == -1)
    {
        ErrorLog("系统维护中-解密失败-n_web=[%d]-n_ads=[%d]-sec=[%d]",n_web,n_ads,num_sec);
        throw CTrsExp(ERR_WRITE_SESSION,"系统维护中");
        //return(-1);
    }

    //解密内容14位时间+密码
    if(dec_len < 20)
    {
        ErrorLog("系统维护中-RSA解密后长度<20,dec_len=[%d]-buf=[%s]",dec_len,rsabuf);
        throw CTrsExp(ERR_WRITE_SESSION,"系统维护中");
        //return(-1);
    }
    if(dec_len > 60)
    {
        ErrorLog("系统维护中-RSA解密后长度>60,dec_len=[%d]-buf=[%s]",dec_len,rsabuf);
        //return(-1);
        throw CTrsExp(ERR_WRITE_SESSION,"系统维护中");
    }
    if(n_ads == 1) //交易密码是6位数字
    {
        if(dec_len != 20)
        {
            ErrorLog("系统维护中-RSA解密后长度不等于20,dec_len=[%d]-buf=[%s]",dec_len,rsabuf);
            throw CTrsExp(ERR_WRITE_SESSION,"系统维护中");
            //return(-1);
        }
    }

    //时间验证 20秒有效期
    memset(des_open,0x00,sizeof(des_open));
    memcpy(des_open,rsabuf,14);
    InfoLog("kkkkkkk-rsabuf=[%s] des_open=[%s]",rsabuf,des_open);
    string tm=des_open;
    CAuthPub::CheckTransTime(tm,num_sec);
    memset(des_open,0x00,sizeof(des_open));
    strcpy(des_open,rsabuf+14);
    InfoLog("kkkkkkk-rsabuf=[%s] des_open=[%s]",rsabuf,des_open);

    CAuthPub::ads_des_idstr_see("E",n_ads,des_open,strlen(des_open),des_enc);

    return string(des_enc);
}


/*------------------------------------------------------------------------
 *Function Name: parsePubRespJson
 *       Desc: 
 *        将":" json串转成map 单层输出
 *      Input: 
 *        待转码串 {"respDesc":"订单号重复","returnUrl":"http://www.jfbill.com/cgi-bin/cdh_in_amount.cgi"}
 *        转码结果Map
 *     Return: -1失败 0成功
 *       Auth: 
 *       Date: 2018-01-28                                                   
 *-----------------------------------------------------------------------*/

int CAuthPub::parsePubRespJson(string &data, CStr2Map& outMap)
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

char* CAuthPub::solve(char* dest,const char*src)
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