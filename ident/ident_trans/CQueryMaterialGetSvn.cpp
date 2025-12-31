#include "CQueryMaterialGetSvn.h"
#include "CIdentRelayApi.h"

#undef max
#undef min
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include "rapidjson/stringbuffer.h"

int CQueryMaterialGetSvn::IdentCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map inMap,outMap,sessionMap;
	string uid;

	// 取得请求参数
    pReqData->GetStrMap(inMap);
	
    this->CheckLogin(sessionMap);

	//检查输入参数
	CheckParameter(inMap);
    

    string strUrl = g_k3ErpCfg.strErpUrl;
    strUrl += "/oauth-service/oauth/token";

    string strOutToken;
    if(CIdentPub::HttpERPtoken(strUrl, g_k3ErpCfg.strErpName,g_k3ErpCfg.strErpPasswd,strOutToken) == -1)
    {
        ErrorLog("http请求ERP错误");
        throw CTrsExp(ERR_NO_LOWER_LEVER,"请求ERP错误");
    }
    DebugLog("token=[%s]",strOutToken.c_str());

    string getOrderUrl =  g_k3ErpCfg.strErpUrl;
    getOrderUrl += "/gcs/erp/api/loadInvokeErpWebApi"; 
    string strResponse;
    //用63查询bom信息
    if(CIdentPub::HttpPostERP(getOrderUrl,buildJsonStringBom(0,5,inMap["materi_63"]),strResponse,strOutToken))
    {
        ErrorLog("http请求ERP错误");
        throw CTrsExp(ERR_NO_LOWER_LEVER,"请求ERP错误");
    }
    DebugLog("strResponse=[%s]",strResponse.c_str());
    //获取bom信息，再去查SVN地址
    string strBomNum ;
    parseBomNumJson(strResponse,strBomNum);
    if(strBomNum.length() == 0)
    {
        ErrorLog("获取Bom号失败");
        throw CTrsExp(ERR_NO_LOWER_LEVER,"该63料号获取不到程序bom号(87开头)");
    }

    //strBomNum = "87.01.0001-001";//测试用
    strResponse.clear();
    //用63查询bom信息
    if(CIdentPub::HttpPostERP(getOrderUrl,buildJsonStringForBDMaterial(strBomNum),strResponse,strOutToken))
    {
        ErrorLog("获取SVN失败");
        throw CTrsExp(ERR_NO_LOWER_LEVER,"获取SVN失败");
    }
    DebugLog("strResponse=[%s]",strResponse.c_str());

    string strSVNpath;
    parseSVNPathJson(strResponse,strSVNpath);
    if(strSVNpath.length()==0)
    {
        ErrorLog("SVN地址为空");
        throw CTrsExp(ERR_NO_LOWER_LEVER,"该料号没有找到固件SVN地址");
    }

    //去查询这个订单号的组装PID最小值
    CIdentRelayApi::GetAssMinPid(inMap,outMap,true);
    if(outMap["ass_pid"].length()==0)
    {
        ErrorLog("没有找到最小PID");
        throw CTrsExp(ERR_NO_LOWER_LEVER,"该订单没有组装记录,请在组装之后设置固件下载规则");
    }

    //返回
    pResData->SetPara("order",inMap["order"]);
    pResData->SetPara("model_name",inMap["model_name"]);
    pResData->SetPara("count",inMap["count"]);
    pResData->SetPara("materi_63",inMap["materi_63"]);
    pResData->SetPara("pid_start",outMap["ass_pid"]);
    pResData->SetPara("svn_path",strSVNpath);
    pResData->SetPara("file_name",getFileNameFromUrl(strSVNpath));

}

string CQueryMaterialGetSvn::getFileNameFromUrl(const string& url) {
    // 找到最后一个斜杠的位置
    size_t lastSlashPos = url.find_last_of('/');
    
    // 如果找到了斜杠，返回斜杠后面的部分
    if (lastSlashPos != std::string::npos) {
        return url.substr(lastSlashPos + 1);
    }
    
    // 如果没有斜杠，返回原始字符串
    return url;
}


void CQueryMaterialGetSvn::CheckParameter( CStr2Map& inMap)
{
	if(inMap["materi_63"].empty())
    {
        ErrorLog("关键字段不能为空-materi_63");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段materi_63为空");
    }
	if(inMap["order"].empty())
    {
        ErrorLog("关键字段不能为空-order");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段order为空");
    }
    if(inMap["model_name"].empty())
    {
        ErrorLog("关键字段不能为空-model_name");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段model_name为空");
    }
    if(inMap["count"].empty())
    {
        ErrorLog("关键字段不能为空-count");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段count为空");
    }
}

string CQueryMaterialGetSvn::buildJsonStringBom(int startRow, int limit,string& materialValue)
{
    return R"({
        "formId": "ENG_BOM",
        "deal": "EXECUTE_BILL_QUERY",
        "data": {
           "FormId": "ENG_BOM",
           "FieldKeys": "FMATERIALIDCHILD.FNumber,FMATERIALIDCHILD.FName",
           "StartRow": )" + std::to_string(startRow) + R"(,
           "Limit": )" + std::to_string(limit) + R"(,
           "FilterString": [
           {"Left":"","FieldName":"FMATERIALID.FNumber","Compare":"67","Value":")" + materialValue + R"(","Right":"","Logic":0},
           {"Left":"","FieldName":"FMATERIALIDCHILD.FNumber","Compare":"60","Value":"87","Right":"","Logic":0},
           {"Left":"","FieldName":"FDocumentStatus","Compare":"105","Value":"C","Right":"","Logic":0},
           {"Left":"","FieldName":"FForbidStatus","Compare":"105","Value":"A","Right":"","Logic":0},
           {"Left":"","FieldName":"FUseOrgId.FNumber","Compare":"67","Value":"1100","Right":"","Logic":0}
           ]
       }
    })";
}

void CQueryMaterialGetSvn::parseBomNumJson(const std::string& jsonString, string& OutMap)
{
    // 清空输出容器
    OutMap.clear();
    
    // 解析JSON字符串
    rapidjson::Document doc;
    rapidjson::ParseResult ok = doc.Parse(jsonString.c_str());
    
    // 检查JSON解析是否成功
    if (!ok) 
    {
        ErrorLog("JSON parse error: ");
        return;
    }
    
    // 检查是否为对象类型
    if (!doc.IsObject()) {
        ErrorLog("JSON root is not an object");
        return;
    }
    
    // 检查顶级键是否存在（可选）
    if (doc.HasMember("result") && doc["result"].IsBool()) 
    {
        if(!doc["result"].GetBool())
        {
            ErrorLog("result is false");
            return;
        }

    }
    
    if (doc.HasMember("code") && doc["code"].IsInt()) {
        if(doc["code"].GetInt() != 200)
        {
            ErrorLog("error,code is not 200");
            return;
        }
    }
    
    // 检查data字段是否为数组
    if (!doc.HasMember("data") || !doc["data"].IsArray()) 
    {
        ErrorLog("JSON does not contain a valid 'data' array");
        return;
    }
    
    // 获取data数组（外层数组）
    const rapidjson::Value& dataArray = doc["data"];
    if(dataArray.Size() > 1)
    {
        ErrorLog("多个Bom号,失败");
        return;
    }
    
    // 遍历外层数组中的每个子数组
    for (rapidjson::SizeType i = 0; i < dataArray.Size(); ++i) {
        
        const rapidjson::Value& itemArray = dataArray[i];
        
        // 解析子数组中的元素（按索引位置）
        // 确保子数组有足够的元素
        if (itemArray.Size() >= 2) 
        {
            // 索引0: 创建时间（字符串）
            if (itemArray[0].IsString())
                OutMap = itemArray[0].GetString();
            else 
                ErrorLog("Invalid type for Fcreate at index ");
        } 
        else 
        {
            ErrorLog("Insufficient elements in subarray at index ");
            continue;
        }
    }
}

void CQueryMaterialGetSvn::parseSVNPathJson(const std::string& jsonString, string& OutMap)
{
    // 清空输出容器
    OutMap.clear();
    
    // 解析JSON字符串
    rapidjson::Document doc;
    rapidjson::ParseResult ok = doc.Parse(jsonString.c_str());
    
    // 检查JSON解析是否成功
    if (!ok) 
    {
        ErrorLog("JSON parse error: ");
        return;
    }
    
    // 检查是否为对象类型
    if (!doc.IsObject()) {
        ErrorLog("JSON root is not an object");
        return;
    }
    
    // 检查顶级键是否存在（可选）
    if (doc.HasMember("result") && doc["result"].IsBool()) 
    {
        if(!doc["result"].GetBool())
        {
            ErrorLog("result is false");
            return;
        }

    }
    
    if (doc.HasMember("code") && doc["code"].IsInt()) {
        if(doc["code"].GetInt() != 200)
        {
            ErrorLog("error,code is not 200");
            return;
        }
    }
    
    // 检查data字段是否为数组
    if (!doc.HasMember("data") || !doc["data"].IsArray()) 
    {
        ErrorLog("JSON does not contain a valid 'data' array");
        return;
    }
    
    // 获取data数组（外层数组）
    const rapidjson::Value& dataArray = doc["data"];
    if(dataArray.Size() > 1)
    {
        ErrorLog("多个SVN,失败");
        return;
    }
    
    // 遍历外层数组中的每个子数组
    for (rapidjson::SizeType i = 0; i < dataArray.Size(); ++i) {
        
        const rapidjson::Value& itemArray = dataArray[i];
        
        // 解析子数组中的元素（按索引位置）
        // 确保子数组有足够的元素
        if (itemArray.Size() >= 2) 
        {
            // 索引0: 创建时间（字符串）
            if (itemArray[2].IsString())
                OutMap = itemArray[2].GetString();
            else 
                ErrorLog("Invalid type for Fcreate at index ");
        } 
        else 
        {
            ErrorLog("Insufficient elements in subarray at index ");
            continue;
        }
    }
}

//SVN查询
string CQueryMaterialGetSvn::buildJsonStringForBDMaterial(const std::string& materialValue)
{
    // 使用C++11的原始字符串字面量(Raw string literal)可以避免大量转义字符
    return R"({
        "formId": "BD_MATERIAL",
        "deal": "EXECUTE_BILL_QUERY",
        "data": {
            "FormId": "BD_MATERIAL",
            "FieldKeys": "FNumber,FName,F_BHR_ATTACHMENT",
            "FilterString": [
                {"Left":"","FieldName":"FNumber","Compare":"67","Value":")" + materialValue + R"(","Right":"","Logic":0},
                {"Left":"","FieldName":"FUseOrgId.FNumber","Compare":"67","Value":"1100","Right":"","Logic":0},
                {"Left":"","FieldName":"FDocumentStatus","Compare":"105","Value":"C","Right":"","Logic":0},
                {"Left":"","FieldName":"FForbidStatus","Compare":"105","Value":"A","Right":"","Logic":0},
                {"Left":"","FieldName":"FUseOrgId.FNumber","Compare":"67","Value":"1100","Right":"","Logic":0}
            ]
        }
    })";
}