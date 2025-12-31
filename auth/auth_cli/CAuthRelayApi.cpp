#include "CAuthRelayApi.h"
#include "tools/CTrsExp.h"
#include "tools/RuntimeGather.h"
#include "tools/tinyxml.h"
#include "tools/xmlHelper.h"
#include "tools/CTools.h"
#include "relaycomm/CEnRelayCall.h"
#include "tools/transxmlcfg.h"
#include "relaycomm/clienterr.h"

CRelayCall* GetAuthSingleInstance(int objtype)
{
	static CRelayCall  *pRelayCall 	= NULL ;
	static CRelayCall  *pEnRelayCall 	= NULL ;

	if(objtype == 1 && pRelayCall == NULL)
	{
		pRelayCall = new CRelayCall() ; 
	}

	if(objtype == 2 && pEnRelayCall == NULL)
	{
		pEnRelayCall = new CEnRelayCall() ; 
	}

	if(objtype == 1)
	{
		return pRelayCall ;
	}
	if(objtype == 2)
	{
		return pEnRelayCall ;
	}

	return NULL ;
}

CRelayCall* CAuthRelayApi::GetRelayObj(int request_type,int objtype)
{
	string svrip	= "";
	int    iPort	= 22000 ;
	int    iTimeOut= 2 ;
	string spid = g_allVar.GetValue("default_spid") ;
	string ver =  g_allVar.GetValue("default_spid","ver") ;

	string reqtype = Tools::IntToStr(request_type) ;
	g_serverConf.GetSvrConf(reqtype,svrip,iPort,iTimeOut) ;
	if(svrip == "")
	{
		g_RuntimeGather.SaveLog(WARN,"[%s,%d]访问relay配置失败",__FILE__,__LINE__);
		throw(CTrsExp(CONFIG_RELAY_ERR,"系统配置失败，请联系管理员")) ;
	}
	CRelayCall *pRelayCall = GetAuthSingleInstance(objtype);

	if(pRelayCall == NULL)
	{
		g_RuntimeGather.SaveLog(WARN,"[%s,%d]创建relay对像失败",__FILE__,__LINE__);
		throw(CTrsExp(ALLCA_MEM_ERR,"系统繁忙,请稍后再试")) ;
	}
	pRelayCall->Init(svrip.c_str(),iPort,spid.c_str(),iTimeOut,ver) ;
	g_RuntimeGather.SaveLog(INFO,"%s|%d",svrip.c_str(),iPort) ;
	return pRelayCall ;
}

CRelayCall* CAuthRelayApi::GetRelayObj(const string & reqtype,int objtype)
{
	string svrip	= "";
	int    iPort	= 22000 ;
	int    iTimeOut= 2 ;
	string spid = g_allVar.GetValue("default_spid") ;
	string ver =  g_allVar.GetValue("default_spid","ver") ;


	g_serverConf.GetSvrConf(reqtype,svrip,iPort,iTimeOut) ;
	if(svrip == "")
	{
		g_RuntimeGather.SaveLog(WARN,"[%s,%d]访问relay配置失败",__FILE__,__LINE__);
		throw(CTrsExp(CONFIG_RELAY_ERR,"系统配置失败，请联系管理员")) ;
	}
	CRelayCall *pRelayCall = GetAuthSingleInstance(objtype);

	if(pRelayCall == NULL)
	{
		g_RuntimeGather.SaveLog(WARN,"[%s,%d]创建relay对像失败",__FILE__,__LINE__);
		throw(CTrsExp(ALLCA_MEM_ERR,"系统繁忙,请稍后再试")) ;
	}
	pRelayCall->Init(svrip.c_str(),iPort,spid.c_str(),iTimeOut,ver) ;
	g_RuntimeGather.SaveLog(INFO,"%s|%d",svrip.c_str(),iPort) ;
	return pRelayCall ;
}

bool CAuthRelayApi::ParseResult(bool bCallRes,CRelayCall* m_pRelayCall,CStr2Map & resultMap,bool throwexp,bool xmlPrase)
{
	if(!bCallRes)
	{
		g_RuntimeGather.SaveLog(WARN,"[%s,%d]error info %s",__FILE__,__LINE__,m_pRelayCall->GetLastError().c_str());
		if(throwexp)
		{
			throw(CTrsExp(RELAY_SYS_ERR,"系统繁忙,请稍后再试"));
		}
		else
		{
			resultMap["errorcode"] = RELAY_SYS_ERR ;
			resultMap["errormessage"] = "系统繁忙" ;
			return false ;
		}

	}
	//解析
	TiXmlDocument doc;
	doc.Parse(string(m_pRelayCall->getResultStr()).c_str());
	if (doc.Error())
	{
		g_RuntimeGather.SaveLog(WARN,"[%s %d]解析XML结果失败!", __FILE__, __LINE__ );
		if(throwexp)
		{
			throw(CTrsExp(PARSE_XML_ERR,"查询结果格式错误,请稍后再试")) ;
		}
		return false ;
	}
	XmlNodeWrapper xn(doc.RootElement());
	resultMap["errorcode"] = xn.getValue("errorcode",RELAY_SYS_ERR);
	resultMap["errormessage"] = xn.getValue("errormessage","系统繁忙");

	if(resultMap["errorcode"] != "0000")
	{
		//转码
		char buff[1024*2] = {0} ;
		bzero(buff,sizeof(buff)) ;
		int iDestLen = sizeof(buff)-1;
    	//Tools::ConvertCharSet((char*)(resultMap["errormessage"].c_str()),buff,iDestLen,"GBK","utf-8"); //不需要转码了
		//resultMap["errormessage"] = buff;
		if(throwexp)
		{
			throw(CTrsExp(resultMap["errorcode"],resultMap["errormessage"])) ;
		}
		return false ;
	}

	if(xmlPrase)
	{

		int ret = xn.getValue("/root/record", -1);
		if(ret != 0)
		{
			if(throwexp)
			{
				g_RuntimeGather.SaveLog(WARN,"[%s,%d]返回XML结果集出错",__FILE__,__LINE__) ;
				throw(CTrsExp(XML_FORMAT_ERR,"查询结果格式错误,请稍后再试")) ;
			}
			return false ;
		}
	}
	return true ;
}
void CAuthRelayApi::CheckParams(const char ** pParams,CStr2Map & paramMap,const int& iRequestType, bool bStrongCheck)
{
	for(int iIndex=0; pParams[iIndex]!= NULL;iIndex++)
	{
		if(!paramMap.count(pParams[iIndex]))
		{
			WarnLog("%d 接口调用缺少参数:%s",iRequestType,pParams[iIndex]);
			throw(CTrsExp(LACK_OF_PARAM,"参数错误")) ;
		}
	}
	//bStrongCheck为true需校验参数是否为空值
	if(bStrongCheck)
	{
		for(int iIndex=0; pParams[iIndex]!= NULL;iIndex++)
		{
			if(paramMap[pParams[iIndex]].empty())
			{
				WarnLog("%d 接口调用参数:%s为空值",iRequestType,pParams[iIndex]);
				throw(CTrsExp(LACK_OF_PARAM,"参数错误")) ;
			}
		}
	}
}
