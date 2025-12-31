#include "CIdentAppTools.h"
#include "CIdentRelayApi.h"
#include "CAuthRelayApi.h"
#include "ident_err.h"
#include "transxmlcfg.h"
#include "RuntimeGather.h"
#include "CTools.h"
#include "CIdentPub.h"

void CIdentAppTools::DelMapF(CStr2Map& dataMap,CStr2Map& outMap)
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

string CIdentAppTools::GetTransConfigNotEmpty(const string & tid,const string & attrName)
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

//取当前日期格式为YYYYMMDD
string CIdentAppTools::GetFormatDateNow()
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

