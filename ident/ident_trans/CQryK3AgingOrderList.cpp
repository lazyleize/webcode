#include "CQryK3AgingOrderList.h"
#include "CIdentRelayApi.h"
#include <iostream>
#include <string>

#undef max
#undef min
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include "rapidjson/stringbuffer.h"

std::string createJson(int startRow, int limit) {
    // 构建 JSON 字符串
    std::string jsonString = "{\"formId\":\"PRD_INSTOCK\",\"deal\":\"EXECUTE_BILL_QUERY\",\"data\":{\"FormId\":\"PRD_INSTOCK\",\"StartRow\":"
        + std::to_string(startRow) + ",\"Limit\":"
        + std::to_string(limit) + ",\"FieldKeys\":\"FDate,FRealQty,FMaterialID.F_BHR_Classific,FMaterialId.F_BHR_Model,FSrcBillNo,FMaterialId.FNumber\",\"FilterString\":[{\"Left\":\"\",\"FieldName\":\"FDate\",\"Compare\":\"265\",\"Value\":\"30\",\"Right\":\"\",\"Logic\":0},{\"Left\":\"\",\"FieldName\":\"FStockId.FName\",\"Compare\":\"67\",\"Value\":\"老化仓\",\"Right\":\"\",\"Logic\":0},{\"Left\":\"\",\"FieldName\":\"FMaterialId.FNumber\",\"Compare\":\"60\",\"Value\":\"62\",\"Right\":\"\",\"Logic\":0},{\"Left\":\"\",\"FieldName\":\"FDocumentStatus\",\"Compare\":\"105\",\"Value\":\"C\",\"Right\":\"\",\"Logic\":0},{\"Left\":\"\",\"FieldName\":\"FStockOrgId.FNumber\",\"Compare\":\"67\",\"Value\":\"1100\",\"Right\":\"\",\"Logic\":0}]}}";

    return jsonString;
}

int CQryK3AgingOrderList::IdentCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map sessMap;
	CStr2Map inMap, outMap;
	pReqData->GetStrMap(inMap);

	this->CheckLogin(sessMap);

	int limit = atol(inMap["limit"].c_str());
	int offset = atol(inMap["offset"].c_str());
    outMap["offset"] = inMap["offset"];

	offset = (offset -1)*limit;
    inMap["limit"] = Tools::IntToStr(limit);
	inMap["offset"] = Tools::IntToStr(offset);

	inMap["usertype"] = sessMap["usertype"];
    int usertype = atoi(sessMap["usertype"].c_str());
    /*if(usertype != 2  && usertype != 1)
    {
        ErrorLog("普通用户没有获取星空订单列表权限: usertype=[%s]" ,inMap["usertype"].c_str());
        throw CTrsExp(ERR_NO_LOWER_LEVER,"没有权限");
    }*/

    string strUrl = g_k3ErpCfg.strErpUrl;
    strUrl += "/oauth-service/oauth/token";

    string strOutToken;

    ErrorLog("strUrl[%s],strErpName[%s],strErpPasswd[%s]: usertype=[%s]" ,strUrl.c_str(),g_k3ErpCfg.strErpName.c_str(),g_k3ErpCfg.strErpPasswd.c_str());
    if(CIdentPub::HttpERPtoken(strUrl,g_k3ErpCfg.strErpName,g_k3ErpCfg.strErpPasswd,strOutToken) == -1)
    {
        ErrorLog("http请求ERP错误");
        throw CTrsExp(ERR_NO_LOWER_LEVER,"请求ERP错误");
    }
    DebugLog("token=[%s]",strOutToken.c_str());

    //再去调用订单列表
    string getOrderUrl =  g_k3ErpCfg.strErpUrl;
    getOrderUrl += "/gcs/erp/api/loadInvokeErpWebApi"; 
    string strResponse;
    string strqu = createJson(offset,limit);
    DebugLog("strqu=[%s]",strqu.c_str());
    if(CIdentPub::HttpPostERP(getOrderUrl,createJson(offset,limit),strResponse,strOutToken))
    {
        ErrorLog("http请求ERP错误");
        throw CTrsExp(ERR_NO_LOWER_LEVER,"请求ERP错误");
    }

    DebugLog("strResponse=[%s]",strResponse.c_str());
    vector<CStr2Map> vectmapArray;
    parseOrderJson(strResponse,vectmapArray);

    pResData->SetPara("ret_num",to_string(vectmapArray.size()));
    pResData->SetPara("offset",outMap["offset"]);

    for(size_t i = 0;i < vectmapArray.size();++i)
    {
        CStr2Map returnMap;
        CIdentComm::DelMapF(vectmapArray[i],returnMap);
        pResData->SetArray(returnMap);
    }

	return 0;
}


void CQryK3AgingOrderList::parseOrderJson(const std::string& jsonString, std::vector<CStr2Map>& vectmapArray)
{
    // 清空输出容器
    vectmapArray.clear();
    
    // 解析JSON字符串
    rapidjson::Document doc;
    rapidjson::ParseResult ok = doc.Parse(jsonString.c_str());
    
    // 检查JSON解析是否成功
    if (!ok) {
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
    
    // 遍历外层数组中的每个子数组
    for (rapidjson::SizeType i = 0; i < dataArray.Size(); ++i) 
    {
        
        const rapidjson::Value& itemArray = dataArray[i];
        CStr2Map mapEntry;
        
        // 解析子数组中的元素（按索引位置）
        // 确保子数组有足够的元素
        if (itemArray.Size() >= 6) 
        {
            // 索引0: 创建时间（字符串）
            if (itemArray[0].IsString())
                mapEntry["Fcreate"] = itemArray[0].GetString();
            else 
                ErrorLog("Invalid type for Fcreate at index ");
            
            // 索引1: 数量（数字）
            if (itemArray[1].IsNumber()) 
            {
                int count = static_cast<int>(itemArray[1].GetDouble());
                mapEntry["Fcount"] = std::to_string(count);
            }
            else 
                ErrorLog("Invalid type for Fcount at index ");
            
            
            // 索引2: 模型名称（字符串）
            if (itemArray[2].IsString()) 
                mapEntry["Fmodel_name"] = itemArray[2].GetString();
            else 
                ErrorLog("Invalid type for Fmodel_name at index ");
            
            
            // 索引3: 订单（字符串）
            if (itemArray[3].IsString()) 
                mapEntry["Fmodel"] = itemArray[3].GetString();
            else 
                ErrorLog("Invalid type for Forder at index " );
            
            
            // 索引4: 未知字段（字符串）
            if (itemArray[4].IsString()) 
                mapEntry["Forder"] = itemArray[4].GetString();
            else 
                ErrorLog("Invalid type for Funknown at index ");
            

            
            // 索引5: 材料编号（字符串）
            if (itemArray[5].IsString()) 
                mapEntry["Fmateri_62"] = itemArray[5].GetString();
            else 
                ErrorLog("Invalid type for Fmateri_62 at index ");
            
        } 
        else 
        {
            ErrorLog("Insufficient elements in subarray at index ");
            continue;
        }
        
        vectmapArray.push_back(mapEntry);
    }
}
