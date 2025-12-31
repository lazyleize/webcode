#include "CQueryOverallList.h"
#include "CIdentRelayApi.h"


int CQueryOverallList::IdentCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map sessMap;
	CStr2Map inMap, outMap,tmpInMap,tmpOutMap;
	pReqData->GetStrMap(inMap);

	this->CheckLogin(sessMap);

	int limit = atol(inMap["limit"].c_str());
	int offset = atol(inMap["offset"].c_str());

	offset = (offset -1)*limit;
    inMap["limit"] = Tools::IntToStr(limit);
	inMap["offset"] = Tools::IntToStr(offset);

	/*inMap["usertype"] = sessMap["usertype"];
    int usertype = atoi(sessMap["usertype"].c_str());
    if(usertype == 2 || usertype == 1)
        inMap["factory_id"].clear();
	else
	    inMap["factory_id"] = sessMap["factoryid"];*/

    //根据ID单笔查询
    CStr2Map termProInMap,termProOutMap;
    termProInMap["id"] = inMap["id"];
    CIdentRelayApi::QueryWorkNode(termProInMap,termProOutMap,true);


    if(termProOutMap["Fpos_sn"].empty())
    {
        inMap["sn"] = termProOutMap["Fass_pid"];
    }
    else
    {
        inMap["sn"] = termProOutMap["Fpos_sn"];
    }
    string term_name=termProOutMap["Fterm_name"];

    termProInMap.clear();
    termProOutMap.clear();
    termProInMap["term_name"] = term_name;
    CIdentRelayApi::QueryTermWorkNode(termProInMap,termProOutMap,true);
    
    int nCurryNum = 0;//当前返回的条数
    string strPidNum,strSnNum ;
    vector<CStr2Map> vectmapArray;
    //区分是PID还是SN号
    bool IsPID = CIdentPub::IsPIDNumber(inMap["sn"]);
    InfoLog("sn=[%s]",inMap["sn"].c_str());
    if(IsPID)
    {
        //去生产数据采集列表批量查询--保存一下机型
        tmpInMap.clear();
        vectmapArray.clear();
        tmpInMap["pos_sn"] = inMap["sn"];
        tmpInMap["limit"]  = "50";
        tmpInMap["offset"] = "0";
        CIdentRelayApi::QueryConfPidCollectList(tmpInMap,tmpOutMap,vectmapArray,true);
        nCurryNum += atoi(tmpOutMap["ret_num"].c_str());
        for(size_t i = 0;i < vectmapArray.size();++i)
        {
            CStr2Map returnMap;
            returnMap["node_id"]    = vectmapArray[i]["Fid"];
            returnMap["order"]      = vectmapArray[i]["Forder_id"];
            returnMap["ass_pid"]    = vectmapArray[i]["Fass_pid"];
            returnMap["result"]     = vectmapArray[i]["Fresult"] == "Success"? "0":"1";
            returnMap["type"]       = vectmapArray[i]["Ftype"];
            returnMap["pos_sn"]     = vectmapArray[i]["Fpossn"];
            returnMap["create_time"]= vectmapArray[i]["Fcreate_time"];
            returnMap["subject"]    = "0";
            pResData->SetArray(returnMap);
            strSnNum = returnMap["pos_sn"];
        }

        //获取工厂生产记录信息
        tmpOutMap.clear();
        vectmapArray.clear();
        CIdentRelayApi::QueryProducePIDRecordListNoToal(tmpInMap,tmpOutMap,vectmapArray,true); 
        nCurryNum += atoi(tmpOutMap["ret_num"].c_str());
        for(size_t i = 0;i < vectmapArray.size();++i)
        {
            CStr2Map returnMap;
            returnMap["node_id"]      = vectmapArray[i]["Flog_id"];
            returnMap["order"]        = vectmapArray[i]["Forder"];
            returnMap["ass_pid"]      = vectmapArray[i]["Fass_pid"];
            returnMap["result"]       = vectmapArray[i]["Fresult"];
            returnMap["type"]         = GetStageType(vectmapArray[i]["Flog_type"], termProOutMap);
            returnMap["pos_sn"]       = vectmapArray[i]["Fpos_sn"];
            returnMap["create_time"]  = vectmapArray[i]["Fcreate_time"];
            returnMap["subject"]      = "1";
            pResData->SetArray(returnMap);
            
        }

        //获取自动化下载记录信息
        /*tmpInMap.clear();
        tmpOutMap.clear();
        vectmapArray.clear();
        tmpInMap["pid"] = inMap["sn"];
        tmpInMap["limit"] = "10";
        tmpInMap["offset"] = "0";
        CIdentRelayApi::QueryFirmDownList(tmpInMap,tmpOutMap,vectmapArray,true); 
        nCurryNum += atoi(tmpOutMap["ret_num"].c_str());
        for(size_t i = 0;i < vectmapArray.size();++i)
        {
            CStr2Map returnMap;
            returnMap["node_id"]      = vectmapArray[i]["Ftask_id"];
            returnMap["order"]      = vectmapArray[i]["Forder_id"];
            returnMap["ass_pid"]    = vectmapArray[i]["Fpid"];
            if(vectmapArray[i]["Fresult"] == "2" || vectmapArray[i]["Fresult"] == "3")
                returnMap["result"]     = "0";//成功
            else
                returnMap["result"]     = "1";
            returnMap["type"]       = "自动化下载";
            returnMap["pos_sn"]     = "";//没有
            returnMap["create_time"]= vectmapArray[i]["Fbegin_time"];
            returnMap["subject"]    = "2";
            pResData->SetArray(returnMap);
        }*/

        //终端运行日志查询
        tmpInMap.clear();
        tmpOutMap.clear();
        vectmapArray.clear();
        tmpInMap["pid"] = inMap["sn"];
        tmpInMap["limit"] = "50";
        tmpInMap["offset"] = "0";
        CIdentRelayApi::QueryTermRunList(tmpInMap,tmpOutMap,vectmapArray,true); 
        nCurryNum += atoi(tmpOutMap["ret_num"].c_str());
        for(size_t i = 0;i < vectmapArray.size();++i)
        {
            CStr2Map returnMap;
            returnMap["node_id"]      = vectmapArray[i]["Fterm_log_id"];
            returnMap["order"]        = vectmapArray[i]["Fterm_log_id"];
            returnMap["ass_pid"]      = vectmapArray[i]["Fpid"];
            returnMap["result"]       = "0";//成功
            returnMap["type"]         = "终端运行日志";
            returnMap["pos_sn"]       = vectmapArray[i]["Fpossn"];;//没有
            returnMap["create_time"]  = vectmapArray[i]["Fbegin_time"];
            returnMap["subject"]      = "2";
            pResData->SetArray(returnMap);
        }



    }
    else //是SN号
    {
        //去生产数据采集列表批量查询--保存一下机型
        tmpInMap.clear();
        vectmapArray.clear();
        tmpInMap["pos_sn"] = inMap["sn"];
        tmpInMap["limit"] = "50";
        tmpInMap["offset"] = "0";
        CIdentRelayApi::QueryCollectList(tmpInMap,tmpOutMap,vectmapArray,true);
        nCurryNum += atoi(tmpOutMap["ret_num"].c_str());
        for(size_t i = 0;i < vectmapArray.size();++i)
        {
            CStr2Map returnMap;
            returnMap["node_id"]    = vectmapArray[i]["Fid"];
            returnMap["order"]      = vectmapArray[i]["Forder_id"];
            returnMap["ass_pid"]    = vectmapArray[i]["Fass_pid"];
            returnMap["result"]     = vectmapArray[i]["Fresult"] == "Success"? "0":"1";
            returnMap["type"]       = vectmapArray[i]["Ftype"];
            returnMap["pos_sn"]     = vectmapArray[i]["Fpossn"];
            returnMap["create_time"]= vectmapArray[i]["Fcreate_time"];
            returnMap["subject"]    = "0";
            pResData->SetArray(returnMap);
            strPidNum = returnMap["ass_pid"];
        }


        //获取工厂生产记录信息
        tmpOutMap.clear();
        vectmapArray.clear();
        CIdentRelayApi::QueryProduceRecordListNoToal(tmpInMap,tmpOutMap,vectmapArray,true); 
        nCurryNum += atoi(tmpOutMap["ret_num"].c_str());
        for(size_t i = 0;i < vectmapArray.size();++i)
        {
            CStr2Map returnMap;
            returnMap["node_id"]    = vectmapArray[i]["Flog_id"];
            returnMap["order"]      = vectmapArray[i]["Forder"];
            returnMap["ass_pid"]    = vectmapArray[i]["Fass_pid"];
            returnMap["result"]     = vectmapArray[i]["Fresult"];
            returnMap["type"]       = GetStageType(vectmapArray[i]["Flog_type"], termProOutMap);
            returnMap["pos_sn"]     = vectmapArray[i]["Fpos_sn"];
            returnMap["create_time"]= vectmapArray[i]["Fcreate_time"];
            returnMap["subject"]    = "1";
            pResData->SetArray(returnMap);
            
        }


        //获取自动化下载记录信息
        /*tmpInMap.clear();
        tmpOutMap.clear();
        vectmapArray.clear();
        tmpInMap["pid"] = strPidNum;
        tmpInMap["limit"] = "10";
        tmpInMap["offset"] = "0";
        CIdentRelayApi::QueryFirmDownList(tmpInMap,tmpOutMap,vectmapArray,true); 
        nCurryNum += atoi(tmpOutMap["ret_num"].c_str());
        for(size_t i = 0;i < vectmapArray.size();++i)
        {
            CStr2Map returnMap;
            returnMap["node_id"]      = vectmapArray[i]["Ftask_id"];
            returnMap["order"]      = vectmapArray[i]["Forder_id"];
            returnMap["ass_pid"]    = vectmapArray[i]["Fpid"];
            if(vectmapArray[i]["Fresult"] == "2" || vectmapArray[i]["Fresult"] == "3")
                returnMap["result"]     = "0";//成功
            else
                returnMap["result"]     = "1";
            returnMap["type"]       = "自动化下载";
            returnMap["pos_sn"]     = "";//没有
            returnMap["create_time"]= vectmapArray[i]["Fbegin_time"];
            returnMap["subject"]    = "2";
            pResData->SetArray(returnMap);
        }   */

        //终端运行日志查询
        tmpInMap.clear();
        tmpOutMap.clear();
        vectmapArray.clear();
        tmpInMap["possn"] = inMap["sn"];
        tmpInMap["limit"] = "50";
        tmpInMap["offset"] = "0";
        CIdentRelayApi::QueryTermRunList(tmpInMap,tmpOutMap,vectmapArray,true); 
        nCurryNum += atoi(tmpOutMap["ret_num"].c_str());
        for(size_t i = 0;i < vectmapArray.size();++i)
        {
            CStr2Map returnMap;
            returnMap["node_id"]      = vectmapArray[i]["Fterm_log_id"];
            returnMap["order"]        = vectmapArray[i]["Fterm_log_id"];
            returnMap["ass_pid"]      = vectmapArray[i]["Fpid"];
            returnMap["result"]       = "0";//成功
            returnMap["type"]         = "终端运行日志";
            returnMap["pos_sn"]       = vectmapArray[i]["Fpossn"];;//没有
            returnMap["create_time"]  = vectmapArray[i]["Fbegin_time"];
            returnMap["subject"]      = "2";
            pResData->SetArray(returnMap);
        }
    }

    //汇总返回
    pResData->SetPara("ret_num",to_string(nCurryNum));
    return 0;
 
}

void CQueryOverallList::CheckParameter(CStr2Map& inMap)
{
    if(inMap["sn"].empty())
    {
        ErrorLog("关键字段不能为空-sn");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段sn为空");
    }
}

//0：Recovery下防切  1：下机身号 2：统一协议组装 3：产品校验流程和N80解触发流程  4：统一协议维修流程 5:华勤成品机软件配置下载
string CQueryOverallList::GetStageType(string& strtype, CStr2Map& termProOutMap)
{
    if (strtype == "0")
    {
        return "防切机下载(存量)";
    }
    else if(strtype == "1")
    {
        // 判断termProOutMap["Fpack_node"]是否包含"机身号下载"
        string packNode = termProOutMap["Fpack_node"];
        if(packNode.find("机身号下载") != string::npos)
        {
            return "机身号下载";
        }
        else
        {
            return "防切机下载(新型)";
        }
    }
    else if(strtype == "2")
    {
        return "终端配置下载";
    }
    else if(strtype == "3")
    {
        // 判断termProOutMap["Fpack_node"]是否包含"产品校验"
        string packNode = termProOutMap["Fpack_node"];
        if(packNode.find("产品校验") != string::npos)
        {
            return "产品校验";
        }
        else
        {
            return "终端信息核对";
        }
    }
    else if(strtype == "4")
    {
        return "维修流程";
    }
    
    return "";
}