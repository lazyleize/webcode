#include "CQueryCollectFuncList.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"

// 取消宏定义，避免冲突
#undef max
#undef min
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson; 

int CQueryCollectFuncList::IdentCommit(CReqData *pReqData, CResData *pResData)
{
    CStr2Map sessMap;
	CStr2Map inMap, tmpMap,outMap;
	pReqData->GetStrMap(inMap);

	this->CheckLogin(sessMap);

	int limit = atol(inMap["limit"].c_str());
	int offset = atol(inMap["offset"].c_str());

	offset = (offset -1)*limit;
    inMap["limit"] = Tools::IntToStr(limit);
	inMap["offset"] = Tools::IntToStr(offset);

	inMap["usertype"] = sessMap["usertype"];
    int usertype = atoi(sessMap["usertype"].c_str());
    /*if(usertype == 2 || usertype == 1)
        inMap["factory_id"].clear();
	else
	    inMap["factory_id"] = sessMap["factoryid"];*/
	
    vector<CStr2Map> vectmapArray;

    CIdentRelayApi::QueryCollectPath(inMap,tmpMap,true);
    
    if(tmpMap["File_path"].empty())
    {
        ErrorLog("找不到文件路径: collect_id=[%s]" ,inMap["collect_id"].c_str());
        throw CTrsExp(ERR_NO_LOWER_LEVER,"找不到文件路径");
    }

    ifstream inFile(tmpMap["File_path"].c_str(), std::ios::in | std::ios::binary);
    if (!inFile)
	{
		ErrorLog("文件打开失败[%s]",tmpMap["File_path"].c_str());
		throw(CTrsExp(ERR_UPFILE_COUNT, "文件打开失败"));
		return 0;
	}

	std::ostringstream oss;
    oss << inFile.rdbuf(); // 读取整个文件内容到ostringstream中

    // 关闭文件
    inFile.close();
	string filecontext = oss.str();

    //解析JSON
    Document document;
	if(document.Parse(filecontext.c_str()).HasParseError())
	{
		ErrorLog("JSON解析失败",tmpMap["File_path"].c_str());
		throw(CTrsExp(ERR_UPFILE_COUNT, "文件打开失败"));
		return 0;
	}

	if(document.HasMember("timestamp") && document["timestamp"].IsString())
		inMap["strTime"] = document["timestamp"].GetString();

	if(document.HasMember("type") && document["type"].IsString())
		inMap["strType"] = document["type"].GetString();

	if(document.HasMember("result") && document["result"].IsString())
		inMap["strResult"] = document["result"].GetString();

    // 处理 funclist
    const rapidjson::Value& funclist = document["funclist"];
    if (funclist.IsArray())
    {
        pResData->SetPara("ret_num","1");
        pResData->SetPara("total",to_string(funclist.Size()));
        for (rapidjson::SizeType i = 0; i < funclist.Size(); i++)
        {
            CStr2Map returnMap;
            const rapidjson::Value& funcItem = funclist[i];
            returnMap["func"] = funcItem["func"].GetString();
            returnMap["description"]  = funcItem["description"].GetString();
            returnMap["funcResult"]  = funcItem["result"].GetString();

            //处理参数
            const rapidjson::Value& parameters = funcItem["parameters"];
            if (parameters.IsObject())
            {
                string parameter;
                for(rapidjson::Value::ConstMemberIterator itr = parameters.MemberBegin();itr != parameters.MemberEnd(); ++itr)
                {
                    parameter += itr->name.GetString();
                    parameter += ":";
                    parameter += itr->value.GetString();
                    parameter += " \r\n";
                }
                returnMap["parameters"]=parameter;
                                 
            }
            pResData->SetArray(returnMap);
        }
    }

	return 0;
}
