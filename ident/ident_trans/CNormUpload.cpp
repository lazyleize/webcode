#include "CNormUpload.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"
#include "xml/unicode.h"
#include "base/charset.hpp"

// 取消宏定义，避免冲突
#undef max
#undef min
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace aps;

// 构建按订单号查询的JSON
static string CreateOrderQueryJson(const string& order_no)
{
    string filterStr = "[{\"Left\":\"\",\"FieldName\":\"FBillNo\",\"Compare\":\"67\",\"Value\":\"" 
                      + order_no + "\",\"Right\":\"\",\"Logic\":0}]";
    
    string jsonStr = "{\"formId\":\"PRD_MO\",\"deal\":\"EXECUTE_BILL_QUERY\",\"data\":{"
                    "\"FormId\":\"PRD_MO\","
                    "\"FieldKeys\":\"FBillNo,FMaterialId.FNumber,FMaterialId.FName,FQTY\","
                    "\"FilterString\":" + filterStr + "}}";
    
    return jsonStr;
}

// 从ERP查询订单生产数量
// 返回值：0-成功且有数据，-1-失败，1-成功但无数据
static int QueryOrderProductionFromERP(const string& order_no, int& production_qty)
{
    production_qty = 0;
    
    // 1. 获取ERP Token
    string strUrl = g_k3ErpCfg.strErpUrl;
    strUrl += "/oauth-service/oauth/token";
    
    string strOutToken;
    if(CIdentPub::HttpERPtoken(strUrl, g_k3ErpCfg.strErpName, g_k3ErpCfg.strErpPasswd, strOutToken) == -1)
    {
        ErrorLog("获取ERP Token失败: order_no=[%s]", order_no.c_str());
        return -1;
    }
    
    // 2. 查询订单生产数据
    string getOrderUrl = g_k3ErpCfg.strErpUrl;
    getOrderUrl += "/gcs/erp/api/loadInvokeErpWebApi";
    
    string queryJson = CreateOrderQueryJson(order_no);
    string strResponse;
    
    if(CIdentPub::HttpPostERP(getOrderUrl, queryJson, strResponse, strOutToken) != 0)
    {
        ErrorLog("查询ERP失败: order_no=[%s]", order_no.c_str());
        return -1;
    }
    
    // 3. 解析返回的JSON
    rapidjson::Document doc;
    rapidjson::ParseResult ok = doc.Parse(strResponse.c_str());
    
    if (!ok || !doc.IsObject())
    {
        ErrorLog("解析ERP响应JSON失败: order_no=[%s]", order_no.c_str());
        return -1;
    }
    
    // 检查result和code
    if (doc.HasMember("result") && doc["result"].IsBool() && !doc["result"].GetBool())
    {
        ErrorLog("ERP返回result=false: order_no=[%s]", order_no.c_str());
        return -1;
    }
    
    if (doc.HasMember("code") && doc["code"].IsInt() && doc["code"].GetInt() != 200)
    {
        ErrorLog("ERP返回code!=200: order_no=[%s], code=[%d]", order_no.c_str(), doc["code"].GetInt());
        return -1;
    }
    
    // 解析data数组，累加所有记录的生产数量，返回订单总数量
    if (!doc.HasMember("data") || !doc["data"].IsArray())
    {
        DebugLog("订单无生产记录: order_no=[%s], production_qty=0", order_no.c_str());
        production_qty = 0;
        return 1;  // 成功但无数据
    }
    
    const rapidjson::Value& dataArray = doc["data"];
    int total_qty = 0;
    int record_count = 0;
    
    // 解析返回数据：格式为 [["WKI25407", "62.01.3828-0008", "N82-K2-V1.0...", 4120.0], ...]
    // 字段顺序：FBillNo(0), FMaterialId.FNumber(1), FMaterialId.FName(2), FQTY(3)
    // 数量在索引3（最后一个元素），是浮点数，需要转换为整数
    // 可能查询到多个订单号，需要把所有记录的数量相加
    for (rapidjson::SizeType i = 0; i < dataArray.Size(); ++i)
    {
        const rapidjson::Value& itemArray = dataArray[i];
        if (itemArray.IsArray() && itemArray.Size() >= 4)
        {
            // 获取订单号（索引0）
            string record_order_no = "";
            if (itemArray[0].IsString())
            {
                record_order_no = itemArray[0].GetString();
            }
            
            // 数量在索引3（FQTY字段）
            int record_qty = 0;
            const rapidjson::Value& qtyValue = itemArray[3];
            if (qtyValue.IsNumber())
            {
                // 浮点数转整数，去掉小数点
                record_qty = static_cast<int>(qtyValue.GetDouble());
                total_qty += record_qty;
            }
            else if (qtyValue.IsString())
            {
                // 如果是字符串，先转换为浮点数再转整数
                double qty = atof(qtyValue.GetString());
                record_qty = static_cast<int>(qty);
                total_qty += record_qty;
            }
            
            record_count++;
            DebugLog("ERP记录[%d]: 订单号=[%s], 数量=[%d]", 
                    record_count, record_order_no.c_str(), record_qty);
        }
        else
        {
            ErrorLog("ERP返回数据格式错误: 记录[%d]不是有效数组或字段不足", i);
        }
    }
    production_qty = total_qty;
    DebugLog("查询ERP成功: 查询订单号=[%s], 返回记录数=[%d], 总生产数量=[%d]", 
            order_no.c_str(), record_count, production_qty);
    return 0;  // 成功且有数据
}

int CNormUpload::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,outMap;

	string strPostdata=pReqData->GetPostData();
	//DebugLog("strPostdata=[%s]",strPostdata.c_str());
	CIdentPub::parsePubRespJson(strPostdata,inMap);

	CheckParameter(inMap);

    char szSavePath[125]={0};
	char szStandPath[125]={0};
	char cmd[512]={0};

	memcpy(szSavePath,g_mTransactions[GetTid()].m_mVars["save_path"].c_str(),
                      g_mTransactions[GetTid()].m_mVars["save_path"].length());

	memcpy(szStandPath,g_mTransactions[GetTid()].m_mVars["save_stand_log"].c_str(),
                      g_mTransactions[GetTid()].m_mVars["save_stand_log"].length());

	DebugLog("szSavePath=[%s]",g_mTransactions[GetTid()].m_mVars["save_path"].c_str());
	//获取年月
	string dateFile = CIdentPub::GetFormatDateNow();
	char saveFullPath[128]={0};
	char standPath[128]={0};
	snprintf(saveFullPath,sizeof(saveFullPath),"%s%s",szSavePath,dateFile.substr(0,6).c_str());
	snprintf(cmd,sizeof(cmd),"mkdir -p %s && chmod 777 %s",saveFullPath,saveFullPath);
    system(cmd);
	DebugLog("cmd=[%s]",cmd);

	snprintf(standPath,sizeof(standPath),"%s%s",szStandPath,dateFile.substr(0,6).c_str());
	snprintf(cmd,sizeof(cmd),"mkdir -p %s && chmod 777 %s",standPath,standPath);
    system(cmd);
	DebugLog("stand=[%s]",cmd);

	inMap["minio_path"] = _G2U(inMap["file_path"]);
	//从MinIO去查询日志是否上传成功
	if(0 != CIdentPub::CheckFileFromMinIO(g_S3Cfg,inMap["minio_path"].c_str()))
	{
		ErrorLog("获取不到文件,延时500ms 再次拉取..");
		usleep(500000);
		if(0 != CIdentPub::CheckFileFromMinIO(g_S3Cfg,inMap["minio_path"].c_str()))
		{
			ErrorLog("获取不到文件");
			CIdentPub::SendAlarm2("获取不到文件 [%s]",inMap["minio_path"].c_str());
			throw(CTrsExp(ERR_UPFILE_COUNT, "get file error"));
			return 0;
		}
	}

	if(!inMap["stand_path"].empty())
	{
		inMap["stand_minio_path"] = _G2U(inMap["stand_path"]);
		if(0 != CIdentPub::CheckFileFromMinIO(g_S3Cfg,inMap["stand_minio_path"].c_str()))
		{
			ErrorLog("获取不到标准日志文件");
			CIdentPub::SendAlarm2("获取不到标准日志文件 [%s]",inMap["stand_minio_path"].c_str());
			throw(CTrsExp(ERR_UPFILE_COUNT, "获取不到标准日志文件"));
			return 0;
		}
		inMap["stand_path"] = standPath;
		inMap["stand_path"] += "/" + inMap["stand_minio_path"].substr(inMap["stand_minio_path"].find_last_of('/')+1);
	}

	inMap["file_path"] = saveFullPath;
	inMap["file_path"] += "/" + inMap["minio_path"].substr(inMap["minio_path"].find_last_of('/')+1);

	//错误详情转UTF8
	inMap["message"] = _G2U(inMap["message"]);

	//如果是组装段需要去判断机型是否存在，不存在就创建
	CStr2Map temInMap,temOutMap;
	if(inMap["log_type"]=="0"||inMap["log_type"]=="2")//下防切或者组装
	{
		if(!inMap["pos_name"].empty())
		{
			temInMap["pos_name"] = inMap["pos_name"];
			CIdentRelayApi::CheckPosName(inMap,outMap,false);//不报错

			if(outMap["is_new"] == "1")
				CIdentPub::SendAlarm2("新机型,请配置工位节点[%s]",temInMap["pos_name"].c_str());
		}
	}

    //有的工厂用的老版本的dll，数据错乱，校正出来
	if(inMap["message"]=="成功" && inMap["result"]=="1")
	{
		inMap["result"]="0";
		int type = atoi(inMap["log_type"].c_str())-1;
		inMap["log_type"] = to_string(type);
	}

	//去更新工厂生产概况表
	inMap["produc_type"] = "0";//表示工厂生产日志类的
	CIdentRelayApi::UpdateTermRecord(inMap,outMap,true);

	//存记录
	inMap["imei"] = replaceAll(inMap["imei"],"\r\n"," ");

	CIdentRelayApi::NormalRecordLog(inMap,outMap,true);	

	// 存入订单号和是否查询过的标记
	if(!outMap["is_create"].empty() && !inMap["order"].empty())
	{
		string order_no = inMap["order"];
		string cache_value;
		
		// 先查询缓存，判断订单是否已查询过
		if(GetCache(order_no, cache_value) == 0)
		{
			// 缓存标记存在，说明已查询过，跳过ERP查询
			DebugLog("订单已查询过（缓存标记存在），跳过ERP查询: order_no=[%s]", order_no.c_str());
		}
		else
		{
			// 缓存标记不存在，说明未查询过，去查询ERP
			DebugLog("订单未查询过（缓存标记不存在），开始查询ERP: order_no=[%s]", order_no.c_str());
			
			int production_qty = 0;
			int ret = QueryOrderProductionFromERP(order_no, production_qty);
			
			// 无论查询成功或失败，都写入缓存标记，避免重复查询
			if(SetOrUpdateCache(order_no, "1", 86400) == 0)
			{
				DebugLog("设置订单查询标记成功: order_no=[%s]", order_no.c_str());
			}
			else
			{
				ErrorLog("设置订单查询标记失败: order_no=[%s]", order_no.c_str());
			}
			
			// 如果ERP查询成功且有数据，需要去建单
			if(ret == 0 && production_qty > 0)
			{
				DebugLog("查询ERP成功且有数据，需要建单: order_no=[%s], production_qty=[%d]",order_no.c_str(), production_qty);
				// 调用建单接口
                CStr2Map inInsertMap,OutInsertMap;
                inInsertMap["order"] = order_no;
                inInsertMap["pos_name"] = inMap["pos_name"];
                if(inMap["log_type"]=="2")
                    inInsertMap["state"] = "组装";
                else if(inMap["log_type"]=="0" || inMap["log_type"]=="1")
                {
                    inInsertMap["state"] = "包装";
                }
                else if(inMap["log_type"]=="3")
                {
                    // 如果pos_name中包含"KD"，则为包装，否则为组装
                    if(!inMap["pos_name"].empty() && inMap["pos_name"].find("KD") != string::npos)
                        inInsertMap["state"] = "包装";
                    else
                        inInsertMap["state"] = "组装";
                }
                else
                    inInsertMap["state"] = "包装";

                inInsertMap["order_total"] = to_string(production_qty);
				inInsertMap["factory"] = outMap["factory"];
				CIdentRelayApi::CreateOrderChange(inInsertMap,OutInsertMap,false);
			}
			else if(ret == 0)
			{
				DebugLog("查询ERP成功但无数据: order_no=[%s], production_qty=[%d]", 
						order_no.c_str(), production_qty);
			}
			else
			{
				ErrorLog("查询ERP失败: order_no=[%s]", order_no.c_str());
			}
		}
	}

	return 0;
}

// 输入判断
void CNormUpload::CheckParameter( CStr2Map& inMap)
{
	if(inMap["file_path"].empty())
    {
        ErrorLog("关键字段不能为空-file_path");
		CIdentPub::SendAlarm2("Key Fields-file_path is empty[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段file_path为空");
    }
    if(inMap["name"].empty())
    {
        ErrorLog("关键字段不能为空-name操作人");
		CIdentPub::SendAlarm2("Key Fields-name is empty[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段name为空");
    }
    if(inMap["trans_time"].length() != 14)           
    {
        ErrorLog("关键字段不能为空-trans_time长度错误");
		CIdentPub::SendAlarm2("Key Fields-trans_time is empty[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段trans_time长度错误");
    }
	if(inMap["cust_id"].empty())
    {
        ErrorLog("关键字段不能为空-cust_id");
		CIdentPub::SendAlarm2("Key Fields-cust_id is empty[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段cust_id为空");
    }
	if(inMap["result"].empty())
    {
        ErrorLog("关键字段不能为空-result");
		CIdentPub::SendAlarm2("Key Fields-result is empty[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段result为空");
    }
	if(inMap["message"].empty())
	{
		ErrorLog("关键字段不能为空-message");
		CIdentPub::SendAlarm2("Key Fields-message is empty[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段message为空");
	}
	if(inMap["log_type"].empty())
	{
		ErrorLog("关键字段不能为空-log_type");
		CIdentPub::SendAlarm2("Key Fields-log_type is empty[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段log_type为空");
	}

	//检查传入的字段
	if(inMap["log_type"]=="0") //量产智能 Recovery 下防切，组装流程，传组装PID
	{
		if(inMap["pack_pid"].empty())
    	{
        	ErrorLog("下载防切,组装PID为空");
			CIdentPub::SendAlarm2("下载防切,组装PID不能为空[%s]",ERR_SIGNATURE_INCORRECT);
        	//throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段ass_pid为空");

			//获取当前时间，当作PID
			inMap["pack_pid"]=CIdentPub::GetCurrentDateTime();

    	}
		inMap["ass_pid"]= inMap["pack_pid"];//入库，后面查询用
	}
	else if(inMap["log_type"]=="1")//量产和统一协议机型 下机身号流程，包装流程，传机身号和包装PID
	{
		if(inMap["sn"].empty())
    	{
        	ErrorLog("下机身号,机身号为空");
			CIdentPub::SendAlarm2("下机身号,机身号不能为空[%s]",ERR_SIGNATURE_INCORRECT);
        	//throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段sn为空");
			inMap["sn"] ="123456789ABCDEF";
    	}
	}
	else if(inMap["log_type"]=="2")//统一协议组装流程，传组装PID
	{
		if(inMap["ass_pid"].empty())
    	{
        	ErrorLog("统一协议组装,组装PID为空");
			CIdentPub::SendAlarm2("统一协议组装,组装PID不能为空[%s]",ERR_SIGNATURE_INCORRECT);
        	//throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段ass_pid为空");
			inMap["pack_pid"]=CIdentPub::GetCurrentDateTime();
    	}
		else
		{
			if(inMap["order"].empty() && !inMap["ass_pid"].empty())
			{
				// 去掉ass_pid的后6位作为订单号
				string ass_pid = inMap["ass_pid"];
				if(ass_pid.length() > 6)
				{
					inMap["order"] = ass_pid.substr(0, ass_pid.length() - 6);
				}
			}
		}
	}
	else if(inMap["log_type"]=="3")//KD68产品校验流程和N80解触发流程，该流程传时间戳
	{
		if(inMap["pos_name"]=="N80")//机型判断N80，在inMap["sn"]加上 ：N80+ASS+到秒的时间戳
			inMap["sn"] = "N80ASS" + CIdentPub::GetCurrentDateTime();

		// 只有当pos_name包含"KD"字符串时才执行此逻辑
		if(inMap["pos_name"].find("KD") != string::npos && inMap["sn"].empty())//需要对一下字段
    	{
        	ErrorLog("KD68产品校验流程和N80解触发流程,SN为空");
			//CIdentPub::SendAlarm2("KD68产品校验流程和N80解触发流程,SN不能为空[%s]",ERR_SIGNATURE_INCORRECT);
        	//throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段sn为空");
			inMap["sn"] ="123456789ABCDEF";
    	}
	}
	else //统一协议维修流程
	{
		if(inMap["sn"].empty())
    	{
        	ErrorLog("统一协议维修流程,机身号为空");
			CIdentPub::SendAlarm2("统一协议维修流程,机身号不能为空[%s]",ERR_SIGNATURE_INCORRECT);
        	//throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段sn为空");
			inMap["sn"] ="123456789ABCDEF";
    	}
	}
}

string CNormUpload::replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}
