#include "CMaterialReverseCheckList.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"

#undef max
#undef min
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include "rapidjson/stringbuffer.h"

int CMaterialReverseCheckList::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,outMap,sessionMap;
	string uid;

	// 取得请求参数
    pReqData->GetStrMap(inMap);
	
    this->CheckLogin(sessionMap);

	//检查输入参数
	CheckParameter(inMap);

    int limit = atol(inMap["limit"].c_str());
	int offset = atol(inMap["offset"].c_str());

	offset = (offset -1)*limit;
    inMap["limit"] = Tools::IntToStr(limit);
	inMap["offset"] = Tools::IntToStr(offset);

    string strUrl = g_k3ErpCfg.strErpUrl;
    strUrl += "/oauth-service/oauth/token";

    string strOutToken;
    if(CIdentPub::HttpERPtoken(strUrl, g_k3ErpCfg.strErpName,g_k3ErpCfg.strErpPasswd,strOutToken) == -1)
    {
        ErrorLog("http请求ERP错误");
        throw CTrsExp(ERR_NO_LOWER_LEVER,"请求ERP错误");
    }
    DebugLog("token=[%s]",strOutToken.c_str());

    //再去调用订单列表
    string getOrderUrl =  g_k3ErpCfg.strErpUrl;
    getOrderUrl += "/gcs/erp/api/loadInvokeErpWebApi"; 
    string strResponse;
    if(CIdentPub::HttpPostERP(getOrderUrl,buildJsonString(offset,limit,inMap["materi_62"]),strResponse,strOutToken))
    {
        ErrorLog("http请求ERP错误");
        throw CTrsExp(ERR_NO_LOWER_LEVER,"请求ERP错误");
    }

    vector<CStr2Map> vectmapArray;
    parseMaterialJson(strResponse,vectmapArray);

    pResData->SetPara("order",inMap["order"]);
    pResData->SetPara("model_name",inMap["model_name"]);
    pResData->SetPara("count",inMap["count"]);
    pResData->SetPara("ret_num",to_string(vectmapArray.size()));

    //返回所有63料号
    for(size_t i = 0;i < vectmapArray.size();++i)
    {
        CStr2Map returnMap;
        CIdentComm::DelMapF(vectmapArray[i],returnMap);
        pResData->SetArray(returnMap);
    }

	return 0;
}

// 输入判断
void CMaterialReverseCheckList::CheckParameter( CStr2Map& inMap)
{
	if(inMap["materi_62"].empty())
    {
        ErrorLog("关键字段不能为空-materi_62");
        throw CTrsExp(ERR_SIGNATURE_INCORRECT,"关键字段materi_62为空");
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

string CMaterialReverseCheckList::buildJsonString(int startRow, int limit,const std::string& materialChildValue)
{
    return R"({
        "formId": "ENG_BOM",
        "deal": "EXECUTE_BILL_QUERY",
        "data": {
           "FormId": "ENG_BOM",
           "FieldKeys": "FMATERIALID.FNumber,FMATERIALID.FName",
           "StartRow": )" + std::to_string(startRow) + R"(,
           "Limit": )" + std::to_string(limit) + R"(,
           "FilterString": [
           {"Left":"","FieldName":"FMATERIALIDCHILD.FNumber","Compare":"17","Value":")" + materialChildValue + R"(","Right":"","Logic":0},
           {"Left":"","FieldName":"FMATERIALID.FNumber","Compare":"60","Value":"63","Right":"","Logic":0},
           {"Left":"","FieldName":"FDocumentStatus","Compare":"105","Value":"C","Right":"","Logic":0},
           {"Left":"","FieldName":"FForbidStatus","Compare":"105","Value":"A","Right":"","Logic":0},
           {"Left":"","FieldName":"FUseOrgId.FNumber","Compare":"67","Value":"1100","Right":"","Logic":0}
           ]
       }
       })";
}


//63查询bom
string CMaterialReverseCheckList::buildJsonStringWithValue(const std::string& materialValue)
{
    // 使用C++11的原始字符串字面量(Raw string literal)可以避免大量转义字符
    return R"({
        "formId": "ENG_BOM",
        "deal": "EXECUTE_BILL_QUERY",
        "data": {
            "FormId": "ENG_BOM",
            "FieldKeys": "FMATERIALIDCHILD.FNumber,FMATERIALIDCHILD.FName",
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

void CMaterialReverseCheckList::parseMaterialJson(const std::string& jsonString, vector<CStr2Map>& vectmapArray)
{
    // 清空输出容器
    vectmapArray.clear();
    
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
    
    if (doc.HasMember("code") && doc["code"].IsInt()) 
    {
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
    /*if(dataArray.Size() > 1)
    {
        ErrorLog("多个63料号,失败");
        return;
    }*/
    
    // 遍历外层数组中的每个子数组
    for (rapidjson::SizeType i = 0; i < dataArray.Size(); ++i) 
    {
        
        const rapidjson::Value& itemArray = dataArray[i];
        CStr2Map mapEntry;
        
        // 解析子数组中的元素（按索引位置）
        // 确保子数组有足够的元素
        if (itemArray.Size() >= 2) 
        {
            // 索引0: 创建时间（字符串）
            if (itemArray[0].IsString())
                mapEntry["Fmateri_63"] = itemArray[0].GetString();
            else 
                ErrorLog("Invalid type for Fcreate at index ");
            
            // 索引2: 模型名称（字符串）
            if (itemArray[1].IsString()) 
                mapEntry["Fname"] = itemArray[1].GetString();
            else 
                ErrorLog("Invalid type for Fmodel_name at index "); 
            
        } 
        else 
        {
            ErrorLog("Insufficient elements in subarray at index ");
            continue;
        }
        vectmapArray.push_back(mapEntry);
    }
}
