#include "CQueryModulePassMRateReport.h"
#include "CIdentRelayApi.h"

// 取消宏定义，避免冲突
#undef max
#undef min
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson; 

//各模块一次通过率批量查询（月）
int CQueryModulePassMRateReport::IdentCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map sessMap,sessOutMap;
	CStr2Map inMap, outMap;
	pReqData->GetStrMap(inMap);

	this->CheckLogin(sessMap);
    CheckParameter(inMap);
    int nPos_num = atoi(inMap["pos_num"].c_str());
    int nModule_num = 12; //固定返回12个模块
    int nMonth_num = atoi(inMap["month_num"].c_str());
    if(nMonth_num <= 0)
        nMonth_num = 3;  // 默认3个月
    string strPostBody = createAggregationJson(nPos_num, nModule_num, nMonth_num);

    string strQuerESUrl = g_esCfg.strESBaseUrl + inMap["index_name"]+"/_search";

    ErrorLog("strQuerESUrl=[%s]",strQuerESUrl.c_str());
    ErrorLog("strPostBody=[%s]",strPostBody.c_str());

    string strOutJson;
    sendToElasticsearch(strQuerESUrl,strPostBody,strOutJson);

    ErrorLog("strOutJson=[%s]",strOutJson.c_str());
    if(strOutJson.length()!=0)
        pResData->SetJsonText( CIdentPub::TransformJson(strOutJson));
    else
    {
        ErrorLog("获取ES月统计数据失败");
        throw CTrsExp(ERR_SYSCODE_INCORRECT,"获取月统计数据失败");
    }
    return 0;
}
// 输入判断
void CQueryModulePassMRateReport::CheckParameter( CStr2Map& inMap)
{
	if(inMap["index_name"].empty())
    {
        ErrorLog("关键字段不能为空-index_name");
		CIdentPub::SendAlarm2("Key Fields-index_name is empty[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段index_name为空");
	}
    if(inMap["module_num"].empty())
    {
        inMap["module_num"]="12";
    }
    if(inMap["pos_num"].empty())
    {
        inMap["pos_num"]="5";
    }
    if(inMap["month_num"].empty())
    {
        inMap["month_num"]="3";
    }
}

void CQueryModulePassMRateReport::sendToElasticsearch(const string& strUrl,string& inStr,string& strOutJson)
{
    string  strResponse;
    if(CIdentPub::HttpESPost(strUrl,inStr,strOutJson))
    {
        InfoLog("http post fail %s", strUrl.c_str()); 
		return;
    }
	return ;
}

//按照出货量的top，如果没有JSON，就跳过
string CQueryModulePassMRateReport::createAggregationJson(int size, const string& posname, const vector<string>& moduleList)
{
    // 创建一个字符串缓冲区
    StringBuffer buffer;

    //去查询最近3个月的出货量排行
    
    return buffer.GetString(); // 返回生成的 JSON 字符串
}

string CQueryModulePassMRateReport::createAggregationJson(int& possize, int& module, int month_num)
{
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);

    writer.StartObject();
    writer.Key("size");
    writer.Int(0); // 固定为 0

    writer.Key("query");
    writer.StartObject();
    writer.Key("bool");
    writer.StartObject();
    
    // 添加 must 条件
    writer.Key("must");
    writer.StartArray();

    // 添加时间范围条件（根据月份参数动态生成，包含当天）
    writer.StartObject();
    writer.Key("range");
    writer.StartObject();
    writer.Key("timestamp");
    writer.StartObject();
    writer.Key("gte");
    // 根据月份参数生成时间范围，例如：now-3M/M, now-6M/M
    char gte_str[32];
    snprintf(gte_str, sizeof(gte_str), "now-%dM/M", month_num);
    writer.String(gte_str);
    writer.Key("lte");
    writer.String("now/d");
    writer.EndObject(); // 结束 timestamp
    writer.EndObject(); // 结束 range
    writer.EndObject(); // 结束 must 条件

    writer.EndArray(); // 结束 must
    writer.EndObject(); // 结束 bool
    writer.EndObject(); // 结束 query

    writer.Key("aggs");
    writer.StartObject();

    writer.Key("top_posnames");
    writer.StartObject();
    writer.Key("terms");
    writer.StartObject();
    writer.Key("field");
    writer.String("posname");
    writer.Key("size");
    writer.Int(possize); // 使用传入的 possize 值
    writer.Key("order");
    writer.StartObject();
    writer.Key("_count");
    writer.String("desc");
    writer.EndObject(); // 结束 order
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
    writer.Key("size");
    writer.Int(1000); // 设置较大的size，因为后面会通过bucket_sort筛选
    writer.EndObject(); // 结束 terms

    // 添加聚合
    writer.Key("aggs");
    writer.StartObject();

    writer.Key("total_count");
    writer.StartObject();
    writer.Key("value_count");
    writer.StartObject();
    writer.Key("field");
    writer.String("funclist.func");
    writer.EndObject(); // 结束 value_count
    writer.EndObject(); // 结束 total_count

    writer.Key("success_count");
    writer.StartObject();
    writer.Key("filter");
    writer.StartObject();
    writer.Key("term");
    writer.StartObject();
    writer.Key("funclist.OnceSuccess");
    writer.String("Yes");
    writer.EndObject(); // 结束 term
    writer.EndObject(); // 结束 filter
    writer.EndObject(); // 结束 success_count

    writer.Key("success_rate");
    writer.StartObject();
    writer.Key("bucket_script");
    writer.StartObject();
    writer.Key("buckets_path");
    writer.StartObject();
    writer.Key("success");
    writer.String("success_count._count");
    writer.Key("total");
    writer.String("total_count");
    writer.EndObject(); // 结束 buckets_path
    writer.Key("script");
    writer.String("params.total > 0 ? params.success / params.total : 0");
    writer.EndObject(); // 结束 bucket_script
    writer.EndObject(); // 结束 success_rate

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
    writer.EndObject(); // 结束 description

    // 添加 bucket_selector 以过滤 success_count 为 0 的情况
    // 但当模块数量超过100且成功率为0时，不过滤（保留）
    writer.Key("filter_success");
    writer.StartObject();
    writer.Key("bucket_selector");
    writer.StartObject();
    writer.Key("buckets_path");
    writer.StartObject();
    writer.Key("success");
    writer.String("success_count._count");
    writer.Key("total");
    writer.String("total_count");
    writer.EndObject(); // 结束 buckets_path
    writer.Key("script");
    writer.String("params.success > 0 || params.total > 100");
    writer.EndObject(); // 结束 bucket_selector
    writer.EndObject(); // 结束 filter_success

    // 添加 bucket_sort 按成功率升序排序
    writer.Key("sorted");
    writer.StartObject();
    writer.Key("bucket_sort");
    writer.StartObject();
    writer.Key("sort");
    writer.StartArray();
    writer.StartObject();
    writer.Key("success_rate");
    writer.StartObject();
    writer.Key("order");
    writer.String("asc");
    writer.EndObject(); // 结束 success_rate
    writer.EndObject(); // 结束 sort 数组元素
    writer.EndArray(); // 结束 sort
    writer.Key("size");
    writer.Int(module); // 使用传入的 module 值，返回前N个
    writer.EndObject(); // 结束 bucket_sort
    writer.EndObject(); // 结束 sorted

    writer.EndObject(); // 结束 aggs
    writer.EndObject(); // 结束 by_func
    writer.EndObject(); // 结束 aggs
    writer.EndObject(); // 结束 nested_funclist
    writer.EndObject(); // 结束 aggs
    writer.EndObject(); // 结束 top_posnames
    writer.EndObject(); // 结束 aggs
    writer.EndObject(); // 结束整个 JSON

    return buffer.GetString(); // 返回生成的 JSON 字符串
}

//按照采集上来的数量的top5
string CQueryModulePassMRateReport::createAggregationJson1(int& possize, int& module, int month_num)
{
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);

    writer.StartObject();
    writer.Key("size");
    writer.Int(0); // 固定为 0

    writer.Key("query");
    writer.StartObject();
    writer.Key("bool");
    writer.StartObject();
    
    // 添加 must 条件
    writer.Key("must");
    writer.StartArray();

    // 添加时间范围条件（根据月份参数动态生成，包含当天）
    writer.StartObject();
    writer.Key("range");
    writer.StartObject();
    writer.Key("timestamp");
    writer.StartObject();
    writer.Key("gte");
    // 根据月份参数生成时间范围，例如：now-3M/M, now-6M/M
    char gte_str[32];
    snprintf(gte_str, sizeof(gte_str), "now-%dM/M", month_num);
    writer.String(gte_str);
    writer.Key("lte");
    writer.String("now/d");
    writer.EndObject(); // 结束 timestamp
    writer.EndObject(); // 结束 range
    writer.EndObject(); // 结束 must 条件

    writer.EndArray(); // 结束 must
    writer.EndObject(); // 结束 bool
    writer.EndObject(); // 结束 query

    writer.Key("aggs");
    writer.StartObject();

    writer.Key("top_posnames");
    writer.StartObject();
    writer.Key("terms");
    writer.StartObject();
    writer.Key("field");
    writer.String("posname");
    writer.Key("size");
    writer.Int(possize); // 使用传入的 possize 值
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

    // 添加聚合来获取父级posname的文档数（用于过滤）
    writer.Key("posname_doc_count");
    writer.StartObject();
    writer.Key("reverse_nested");
    writer.StartObject();
    writer.EndObject(); // 结束 reverse_nested
    writer.EndObject(); // 结束 posname_doc_count

    writer.Key("by_func");
    writer.StartObject();
    writer.Key("terms");
    writer.StartObject();
    writer.Key("field");
    writer.String("funclist.func");
    writer.Key("size");
    writer.Int(module); // 使用传入的 module 值
    writer.EndObject(); // 结束 terms

    // 添加聚合
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

    writer.Key("success_count");
    writer.StartObject();
    writer.Key("filter");
    writer.StartObject();
    writer.Key("term");
    writer.StartObject();
    writer.Key("funclist.OnceSuccess");
    writer.String("Yes");
    writer.EndObject(); // 结束 term
    writer.EndObject(); // 结束 filter
    writer.EndObject(); // 结束 success_count

    writer.Key("success_rate");
    writer.StartObject();
    writer.Key("bucket_script");
    writer.StartObject();
    writer.Key("buckets_path");
    writer.StartObject();
    writer.Key("success");
    writer.String("success_count._count");
    writer.Key("total");
    writer.String("total_count");
    writer.EndObject(); // 结束 buckets_path
    writer.Key("script");
    writer.String("params.success / params.total");
    writer.EndObject(); // 结束 bucket_script
    writer.EndObject(); // 结束 success_rate

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
    writer.EndObject(); // 结束 description

    // 添加 bucket_selector 以过滤 success_count 为 0 的情况
    // 但当模块数量超过100且成功率为0时，不过滤（保留）
    writer.Key("filter_success");
    writer.StartObject();
    writer.Key("bucket_selector");
    writer.StartObject();
    writer.Key("buckets_path");
    writer.StartObject();
    writer.Key("success");
    writer.String("success_count._count");
    writer.Key("total");
    writer.String("total_count");
    writer.EndObject(); // 结束 buckets_path
    writer.Key("script");
    writer.String("params.success > 0 || params.total > 100");
    writer.EndObject(); // 结束 bucket_selector
    writer.EndObject(); // 结束 filter_success

    writer.EndObject(); // 结束 aggs
    writer.EndObject(); // 结束 by_func
    writer.EndObject(); // 结束 nested_funclist
    writer.EndObject(); // 结束 aggs
    writer.EndObject(); // 结束 top_posnames
    writer.EndObject(); // 结束 aggs
    writer.EndObject(); // 结束整个 JSON
    writer.EndObject(); // 结束整个 JSON

    return buffer.GetString(); // 返回生成的 JSON 字符串
}


/*string CQueryModulePassMRateReport::createAggregationJson(int& possize, int& module)
{
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);

    writer.StartObject();
    writer.Key("size");
    writer.Int(0); // 固定为 0

    writer.Key("query");
    writer.StartObject();
    writer.Key("bool");
    writer.StartObject();
    
    // 添加 must 条件
    writer.Key("must");
    writer.StartArray();

    // 添加时间范围条件
    writer.StartObject();
    writer.Key("range");
    writer.StartObject();
    writer.Key("timestamp");
    writer.StartObject();
    writer.Key("gte");
    writer.String("now-3M/M");
    writer.Key("lt");
    writer.String("now/M");
    writer.EndObject(); // 结束 timestamp
    writer.EndObject(); // 结束 range
    writer.EndObject(); // 结束 must 条件

    writer.EndArray(); // 结束 must
    writer.EndObject(); // 结束 bool
    writer.EndObject(); // 结束 query

    writer.Key("aggs");
    writer.StartObject();

    writer.Key("top_posnames");
    writer.StartObject();
    writer.Key("terms");
    writer.StartObject();
    writer.Key("field");
    writer.String("posname");
    writer.Key("size");
    writer.Int(possize); // 使用传入的 possize 值
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
    writer.Key("size");
    writer.Int(module); // 使用传入的 module 值
    writer.EndObject(); // 结束 terms

    // 添加聚合
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

    writer.Key("success_count");
    writer.StartObject();
    writer.Key("filter");
    writer.StartObject();
    writer.Key("term");
    writer.StartObject();
    writer.Key("funclist.result");
    writer.String("Success");
    writer.EndObject(); // 结束 term
    writer.EndObject(); // 结束 filter
    writer.EndObject(); // 结束 success_count

    writer.Key("success_rate");
    writer.StartObject();
    writer.Key("bucket_script");
    writer.StartObject();
    writer.Key("buckets_path");
    writer.StartObject();
    writer.Key("success");
    writer.String("success_count._count");
    writer.Key("total");
    writer.String("total_count");
    writer.EndObject(); // 结束 buckets_path
    writer.Key("script");
    writer.String("params.success / params.total");
    writer.EndObject(); // 结束 bucket_script
    writer.EndObject(); // 结束 success_rate

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
    writer.EndObject(); // 结束 description

    writer.EndObject(); // 结束 aggs
    writer.EndObject(); // 结束 by_func
    writer.EndObject(); // 结束 aggs
    writer.EndObject(); // 结束 nested_funclist
    writer.EndObject(); // 结束 aggs
    writer.EndObject(); // 结束 top_posnames
    writer.EndObject(); // 结束 aggs
    writer.EndObject(); // 结束整个 JSON

    return buffer.GetString(); // 返回生成的 JSON 字符串
}
*/