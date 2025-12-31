#include "CGetDataAcquisitionReport.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"
#include <cstdlib> // for system
#include <cstdio>  // for popen and pclose
#include <datetime.hpp>
#include <file.hpp>

// 取消宏定义，避免冲突
#undef max
#undef min
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace aps;
using namespace rapidjson;

int CGetDataAcquisitionReport::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,outMap,sessionMap;
    vector<CStr2Map> vectmapArray;

    // 取得请求参数
    pReqData->GetStrMap(inMap);
    //调试屏蔽
	this->CheckLogin(sessionMap);

    /*inMap["usertype"] = sessionMap["usertype"];
    int usertype = atoi(sessionMap["usertype"].c_str());
    if(usertype != 2  && usertype != 1)
    {
        ErrorLog("没有权限: usertype=[%s]" ,inMap["usertype"].c_str());
        throw CTrsExp(ERR_NO_LOWER_LEVER,"没有权限");
    }*/
    CheckParameter(inMap);
    
    //从配置脚本的路径和保存路径json文件存放路径
    char pythopath[256]={0};
    char jsonbasepath[256]={0};
    char exceSavepath[256]={0};
    memcpy(pythopath,g_mTransactions[GetTid()].m_mVars["python_path"].c_str(),g_mTransactions[GetTid()].m_mVars["python_path"].length());
    memcpy(jsonbasepath,g_mTransactions[GetTid()].m_mVars["json_path_base"].c_str(),g_mTransactions[GetTid()].m_mVars["json_path_base"].length());
    memcpy(exceSavepath,g_mTransactions[GetTid()].m_mVars["down_path_base"].c_str(),g_mTransactions[GetTid()].m_mVars["down_path_base"].length());

    string strPostBody = createAggregationJson(inMap);     
    InfoLog("strPostBody=[%s]",strPostBody.c_str());             

    //请求ES
    string strQuerESUrl = g_esCfg.strESBaseUrl + inMap["product_type"]+"/_search";
    string strOutJson;
    sendToElasticsearch(strQuerESUrl,strPostBody,strOutJson);
    InfoLog("strOutJson=[%s]",strOutJson.c_str());

    //写入json文件
    string strJsonPath = jsonbasepath;
    strJsonPath += "EsData.json";

    std::ofstream file(strJsonPath);
    if (file.is_open())
    {
        file << strOutJson; // 写入 JSON 字符串
        file.close();
        InfoLog("JSON 数据已成功写入到[%s]",strJsonPath.c_str());
    } else 
    {
        ErrorLog("File open error.");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"JSON File open error");
    }

    //解析之后调用python3脚本
    string strSavePath = exceSavepath;
    strSavePath += Datetime::now().format("YMDHISU")+".xlsx";

    char cmd[512];
    snprintf(cmd, sizeof(cmd), "python3 %s %s %s  > /dev/null 2>&1", pythopath, strJsonPath.c_str(), strSavePath.c_str());
    InfoLog(": cmd=[%s]", cmd);

    if (std::system(cmd) != 0)
    {
        ErrorLog("Pytho3执行异常.");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"报表生成异常");
    }

    //处理 strDonwPath
    string downexcle;
    size_t nPos = strSavePath.find("download");
    if(nPos != string::npos)
        downexcle = strSavePath.substr(nPos);

    pResData->SetPara("report_url",downexcle);


	return 0;
}

void CGetDataAcquisitionReport::sendToElasticsearch(const string& strUrl,string& inStr,string& strOutJson)
{
    string  strResponse;
    if(CIdentPub::HttpESPost(strUrl,inStr,strOutJson))
    {
        InfoLog("http post fail %s", strUrl.c_str()); 
		return;
    }
	return ;
}

// 输入判断
void CGetDataAcquisitionReport::CheckParameter( CStr2Map& inMap)
{
	if(inMap["type"].empty())
    {
        ErrorLog("type缺少");
		CIdentPub::SendAlarm2("type缺少[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"type缺少");
    }
    if(inMap["type"]=="M")
    {
        if(inMap["month"].empty())
            inMap["month"] = Datetime::now().format("Y-M-D").substr(0,7);
        else
        {
            //转换成YYYY-MM
            m_date = m_date.now();
            m_date.setMonth(atoi(inMap["month"].c_str()));
            inMap["month"] = m_date.toString().substr(0,7);
        }
    }

    if(inMap["product_type"].empty())
    {
        inMap["product_type"] = "fac";
    }
}


string CGetDataAcquisitionReport::createAggregationJson(CStr2Map& inMap)
{
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);

    writer.StartObject(); // 开始整个 JSON 对象

    writer.Key("size");
    writer.Int(0); // 固定为 0

    writer.Key("query");
    writer.StartObject();
    writer.Key("bool");
    writer.StartObject();
    writer.Key("must");
    writer.StartArray();

    // 添加时间范围条件
    if (!inMap["month"].empty()) {
        writer.StartObject();
        writer.Key("range");
        writer.StartObject();
        writer.Key("timestamp");
        writer.StartObject();
        writer.Key("gte");
        writer.String((inMap["month"] + "-01").c_str()); // 假设 month 是 "YYYY-MM"
        writer.Key("lt");
        writer.String((m_date.addMonths(1).toString().substr(0,7)+"-01").c_str()); // 下一个月的开始
        writer.EndObject(); // 结束 timestamp
        writer.EndObject(); // 结束 range
        writer.EndObject(); // 结束 must 条件
    } else if (!inMap["year"].empty()) {
        writer.StartObject();
        writer.Key("range");
        writer.StartObject();
        writer.Key("timestamp");
        writer.StartObject();
        writer.Key("gte");
        writer.String((inMap["year"] + "-01-01").c_str()); // 假设 year 是 "YYYY"
        writer.Key("lt");
        writer.String((inMap["year"] + "-12-31").c_str()); // 该年的最后一天
        writer.EndObject(); // 结束 timestamp
        writer.EndObject(); // 结束 range
        writer.EndObject(); // 结束 must 条件
    } else if (!inMap["date_beg"].empty() && !inMap["date_end"].empty()) {
        writer.StartObject();
        writer.Key("range");
        writer.StartObject();
        writer.Key("timestamp");
        writer.StartObject();
        writer.Key("gte");
        writer.String(inMap["date_beg"].c_str()); // 开始日期
        writer.Key("lt");
        writer.String(inMap["date_end"].c_str()); // 结束日期
        writer.EndObject(); // 结束 timestamp
        writer.EndObject(); // 结束 range
        writer.EndObject(); // 结束 must 条件
    }

    // 添加 orderNumber 条件
    if (!inMap["order"].empty()) {
        writer.StartObject();
        writer.Key("term");
        writer.StartObject();
        writer.Key("orderNumber");
        writer.String(inMap["order"].c_str()); // 替换为您想要的订单号
        writer.EndObject(); // 结束 term
        writer.EndObject(); // 结束 must 条件
    }

    // 添加 posname 条件
    if (!inMap["term_name"].empty()) {
        writer.StartObject();
        writer.Key("term");
        writer.StartObject();
        writer.Key("posname");
        writer.String(inMap["term_name"].c_str()); // 替换为您想要的机型名称
        writer.EndObject(); // 结束 term
        writer.EndObject(); // 结束 must 条件
    }

    writer.EndArray(); // 结束 must
    writer.EndObject(); // 结束 bool
    writer.EndObject(); // 结束 query

    // 开始聚合部分
    writer.Key("aggs");
    writer.StartObject();

    writer.Key("top_posnames");
    writer.StartObject();
    writer.Key("terms");
    writer.StartObject();
    writer.Key("field");
    writer.String("posname");
    writer.EndObject(); // 结束 terms

    writer.Key("aggs");
    writer.StartObject();

    writer.Key("nested_funclist");
    writer.StartObject();
    writer.Key("nested");
    writer.StartObject();
    writer.Key("path");
    writer.String("funclist");
    writer.EndObject(); // 结束 nested

    writer.Key("aggs");
    writer.StartObject();

    writer.Key("by_func");
    writer.StartObject();
    writer.Key("terms");
    writer.StartObject();
    writer.Key("field");
    writer.String("funclist.func");
    writer.EndObject(); // 结束 terms

    writer.Key("aggs"); // 添加 aggs
    writer.StartObject();

    writer.Key("total_count");
    writer.StartObject();
    writer.Key("value_count");
    writer.StartObject();
    writer.Key("field");
    writer.String("funclist.func");
    writer.EndObject(); // 结束 value_count
    writer.EndObject(); // 结束 total_count

    writer.Key("total_duration");
    writer.StartObject();
    writer.Key("sum");
    writer.StartObject();
    writer.Key("field");
    writer.String("funclist.duration");
    writer.EndObject(); // 结束 sum
    writer.EndObject(); // 结束 total_duration

    writer.Key("average_duration");
    writer.StartObject();
    writer.Key("bucket_script");
    writer.StartObject();
    writer.Key("buckets_path");
    writer.StartObject();
    writer.Key("totalCount");
    writer.String("total_count");
    writer.Key("totalDuration");
    writer.String("total_duration");
    writer.EndObject(); // 结束 buckets_path
    writer.Key("script");
    writer.String("params.totalDuration / params.totalCount");
    writer.EndObject(); // 结束 bucket_script
    writer.EndObject(); // 结束 average_duration

    writer.Key("total_denominator");
    writer.StartObject();
    writer.Key("scripted_metric");
    writer.StartObject();
    writer.Key("init_script");
    writer.String("state.total = 0;");
    writer.Key("map_script");
    writer.String("if (doc[\"funclist.SuccessRate\"].size() > 0) { String rate = doc[\"funclist.SuccessRate\"].value; int slashIndex = rate.indexOf(\"/\"); if (slashIndex >= 0 && slashIndex < rate.length() - 1) { String denominator = rate.substring(slashIndex + 1); try { state.total += Integer.parseInt(denominator); } catch (Exception e) { /* 忽略解析错误 */ } } }");
    writer.Key("combine_script");
    writer.String("return state.total;");
    writer.Key("reduce_script");
    writer.String("double totalDenominator = 0; for (s in states) { totalDenominator += s; } return totalDenominator;");
    writer.EndObject(); // 结束 scripted_metric
    writer.EndObject(); // 结束 total_denominator

    writer.Key("description");
    writer.StartObject();
    writer.Key("top_hits");
    writer.StartObject();
    writer.Key("_source");
    writer.StartObject();
    writer.Key("includes");
    writer.StartArray();
    writer.String("funclist.description");
    writer.EndArray(); // 结束 includes
    writer.EndObject(); // 结束 _source
    writer.Key("size");
    writer.Int(1); // 返回每个功能的一个描述
    writer.EndObject(); // 结束 top_hits
    writer.EndObject(); // 结束 by_func
    writer.EndObject(); // 结束 aggs
    writer.EndObject(); // 结束 nested_funclist
    writer.EndObject(); // 结束 aggs
    writer.EndObject(); // 结束 top_posnames
    writer.Key("total_docs");
    writer.StartObject();
    writer.Key("value_count");
    writer.StartObject();
    writer.Key("field");
    writer.String("timestamp");
    writer.EndObject(); // 结束 value_count
    writer.EndObject(); // 结束 total_docs
    writer.EndObject(); // 结束 aggs
    writer.EndObject(); // 结束整个 JSON 对象
    writer.EndObject(); // 结束整个 JSON 对象
    writer.EndObject(); 

    return buffer.GetString(); // 返回生成的 JSON 字符串
}