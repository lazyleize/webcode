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
    getOrderUrl += "/gcs/erp/api/loadBomByMaterialCode"; 
    string strResponse;
    //用63查询bom信息
    string jsonRequest = buildJsonStringForLoadBom(inMap["materi_63"]);
    DebugLog("请求JSON=[%s]",jsonRequest.c_str());
    if(CIdentPub::HttpPostERP(getOrderUrl, jsonRequest, strResponse, strOutToken))
    {
        ErrorLog("http请求ERP错误");
        throw CTrsExp(ERR_NO_LOWER_LEVER,"请求ERP错误");
    }
    //DebugLog("strResponse=[%s]",strResponse.c_str());
    
    //解析BOM信息，提取所有SVN地址
    vector<CStr2Map> svnList;
    parseBomJsonForSVN(strResponse, svnList);
    
    if(svnList.size() == 0)
    {
        ErrorLog("未找到有效的SVN地址");
        throw CTrsExp(ERR_NO_LOWER_LEVER,"该63料号未找到有效的SVN地址");
    }
    
    DebugLog("找到 %d 个SVN地址", svnList.size());

    //返回基本参数
    pResData->SetPara("order",inMap["order"]);
    pResData->SetPara("model_name",inMap["model_name"]);
    pResData->SetPara("count",inMap["count"]);
    pResData->SetPara("materi_63",inMap["materi_63"]);
    pResData->SetPara("pid_start",outMap["ass_pid"]);
    pResData->SetPara("ret_num", to_string(svnList.size()));
    
    //返回SVN地址数组
    for(size_t i = 0; i < svnList.size(); ++i)
    {
        pResData->SetArray(svnList[i]);
    }

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

string CQueryMaterialGetSvn::buildJsonStringForLoadBom(const string& materialCode)
{
    // 构建 loadBomByMaterialCode API 的请求JSON
    return R"({"materialCode":")" + materialCode + R"("})";
}

void CQueryMaterialGetSvn::parseBomJsonForSVN(const std::string& jsonString, vector<CStr2Map>& svnList)
{
    // 清空输出容器
    svnList.clear();
    
    // 解析JSON字符串
    rapidjson::Document doc;
    rapidjson::ParseResult ok = doc.Parse(jsonString.c_str());
    
    // 检查JSON解析是否成功
    if (!ok) 
    {
        ErrorLog("JSON parse error at offset %u", (unsigned)ok.Offset());
        return;
    }
    
    // 检查是否为对象类型
    if (!doc.IsObject()) {
        ErrorLog("JSON root is not an object");
        return;
    }
    
    // 检查result字段
    if (doc.HasMember("result") && doc["result"].IsBool()) 
    {
        if(!doc["result"].GetBool())
        {
            ErrorLog("result is false");
            return;
        }
    }
    
    // 检查code字段
    if (doc.HasMember("code") && doc["code"].IsInt()) {
        if(doc["code"].GetInt() != 200)
        {
            ErrorLog("error,code is not 200, code=%d", doc["code"].GetInt());
            return;
        }
    }
    
    // 检查data字段是否存在
    if (!doc.HasMember("data") || !doc["data"].IsObject()) 
    {
        ErrorLog("JSON does not contain a valid 'data' object");
        return;
    }
    
    // 递归遍历BOM树，提取所有有效的SVN地址
    const rapidjson::Value& dataObj = doc["data"];
    extractSVNFromBomNode(dataObj, svnList);
}

void CQueryMaterialGetSvn::extractSVNFromBomNode(const rapidjson::Value& node, vector<CStr2Map>& svnList)
{
    if (!node.IsObject()) {
        return;
    }
    
    // 检查当前节点是否有material字段，且attachment字段有效
    if (node.HasMember("material") && node["material"].IsObject())
    {
        const rapidjson::Value& material = node["material"];
        if (material.HasMember("attachment") && material["attachment"].IsString())
        {
            string attachment = material["attachment"].GetString();
            // 去除首尾空格
            size_t start = attachment.find_first_not_of(" \t\n\r");
            if (start != string::npos)
            {
                size_t end = attachment.find_last_not_of(" \t\n\r");
                attachment = attachment.substr(start, end - start + 1);
            }
            
            // 如果attachment不为空，且包含svn地址（以http://或https://开头）
            if (!attachment.empty() && 
                (attachment.find("http://") == 0 || attachment.find("https://") == 0))
            {
                CStr2Map svnMap;
                svnMap["svn_path"] = attachment;
                
                // 提取料号信息
                if (material.HasMember("code") && material["code"].IsString())
                {
                    svnMap["material_code"] = material["code"].GetString();
                }
                if (material.HasMember("name") && material["name"].IsString())
                {
                    svnMap["material_name"] = material["name"].GetString();
                }
                
                // 提取当前节点的code（BOM版本号）
                if (node.HasMember("code") && node["code"].IsString())
                {
                    svnMap["bom_code"] = node["code"].GetString();
                }
                
                svnList.push_back(svnMap);
                DebugLog("找到SVN地址: %s", attachment.c_str());
            }
        }
    }
    
    // 递归处理子节点
    if (node.HasMember("childs") && node["childs"].IsArray())
    {
        const rapidjson::Value& childs = node["childs"];
        for (rapidjson::SizeType i = 0; i < childs.Size(); ++i)
        {
            extractSVNFromBomNode(childs[i], svnList);
        }
    }
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