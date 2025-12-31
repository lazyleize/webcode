#include "CQueryCollectionData.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"
#include "xml/unicode.h"
#include <base/file.hpp>
#include <base/strHelper.hpp>
#include <algorithm>

// 取消宏定义，避免冲突
#undef max
#undef min
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace aps;
using namespace rapidjson;
int CQueryCollectionData::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,tmpMap,outMap,sessionMap;
	string uid;

	// 取得请求参数
    pReqData->GetStrMap(inMap);

	this->CheckLogin(sessionMap);
	
	//检查输入参数
	CheckParameter(inMap);

    CIdentRelayApi::QueryCollectPath(inMap,tmpMap,true);

    pResData->SetPara("order",tmpMap["Forder_id"]);
    pResData->SetPara("ass_pid",tmpMap["Fass_pid"]);
    pResData->SetPara("term_name",tmpMap["Fterm_type"]);
    pResData->SetPara("pos_sn",tmpMap["Fpossn"]);
    pResData->SetPara("type",tmpMap["Ftype"]);
    pResData->SetPara("result",tmpMap["Fresult"]);
    pResData->SetPara("collection_time",tmpMap["Ftime_map"]);

    string OutStrContext;
    //去解析JSON内容
    if(!File::exists(tmpMap["File_path"]))
    {
        InfoLog("去ES获取数据采集文件内容");
        string strESUrl = g_esCfg.strESBaseUrl + StrHelper::toLower(tmpMap["Ftype"]) +"/_search";
        string queryJson = R"({
            "query": {
                "term": {
                    "_id": ")" + inMap["id"] + R"("
                }
            }
        })";
        
        QueryESdoc(strESUrl,queryJson,OutStrContext);
        
        if(OutStrContext.length()==0)
        {
            ErrorLog("找不到数据采集文件id[%s]",inMap["id"].c_str());
            throw CTrsExp(ERR_SIGNATURE_INCORRECT,"找不到数据采集文件");
        }

    }
    else
    {
        //获取内容
        ifstream inFile(tmpMap["File_path"].c_str(), std::ios::in | std::ios::binary);
        if (!inFile)
	    {
		    ErrorLog("文件打开失败[%s]",tmpMap["File_path"].c_str());
		    throw(CTrsExp(ERR_UPFILE_COUNT, "文件打开失败"));
		    return 0;
	    }
        std::ostringstream oss;
        oss << inFile.rdbuf(); // 读取整个文件内容到ostringstream中

	    OutStrContext = oss.str();
    }

    //解析出检测的内容列表
    Document document;
    if(!document.Parse(OutStrContext.c_str()).HasParseError())
    {
        if (document.HasMember("funclist") && document["funclist"].IsArray())
        {
            const Value& funcList = document["funclist"];
            pResData->SetPara("total",to_string(funcList.Size()));

            for (size_t i = 0; i < funcList.Size(); i++)
            {
                CStr2Map resultMap;
                const Value& func = funcList[i];
                resultMap["items_name"]   = func["description"].GetString();
                resultMap["items_time"]   = to_string(func["duration"].GetInt()); 
                resultMap["items_once"]   = func["OnceSuccess"].GetString();
                resultMap["items_result"] = func["result"].GetString();
                resultMap["rate"]         = func["SuccessRate"].GetString();
                if (func.HasMember("errorMsg") && func["errorMsg"].IsArray())
                {
                    const Value& errorMsgArray = func["errorMsg"];
                    string errorMessage = "";
                    for (size_t j = 0; j < errorMsgArray.Size(); j++)
                    {
                        if (j > 0) errorMessage += ",";
                            errorMessage += errorMsgArray[j].GetString();
                    }
                    //去掉回车
                    errorMessage.erase(std::remove(errorMessage.begin(), errorMessage.end(), '\r'), errorMessage.end());
                    errorMessage.erase(std::remove(errorMessage.begin(), errorMessage.end(), '\n'), errorMessage.end());
                    resultMap["error_message"] = errorMessage;
                }
                else
                    resultMap["error_message"]="";
            
                pResData->SetArray(resultMap);
            }
        }
        else
        {
            ErrorLog("找不到数据采集文件id[%s]",inMap["id"].c_str());
            throw CTrsExp(ERR_SIGNATURE_INCORRECT,"找不到数据采集文件");
        }
        
        
    }
    else
    {
        ErrorLog("数据采集文件Json解析报错[%s]",tmpMap["File_path"].c_str());
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"数据采集文件Json解析报错");
    }

	return 0;
}

// 输入判断
void CQueryCollectionData::CheckParameter( CStr2Map& inMap)
{
	if(inMap["id"].empty())
    {
        ErrorLog("关键字段不能为空-id");
		CIdentPub::SendAlarm2("Key Fields-id is empty[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段id为空");
	}
    /*if(inMap["type"].empty())
    {
        ErrorLog("关键字段不能为空-type");
		CIdentPub::SendAlarm2("Key Fields-type is empty[%s]",ERR_SIGNATURE_INCORRECT);
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段type为空");
	}*/
}

void CQueryCollectionData::QueryESdoc(const string& strUrl,string& inStr,string& outStr)
{
    string  strResponse;

    if(CIdentPub::HttpESPost(strUrl,inStr,strResponse))
    {
        ErrorLog("http post fail %s", strUrl.c_str()); 
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"请求ES失败");
    }
    InfoLog("strResponse %s", strResponse.c_str()); 
	//解析返回
    Document responseDocument;
    responseDocument.Parse(strResponse.c_str());
    if (responseDocument.HasParseError())
    {
        ErrorLog("strResponse %s", strResponse.c_str()); 
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"解析ES返回失败");
    }

    // 构建输出
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);

    if (responseDocument.HasMember("hits") && responseDocument["hits"].HasMember("hits"))
    {
        const Value& hits = responseDocument["hits"]["hits"];
        writer.StartObject();
        writer.Key("es_state");
        writer.String("S");
        writer.Key("documents");
        writer.StartArray();
        for (SizeType i = 0; i < hits.Size(); i++) 
        {
            const Value& document = hits[i]["_source"];
            document.Accept(writer); // 将文档内容写入 buffer
        }
        writer.EndArray(); // End documents array
        writer.EndObject(); // End response object
        outStr = buffer.GetString(); // 设置输出字符串
    }
    else
    {
        outStr="";//文档为空
    }

}