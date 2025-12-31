#include "CGetFirmDownPath.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"
#include "xml/unicode.h"

#include <base/datetime.hpp>

using namespace aps;

int CGetFirmDownPath::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,tmpMap,outMap,ruleMap,tempMap;
	string uid;
    vector<CStr2Map> vectmapArray;

	// 取得请求参数
    pReqData->GetStrMap(inMap);
	
	//检查输入参数
	CheckParameter(inMap);

    //从配置文件获取文件服务器的地址和延时程序
    char szFileSerPath[256]={0};
    char szdelayApp[256]={0};
    char szAppConf[256]={0};


	memcpy(szFileSerPath,g_mTransactions[GetTid()].m_mVars["file_server_path"].c_str(),
                      g_mTransactions[GetTid()].m_mVars["file_server_path"].length());
    memcpy(szdelayApp,g_mTransactions[GetTid()].m_mVars["delay_app_path"].c_str(),
                      g_mTransactions[GetTid()].m_mVars["delay_app_path"].length());
    memcpy(szAppConf,g_mTransactions[GetTid()].m_mVars["app_conf_path"].c_str(),
                      g_mTransactions[GetTid()].m_mVars["app_conf_path"].length());

    //先用pack_id,去ADS判断是否走AP下载，如果不是走AP下载就返回GO列表
    

    if(inMap["channel_flag"] == "GO")//要求走GO
    {
        inMap["ap_down_flag"] = "2";//走GO
    }
    else //要求走AP或者不传,正常判断
    {
        if(!CIdentRelayApi::GetDownloadMethod(inMap,tempMap,true))
        {
            ErrorLog("判断下载方式失败");
            throw CTrsExp(ERR_SIGNATURE_INCORRECT,"判断下载方式失败");
        }
        inMap["ap_down_flag"] = tempMap["ap_down_flag"];
    }

    if(!CIdentRelayApi::GetFirmDownloadPath(inMap,ruleMap,true))
    {
        ErrorLog("获取固件下载规则失败");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"获取固件下载规则失败");
    }
    if(ruleMap["first_down"] == "YES")//说明首次下载
    {
        //设置延时程序去生成报告
        int nSpent = atoi(ruleMap["first_down"].c_str()) + 2;
        string strSetTime = Datetime::now().addHours(nSpent).format("h:i m/d/y");

        string programPath = szdelayApp;
        string parameters = szAppConf ;
        parameters += " " ;
        parameters += inMap["pack_id"];

        //构建 at 命令
        string command = "echo '" + programPath + " " + parameters + "' | at " + strSetTime;

        int result = system(command.c_str());
        if (result == 0)
        {
            InfoLog("定时任务已设置，程序将在 %d 小时后执行",nSpent);
		    CIdentPub::SendAlarm2("定时任务已设置，程序将在 %d 小时后执行",nSpent);
        }
        else
        {
            CIdentPub::SendAlarm2("设置定时任务失败[%s]",command.c_str());
        }
    }

    //leve 优先级，当多次GO连接不上可传1
    if(inMap["leve"] == "1")//多次连接不上
    {
        if(tempMap["go_leve_flag"]=="1")//还可以走AP
            tempMap["ap_down_flag"]=="1";//改成走AP
    }

    if(inMap["is_hand"] == "1")//说明是手动下载，只能走AP
        tempMap["ap_down_flag"]=="1";//改成走AP

    if(tempMap["ap_down_flag"]=="2") //GO热点下载
    {
        //直接去GO热点列表批量查询
        inMap["offset"] = "0";
        inMap["limit"]  = "30"; //一次查询30笔
    
        CIdentRelayApi::QueryGODownList(inMap,outMap,vectmapArray,true);
        pResData->SetPara("ap_down_flag",tempMap["ap_down_flag"]);
        pResData->SetPara("trans_id",ruleMap["trans_id"]);
        pResData->SetPara("ret_num",outMap["ret_num"]);
        pResData->SetPara("total",outMap["total"]);

        for(size_t i = 0;i < vectmapArray.size();++i)
        {
            CStr2Map returnMap,tmpMap;
            CIdentComm::DelMapF(vectmapArray[i],returnMap);
            tmpMap["go_name"]    = returnMap["go_name"];
            tmpMap["go_passwd"]  = returnMap["go_passwd"];
            pResData->SetArray("go_array",tmpMap);
        }
        return 0;
    }

    //直接去固件信息表批量查询，最后再去生成日志记录tran_id
    inMap["offset"] = "0";
    inMap["limit"]  = "10"; //一次查询10笔
	inMap["state"] = "1";  //生效

    CIdentRelayApi::QueryFirmInfoList(inMap,outMap,vectmapArray,true);
    pResData->SetPara("ap_down_flag",inMap["ap_down_flag"]);
    pResData->SetPara("trans_id",ruleMap["trans_id"]);
    pResData->SetPara("ret_num",outMap["ret_num"]);
    pResData->SetPara("total",outMap["total"]);

    for(size_t i = 0;i < vectmapArray.size();++i)
    {
        CStr2Map returnMap,tmpMap;
        CIdentComm::DelMapF(vectmapArray[i],returnMap);
        tmpMap["pack_path"]  = returnMap["pack_path"];
        tmpMap["pack_name"]  = returnMap["pack_name"];
        tmpMap["pack_size"]  = returnMap["pack_size"];
        tmpMap["id"]         = returnMap["id"];
        pResData->SetArray("ap_array",tmpMap);
    }
    
	return 0;
}

// 输入判断
void CGetFirmDownPath::CheckParameter( CStr2Map& inMap)
{
	if(inMap["pack_id"].empty())
    {
        ErrorLog("pack_id缺少");
		CIdentPub::SendAlarm2("pack_id缺少[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"pack_id缺少");
    }
    if(inMap["order_id"].empty()&&inMap["term_type"].empty()&&inMap["mater_num"].empty()&&inMap["pid"].empty())
    {
        ErrorLog("必须填写一个元素");
		CIdentPub::SendAlarm2("APP获取下载地址必须填写一个元素[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"APP获取下载地址必须填写一个元素");
    }
}


  