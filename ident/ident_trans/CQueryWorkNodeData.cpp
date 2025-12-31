#include "CQueryWorkNodeData.h"
#include "CIdentRelayApi.h"

#include <base/strHelper.hpp>
using namespace aps;

int CQueryWorkNodeData::IdentCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map sessMap;
	CStr2Map inMap,workMap, outMap,resultMap;

    string strPostdata=pReqData->GetPostData();
	CIdentPub::parsePubRespJson(strPostdata,inMap);
    this->CheckLogin(sessMap);

    CheckParameter(inMap);
    
    //根据ID查询工厂生产概况表，获取到
    CIdentRelayApi::QueryWorkNode(inMap,workMap,true);
    
    //初始化最终结果状态为进行中
    string fresult = "IN"; // 进行中：未走包装且没报错
    
    vector<CStr2Map> vectmapArray;
    //开始贴片段的数据---根据PID去t_collect_of_result查询Ftype是SMT的最新记录
    vector<string> tmpVec;
    StrHelper::split(inMap["smt_array"],',',tmpVec);
    for(int i=0;i<tmpVec.size();i++)
    {
        CStr2Map coInMap,coOutMap;
        CStr2Map returnMap;
        vectmapArray.clear();
        if(tmpVec[i]=="贴片功能测试" && workMap["Fsmt_result"] != "N")
        {
            if(!workMap["Fass_pid"].empty())//一定要PID
            {
                coInMap["ass_pid"]  = workMap["Fass_pid"];
                coInMap["type"] = "SMT";
                coInMap["limit"] = "1";
                coInMap["offset"] = "0";
                CIdentRelayApi::QueryCollectList(coInMap,coOutMap,vectmapArray,true);//只会有1条结果
                if(vectmapArray.size()==0)
                    continue;
            
                CStr2Map tmpResMap;
                CIdentComm::DelMapF(vectmapArray[0],tmpResMap);
		    
                returnMap["node_id"]       = tmpResMap["id"];
                //returnMap["node_result"]   = tmpResMap["result"];
                returnMap["node_name"]     = tmpVec[i];
                returnMap["node_time"]     = tmpResMap["create_time"];
                returnMap["node_message"]  = "";

                if(tmpResMap["result"] == "Success")
                    returnMap["node_state"]    = "0";
                else
                {
                    returnMap["node_state"]    = "1";
                    fresult = "F"; // 贴片段失败，设置最终结果为失败
                }

                pResData->SetArray("smt_arrary",returnMap);

            }
            
        }   
    }

    //开始组装段的数据---根据PID去t_log_record查询类型Flog_type是2的最新记录
    tmpVec.clear();
    StrHelper::split(inMap["fac_array"],',',tmpVec);
    for(int i=0;i<tmpVec.size();i++)
    {
        InfoLog("tmpVec=[%s]",tmpVec[i].c_str());
        CStr2Map facInMap,facOutMap;
        CStr2Map returnMap;
        vectmapArray.clear();
        if(tmpVec[i]=="终端配置下载" && workMap["Fac_result"] != "N")
        {
            if(!workMap["Fass_pid"].empty())
            {
                facInMap["pos_sn"]     = workMap["Fass_pid"];
                facInMap["log_type"]   = "2";//组装
                facInMap["limit"]      = "1";
                facInMap["offset"]     = "0";
                CIdentRelayApi::QueryProducePIDRecordListNoToal(facInMap,facOutMap,vectmapArray,true);
                if(vectmapArray.size()==0)
                    continue;
                returnMap["node_state"]  = vectmapArray[0]["Fresult"];
                vectmapArray[0]["Fid"] = vectmapArray[0]["Flog_id"];;
                if(returnMap["node_state"] == "0")
                {
                    returnMap["node_result"]  = "成功";
                    returnMap["node_message"]  = "";
                }
                else
                {
                    returnMap["node_result"]  = "失败";
                    returnMap["node_message"]  = vectmapArray[0]["Fmessage"];
                }
            }
            else
            {
                if(!workMap["Fpos_sn"].empty())
                {
                    facInMap["pos_sn"]     = workMap["Fpos_sn"];
                    facInMap["log_type"]   = "2";//组装
                    facInMap["limit"]      = "1";
                    facInMap["offset"]     = "0";
                    CIdentRelayApi::QueryProduceRecordListNoToal(facInMap,facOutMap,vectmapArray,true);
                    if(vectmapArray.size()==0)
                        continue;
                    returnMap["node_state"]  = vectmapArray[0]["Fresult"];
                    vectmapArray[0]["Fid"]   = vectmapArray[0]["Flog_id"];;
                    if(returnMap["node_state"] == "0")
                    {
                        returnMap["node_result"]  = "成功";
                        returnMap["node_message"]  = "";
                    }
                
                    else
                    {
                        returnMap["node_result"]  = "失败";
                        returnMap["node_message"]  = vectmapArray[0]["Fmessage"];
                    }
                }
                else
                    continue;
                
            }
                
        }
        else if(tmpVec[i]=="组装功能测试")
        {
            if(!workMap["Fass_pid"].empty())
            {
                facInMap["ass_pid"]  = workMap["Fass_pid"];
                facInMap["type"] = "FAC";
                facInMap["limit"] = "1";
                facInMap["offset"] = "0";
                CIdentRelayApi::QueryCollectList(facInMap,facOutMap,vectmapArray,true);//只会有1条结果
                if(vectmapArray.size()==0)
                    continue;
                returnMap["node_message"]  = "";
                returnMap["node_result"]   = vectmapArray[0]["Fresult"];
                if(returnMap["node_result"] == "Success")
                    returnMap["node_state"]    = "0";
                else
                    returnMap["node_state"]    = "1";
            }
            else
                continue;
            
        }
        else if(tmpVec[i]=="终端信息核对")
        {
            if(!workMap["Fass_pid"].empty())
            {
                facInMap["pos_sn"]     = workMap["Fass_pid"];
                facInMap["log_type"]   = "3";//组装
                facInMap["limit"]      = "1";
                facInMap["offset"]     = "0";
                CIdentRelayApi::QueryProducePIDRecordListNoToal(facInMap,facOutMap,vectmapArray,true);
                if(vectmapArray.size()==0)
                    continue;
                returnMap["node_state"]  = vectmapArray[0]["Fresult"];
                vectmapArray[0]["Fid"] = vectmapArray[0]["Flog_id"];;
                if(returnMap["node_state"] == "0")
                {
                    returnMap["node_result"]  = "成功";
                    returnMap["node_message"]  = "";
                }
                else
                {
                    returnMap["node_result"]  = "失败";
                    returnMap["node_message"]  = vectmapArray[0]["Fmessage"];
                }
            }
            else
                continue;
            
        }
        else
            continue;
        CStr2Map tmpResMap;
        CIdentComm::DelMapF(vectmapArray[0],tmpResMap);
        returnMap["node_id"]       = tmpResMap["id"];
        returnMap["node_name"]     = tmpVec[i];
        returnMap["node_time"]     = tmpResMap["create_time"];
        pResData->SetArray("fac_arrary",returnMap);

        //组装段结果判断：如果失败则设置为失败（不能被覆盖），成功则设置为进行中
        if(returnMap["node_state"] == "0")  //组装段成功
        {
            if(fresult != "F")  // 只有在不是失败状态时，才设置为进行中
                fresult = "IN";  //进行中
        }
        else  //组装段失败
        {
            fresult = "F";  //失败（一旦失败，就不能被覆盖）
        }
        
    }

    //开始包装段的数据---用机身号或者PID去查
    tmpVec.clear();
    StrHelper::split(inMap["pack_array"],',',tmpVec);
    InfoLog("tmpVec-size=[%d]",tmpVec.size());
    for(int i=0;i<tmpVec.size();i++)
    {
        InfoLog("tmpVec=[%s]",tmpVec[i].c_str());
        CStr2Map packInMap,packOutMap;
        CStr2Map returnMap;
        vectmapArray.clear();
        if(tmpVec[i]=="防切机下载(存量)" )//只能pack_pid去查，马工有BUG,组装PID也是传的包装PID
        {
            if(!workMap["Fpos_sn"].empty())
            {
                packInMap["pos_sn"]  = workMap["Fpos_sn"];//优先SN查询
                packInMap["log_type"]  = "0";//防切机
                packInMap["limit"] = "1";
                packInMap["offset"] = "0";
                CIdentRelayApi::QueryProduceRecordListNoToal(packInMap,packOutMap,vectmapArray,true);
                if(vectmapArray.size()==0)
                    continue;
                returnMap["node_state"]  = vectmapArray[0]["Fresult"];//获取结果
                vectmapArray[0]["Fid"]   = vectmapArray[0]["Flog_id"];
                if(returnMap["node_state"]=="0")//成功
                {
                    returnMap["node_message"]="";
                    returnMap["node_result"]  = "成功";
                }
                else
                {
                    returnMap["node_result"]  = "失败";
                    returnMap["node_message"]  = vectmapArray[0]["Fmessage"];//错误原因
                }
            }
            else if(!workMap["Fass_pid"].empty())
            {
                packInMap["pos_sn"]  = workMap["Fass_pid"];//包装PID去查
                packInMap["log_type"]  = "0";//防切机
                packInMap["limit"] = "1";
                packInMap["offset"] = "0";
                CIdentRelayApi::QueryProducePIDRecordListNoToal(packInMap,packOutMap,vectmapArray,true);
                if(vectmapArray.size()==0)
                    continue;
                returnMap["node_state"]  = vectmapArray[0]["Fresult"];//获取结果
                vectmapArray[0]["Fid"]   = vectmapArray[0]["Flog_id"];
                if(returnMap["node_state"]=="0")//成功
                {
                    returnMap["node_message"]="";
                    returnMap["node_result"]  = "成功";
                }
                else
                {
                    returnMap["node_result"]  = "失败";
                    returnMap["node_message"]  = vectmapArray[0]["Fmessage"];//错误原因
                }
            }
            else
                continue;
            
                
        }
        else if(tmpVec[i]=="机身号下载" || tmpVec[i]=="防切机下载(新型)")
        {
            
            if(!workMap["Fpos_sn"].empty())//一定要机身号
            {
                packInMap["pos_sn"]  = workMap["Fpos_sn"];
                packInMap["log_type"]  = "1";//下载机身号
                packInMap["limit"] = "1";
                packInMap["offset"] = "0";
                CIdentRelayApi::QueryProduceRecordListNoToal(packInMap,packOutMap,vectmapArray,true);
                if(vectmapArray.size()==0)
                    continue;
                returnMap["node_state"]  = vectmapArray[0]["Fresult"];//获取结果
                vectmapArray[0]["Fid"]   = vectmapArray[0]["Flog_id"];
                if(returnMap["node_state"]=="0")//成功
                {
                    returnMap["node_message"]="";
                    returnMap["node_result"]  = "成功";
                }
                else
                {
                    returnMap["node_result"]  = "失败";
                    returnMap["node_message"]  = vectmapArray[0]["Fmessage"];//错误原因
                }
            }
            else
                continue;

        }
        else if(tmpVec[i]=="包装功能测试" )
        {
            if(!workMap["Fpos_sn"].empty())
            {
                packInMap["pos_sn"]  = workMap["Fpos_sn"];
                packInMap["type"] = "PACK";
                packInMap["limit"] = "1";
                packInMap["offset"] = "0";
                CIdentRelayApi::QueryCollectList(packInMap,packOutMap,vectmapArray,true);//只会有1条结果
                if(vectmapArray.size()==0)
                    continue;
                
                returnMap["node_message"]  = "";
                returnMap["node_result"]   = vectmapArray[0]["Fresult"];
                if(returnMap["node_result"] == "Success")
                    returnMap["node_state"]    = "0";
                else
                    returnMap["node_state"]    = "1";
            }
            else
                continue;
            
        }
        else if(tmpVec[i]=="产品校验")
        {
            
            if(!workMap["Fpos_sn"].empty())//一定要机身号
            {
                packInMap["pos_sn"]  = workMap["Fpos_sn"];
                packInMap["log_type"]  = "3";//有的机型这个是产品校验
                packInMap["limit"] = "1";
                packInMap["offset"] = "0";
                CIdentRelayApi::QueryProduceRecordListNoToal(packInMap,packOutMap,vectmapArray,true);
                if(vectmapArray.size()==0)
                    continue;
                returnMap["node_state"]  = vectmapArray[0]["Fresult"];//获取结果
                vectmapArray[0]["Fid"]   = vectmapArray[0]["Flog_id"];
                if(returnMap["node_state"]=="0")//成功
                {
                    returnMap["node_message"]="";
                    returnMap["node_result"]  = "成功";
                }
                else
                {
                    returnMap["node_result"]  = "失败";
                    returnMap["node_message"]  = vectmapArray[0]["Fmessage"];//错误原因
                }
            }
            else
                continue;

        }
        
        CStr2Map tmpResMap;
        CIdentComm::DelMapF(vectmapArray[0],tmpResMap);
        returnMap["node_id"]       = tmpResMap["id"];
        //returnMap["node_result"]   = tmpResMap["result"];
        returnMap["node_name"]     = tmpVec[i];
        returnMap["node_time"]     = tmpResMap["create_time"];
        pResData->SetArray("pack_arrary",returnMap);

        //包装段结果判断：如果失败则设置为失败（不能被覆盖），成功则设置为成功
        if(returnMap["node_state"] == "0")  //包装段成功
        {
            if(fresult != "F")  // 只有在不是失败状态时，才设置为成功
                fresult = "S";  //成功（完成：走完包装且成功）
        }
        else  //包装段失败
        {
            fresult = "F";  //失败（一旦失败，就不能被覆盖）
        }

    }

    //设置最终结果
    pResData->SetPara("fresult", fresult);
    pResData->SetPara("order",workMap["Forder"]);
    pResData->SetPara("ass_pid",workMap["Fass_pid"]);
    pResData->SetPara("term_name",workMap["Fterm_name"]);
    pResData->SetPara("pos_sn",workMap["Fpos_sn"]);

    //进行中：未走包装且没报错
    //报错：
    //完成：走完包装且成功
	
	return 0;
}

// 输入判断
void CQueryWorkNodeData::CheckParameter( CStr2Map& inMap)
{
	if(inMap["id"].empty())
    {
        ErrorLog("关键字段不能为空-id");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段id不能为空");
    }
    if(inMap["smt_array"].empty()&&inMap["fac_array"].empty()&&inMap["pack_array"].empty())
    {
        ErrorLog("关键字段不能为空-smt_array");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"未配置该机型的工位节点");
    }
    
}
