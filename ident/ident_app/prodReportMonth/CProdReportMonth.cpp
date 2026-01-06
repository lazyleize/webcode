/************************************************************
 Desc:     每月1号凌晨1点生成《工厂生产流程报表(月)》
 Auth:     leize
 Modify:
 data:     2025-07-8
 ***********************************************************/
#include "adstcp.h"
#include "CProdReportMonth.h"
#include "CIdentRelayApi.h"
#include "CTools.h"
#include "CSmtp.h"

// 取消宏定义，避免冲突
#undef max
#undef min
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <base/all.hpp>

using namespace aps;

extern CIdentAppComm* pIdentAppComm;

CProdReportMonth::CProdReportMonth()
{
}
CProdReportMonth::~CProdReportMonth()
{
}

int CProdReportMonth::Commit(CStr2Map &inMap, CStr2Map &outMap)
{
    // 获取本地存储路径和 Python 脚本路径
    CStr2Map qryInMap, InsertInMap, InsertOutMap;
    char savePath[256] = {0};
    char pyPath[256] = {0};
    char pyYearPath[256] = {0};
    char pyCreatePath[256] = {0};
    char mothModelPath[256] = {0};
    char yearModelPath[256] = {0};
    string message,strType;

    // 从全局变量中获取路径
    memcpy(savePath, g_mTransactions[GetTid()].m_mVars["localbasepath"].c_str(), g_mTransactions[GetTid()].m_mVars["localbasepath"].length());
    memcpy(pyPath, g_mTransactions[GetTid()].m_mVars["pythonpwd"].c_str(), g_mTransactions[GetTid()].m_mVars["pythonpwd"].length());
    memcpy(pyYearPath, g_mTransactions[GetTid()].m_mVars["pythonYear"].c_str(), g_mTransactions[GetTid()].m_mVars["pythonYear"].length());
    memcpy(pyCreatePath, g_mTransactions[GetTid()].m_mVars["pythonCreateYear"].c_str(), g_mTransactions[GetTid()].m_mVars["pythonCreateYear"].length());
    memcpy(mothModelPath, g_mTransactions[GetTid()].m_mVars["mothModel"].c_str(), g_mTransactions[GetTid()].m_mVars["mothModel"].length());
    memcpy(yearModelPath, g_mTransactions[GetTid()].m_mVars["yearModel"].c_str(), g_mTransactions[GetTid()].m_mVars["yearModel"].length());

    string strSavePath = savePath;

    // 生成路径，基础路径 + YYYYMM/MonthReport.txt
    string strMonthPath = strSavePath + "/" + inMap["build_date"];
    Directory::mkdir(strMonthPath);
    strMonthPath += "/MonthReport.txt";

    // 拼保存路径和文件名称
    string strSaveMothExcel = strSavePath + "/" + inMap["build_date"] + "/" + inMap["build_date"] + "-mothReport.xlsx";
    string strSaveYearExcel = strSavePath + "/" + inMap["build_date"] + "/" + inMap["build_date"] + "-yearReport.xlsx";

    InfoLog("查询生产流程统计记录表是否已生成 build_date =[%s]", inMap["build_date"].c_str());
    qryInMap["date"] = inMap["build_date"];  // 包ID
    qryInMap["offset"] = "0";
    qryInMap["limit"] = "10"; 

    vector<CStr2Map> vectmapArray;
    vectmapArray.clear();

    // 从生产流程统计记录表查询是否已生成
    CIdentRelayApi::QueryReportRecordList(qryInMap, outMap, vectmapArray, false);
    InfoLog("生产流程统计记录表数据 vect-size=[%d]", vectmapArray.size());

    int retNum = atoi(outMap["ret_num"].c_str());
    if (retNum > 2)
    {
        ErrorLog("数据异常, 人工介入 date = [%s]", inMap["build_date"].c_str());
        return 0;
    }

    if (retNum == 2)
    {
        InfoLog("已经生成");
        return 0;
    }
    string buildDateBeg = inMap["build_date"] + "01";
    Date m_dateBeg;
    m_dateBeg.setDate(buildDateBeg);
    string buildDateEnd = inMap["build_date"] + to_string(m_dateBeg.getDaysOfMonth(m_dateBeg.getMonth()));
    
    // 构造message格式：2025年09月01日至2025年09月30日的
    string yearBeg = buildDateBeg.substr(0, 4);
    string monthBeg = buildDateBeg.substr(4, 2);
    string dayBeg = buildDateBeg.substr(6, 2);
    
    string yearEnd = buildDateEnd.substr(0, 4);
    string monthEnd = buildDateEnd.substr(4, 2);
    string dayEnd = buildDateEnd.substr(6, 2);
    
    message = yearBeg + "年" + monthBeg + "月" + dayBeg + "日至" + yearEnd + "年" + monthEnd + "月" + dayEnd + "日的";
    
    // 需要判断是哪个报表没生成，然后去生成
    if (retNum == 1)
    {
        if (vectmapArray[1]["Ftype"] == "M")
        {
            if (!GenerateMonthReport(inMap, strMonthPath))
            {
                //2025年08月08日的16:32:40, 生成生产月统计报表异常。
                strType = "ERROR";
                message += "工厂生产月统计报表异常。";
                setMessageList(message, strType);
                ErrorLog("生成生产月统计报表异常 strMonthPath= [%s]", strMonthPath.c_str());
                return 0;
            }

            char cmd[512];
            snprintf(cmd, sizeof(cmd), "python3 %s %s %s %s %s > /dev/null 2>&1", 
                     pyPath, strMonthPath.c_str(), mothModelPath, strSaveMothExcel.c_str(), inMap["build_date"].c_str());
            InfoLog(": cmd=[%s]", cmd);

            if (std::system(cmd) != 0)
            {
                ErrorLog("月度报表生成异常, cmd = [%s]", cmd);
                strType = "ERROR";
                message += "工厂生产月统计报表异常。";
                setMessageList(message, strType);
                return 0;
            }

            // 写入 t_report_record，报表生成记录表
            CStr2Map InsertInMap, InsertOutMap;
            InsertInMap["date"] = inMap["build_date"];
            InsertInMap["path"] = strSaveMothExcel;
            InsertInMap["type"] = "M";
            CIdentRelayApi::InsertReportRecord(InsertInMap, InsertOutMap, false);

            strType = "NOTICE";
            message += "《工厂生产流程报表(月度)》已生成";
            setMessageList(message, strType);
        }
        else if (vectmapArray[0]["Ftype"] == "Y")
        {
            if (!YearReport(inMap, strSavePath))
            {
                ErrorLog("生成生产年报表异常 StrsavePath= [%s]", strSavePath.c_str());
                strType = "ERROR";
                message += "工厂生产年统计报表异常。";
                setMessageList(message, strType);
                return 0;
            }

            char cmd[512];
            snprintf(cmd, sizeof(cmd), "python3 %s %s %s %s %s > /dev/null 2>&1", 
                     pyYearPath, strMonthPath.c_str(), yearModelPath, strSaveYearExcel.c_str(), inMap["build_date"].c_str());
            InfoLog(": cmd=[%s]", cmd);

            if (std::system(cmd) != 0)
            {
                ErrorLog("年度报表生成异常, cmd = [%s]", cmd);
                strType = "ERROR";
                message += "工厂生产年统计报表异常。";
                setMessageList(message, strType);
                return 0;
            }

            // 看是否是12月份报表，是就汇总，生成新模板
            string strMonth = inMap["build_date"].substr(4, 2);
            InfoLog("传入月份 strMonth=[%s]", strMonth.c_str());
            if (strMonth == "12")
            {
                snprintf(cmd, sizeof(cmd), "python3 %s %s %s > /dev/null 2>&1", 
                         pyCreatePath, yearModelPath, inMap["build_date"].c_str());
                InfoLog(": cmd=[%s]", cmd);
                if (std::system(cmd) != 0)
                {
                    ErrorLog("创建年报表模板异常, cmd = [%s]", cmd);
                    strType = "ERROR";
                    message += "创建年报表模板异常。";
                    setMessageList(message, strType);
                    return 0;
                }
            }

            // 写入 t_report_record，报表生成记录表
            InsertInMap["date"] = inMap["build_date"];
            InsertInMap["path"] = strSaveYearExcel;
            InsertInMap["type"] = "Y";
            CIdentRelayApi::InsertReportRecord(InsertInMap, InsertOutMap, false);

            strType = "NOTICE";
            message += "《工厂生产流程报表(总表)》已生成";
            setMessageList(message, strType);
        }
    }
    else//一个都没生成
    {
        if (!GenerateMonthReport(inMap, strMonthPath))
        {
            //2025年08月08日的16:32:40, 生成生产月统计报表异常。
            strType = "ERROR";
            message += "工厂生产月统计报表异常。";
            setMessageList(message, strType);
            ErrorLog("生成生产月统计报表异常 strMonthPath= [%s]", strMonthPath.c_str());
            return 0;
        }
    }
    
    // 调用 Python 生成 Excel
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "python3 %s %s %s %s %s > /dev/null 2>&1", 
             pyPath, strMonthPath.c_str(), mothModelPath, strSaveMothExcel.c_str(), inMap["build_date"].c_str());
    InfoLog(": cmd=[%s]", cmd);

    if (std::system(cmd) != 0)
    {
        ErrorLog("月度报表生成异常, cmd = [%s]", cmd);
        strType = "ERROR";
        message += "生成生产月统计报表异常。";
        setMessageList(message, strType);
        
        return 0;
    }

    // 写入 t_report_record，报表生成记录表
    
    InsertInMap["date"] = inMap["build_date"];
    InsertInMap["path"] = strSaveMothExcel;
    InsertInMap["type"] = "M";
    CIdentRelayApi::InsertReportRecord(InsertInMap, InsertOutMap, false);

    strType = "NOTICE";
    message += "《工厂生产流程报表(月度)》已生成";
    setMessageList(message, strType);

    // 生成年度报表
    if (!YearReport(inMap, strSavePath))
    {
        ErrorLog("生成年报表异常 StrsavePath= [%s]", strSavePath.c_str());
        strType = "ERROR";
        message += "生成生产年报表异常。";
        setMessageList(message, strType);
        return 0;
    }

    snprintf(cmd, sizeof(cmd), "python3 %s %s %s %s %s > /dev/null 2>&1", 
             pyYearPath, strMonthPath.c_str(), yearModelPath, strSaveYearExcel.c_str(), inMap["build_date"].c_str());
    InfoLog(": cmd=[%s]", cmd);

    if (std::system(cmd) != 0)
    {
        ErrorLog("年度报表生成异常, cmd = [%s]", cmd);
        strType = "ERROR";
        message += "生成生产年报表异常。";
        setMessageList(message, strType);
        return 0;   
    }
    strType = "NOTICE";
    message += "《工厂生产流程报表(总表)》已生成";
    setMessageList(message, strType);

    // 看是否是12月份报表，是就汇总，生成新模板
    string strMonth = inMap["build_date"].substr(4, 2);
    InfoLog("传入月份 strMonth=[%s]", strMonth.c_str());
    if (strMonth == "12")
    {
        snprintf(cmd, sizeof(cmd), "python3 %s %s %s > /dev/null 2>&1", 
                 pyCreatePath, yearModelPath, inMap["build_date"].c_str());
        InfoLog(": cmd=[%s]", cmd);
        if (std::system(cmd) != 0)
        {
            ErrorLog("创建年报表模板异常, cmd = [%s]", cmd);
            //return 0;
        }
    }

    // 写入 t_report_record，报表生成记录表
    InsertInMap["date"] = inMap["build_date"];
    InsertInMap["path"] = strSaveYearExcel;
    InsertInMap["type"] = "Y";
    CIdentRelayApi::InsertReportRecord(InsertInMap, InsertOutMap, false);
    

    return 0;
}

//月报表生成
bool CProdReportMonth::GenerateMonthReport(CStr2Map& inMap,string& strOutPath)
{
    //去生产日志记录表查询工厂列表
    CStr2Map facInMap,facOutMap,modelInmap,modelOutmap;
    //算出时间范围
    Date m_date(atoi(inMap["build_date"].substr(0,4).c_str()),atoi(inMap["build_date"].substr(4,2).c_str()),1);
    Time m_time(0,0,0,0);
    Datetime m_datetime(m_date,m_time);
    facInMap["create_time_beg"] = m_datetime.toString();
    m_datetime.addMonths(1);
    facInMap["create_time_end"] = (m_datetime-=1).toString();

    facInMap["offset"] = "0";
    facInMap["limit"]  = "10"; 
    vector<CStr2Map> vectmapArray;

    vectmapArray.clear();
    CIdentRelayApi::QueryProductFactoryList(facInMap,facOutMap,vectmapArray,false);
    InfoLog("生产日志记录表查询工厂列表个数 vect-size=[%d]",vectmapArray.size());
    if(atoi(facOutMap["ret_num"].c_str()) ==0)
        return false;

    for(size_t i = 0;i < vectmapArray.size();i++) 
    {
        CStr2Map tmpMap,facTmpMap;
        CIdentPub::DelMapF(vectmapArray[i],tmpMap);
        //批量查询该工厂下的机型，最多50个，
        modelInmap["factory_id"] = tmpMap["actory_id"];
        modelInmap["offset"] = "0";
        modelInmap["limit"]  = "50"; 
        modelInmap["create_time_beg"]  = facInMap["create_time_beg"];
        modelInmap["create_time_end"]  = facInMap["create_time_end"];

        facTmpMap["工厂名称"] = tmpMap["actory"] ;
        facTmpMap["工厂序号"] = to_string(i+1);
        m_factoryData.push_back(facTmpMap);


        vector<CStr2Map> vectModel;
        vectModel.clear();
        CIdentRelayApi::QueryProductModelList(modelInmap,modelOutmap,vectModel,false);
        InfoLog("[%s]工厂下有[%d]个不同机型",tmpMap["actory"].c_str(),vectModel.size());
        for(size_t j = 0;j < vectModel.size();j++)
        {
            CStr2Map tmp2Map,resultMap;
            CIdentPub::DelMapF(vectModel[j],tmp2Map);
            //根据机型还有工厂，去统计 数量(成功，且SN号不同的个数)、生产次数、失败个数;返回2种，一个组装、一个包装的数据
            CStr2Map dataInMap,dataOutMap;
            dataInMap["create_time_beg"] = facInMap["create_time_beg"];
            dataInMap["create_time_end"] = facInMap["create_time_end"];
            dataInMap["factory_id"]      = modelInmap["factory_id"];
            dataInMap["pos_name"]        = tmp2Map["pos_name"];

            CIdentRelayApi::ProductMonthReport(dataInMap,dataOutMap,false);

            resultMap["机型"]             = dataInMap["pos_name"];
            resultMap["组装批次号个数"]   = dataOutMap["asspid_batch_num"];
            resultMap["组装成功个数"]     = dataOutMap["asspid_count"];
            resultMap["组装失败个数"]     = dataOutMap["asspid_fail_total"];
            resultMap["组装请求个数"]     = to_string(atol(dataOutMap["asspid_count"].c_str())+atol(dataOutMap["asspid_fail_total"].c_str()));

            resultMap["包装批次号个数"]   = dataOutMap["pack_batch_num"];
            resultMap["包装失败个数"]     = dataOutMap["pack_fail_total"];
            resultMap["包装成功个数"]     = dataOutMap["pack_success_count"];
            resultMap["包装请求个数"]     = to_string(atol(dataOutMap["pack_success_count"].c_str())+atol(dataOutMap["pack_fail_total"].c_str()));
            
            //组json数据
            SetArray(tmpMap["actory"] ,resultMap);
        }
        //PrintMutiArrayMap();
        OutputFileMutiArrayMap(strOutPath);
    }

    return true;
}

//年报表生成
bool CProdReportMonth::YearReport(CStr2Map& inMap,const string& StrBasePath)
{
    //查看本地有没有月度报表的数据，如果没有就生成，如果有直接调用python3
    string strMonthPath = StrBasePath + "/" ;
    strMonthPath += inMap["build_date"] ;
    Directory::mkdir(strMonthPath);
    strMonthPath += "/MonthReport.txt";

    if(!File::exists(strMonthPath)) //不存在，重新生成月度报表
    {
        if(!GenerateMonthReport(inMap,strMonthPath))
        {
            ErrorLog("生成生产月统计报表一异常 strMonthPath= [%s]", strMonthPath.c_str());
            return false;
        }
    }

    return true;
}

void CProdReportMonth::SetArray(const string& strArrayName,const string& paraName,const string& paraValue)
{
    CResData::MUTI_ARRAY_MAP::iterator iter = m_muti_array_map.find(strArrayName) ;
	if(iter == m_muti_array_map.end())
	{
		CResData::ARRAY_MAP *pNewArrayMap = new CResData::ARRAY_MAP ;
		m_muti_array_map[strArrayName] = pNewArrayMap ;
		CStrVector newCStrVector ;	
		newCStrVector.push_back(paraValue) ;
		(*pNewArrayMap)[paraName] = newCStrVector ;
	}
	else
	{
		CResData::ARRAY_MAP *pArrayMap = iter->second ;
		if(pArrayMap->find(paraName) == pArrayMap->end())
		{
			
			CStrVector newCStrVector ;	
			newCStrVector.push_back(paraValue) ;
			pArrayMap->insert(CResData::ARRAY_MAP::value_type(paraName,newCStrVector));
			
		}else
		{
			(*pArrayMap)[paraName].push_back(paraValue) ;
		}
		
	}
}

void CProdReportMonth::SetArray(const string& factoryName, const CStr2Map& inMap) 
{
	CStr2Map::const_iterator  iter 	 = inMap.begin();
	CStr2Map::const_iterator  iter_end  = inMap.end();
	while(iter != iter_end)
	{
		SetArray(factoryName,iter->first, iter->second) ;
		iter++ ;	
	}
}

void CProdReportMonth::OutputFileMutiArrayMap(const string& outputPath) 
{
    // 创建 JSON 文档对象
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

    // 遍历 m_muti_array_map
    for (const auto& factoryPair : m_muti_array_map)
    {
        const std::string& factoryName = factoryPair.first;
        const CResData::ARRAY_MAP* arrayMapPtr = factoryPair.second;

        if (arrayMapPtr)
        {
            const CResData::ARRAY_MAP& arrayMap = *arrayMapPtr;
            rapidjson::Value factoryData(rapidjson::kArrayType); // 创建工厂数据数组

            // 假设每个参数有相同数量的值
            size_t numValues = arrayMap.begin()->second.size();

            for (size_t i = 0; i < numValues; ++i)
            {
                rapidjson::Value dataPoint(rapidjson::kObjectType); // 创建数据点对象

                for (const auto& itemPair : arrayMap)
                {
                    const std::string& paraName = itemPair.first;
                    const CStrVector& values = itemPair.second;

                    // 添加参数和对应的值到数据点对象
                    if (i < values.size())
                    {
                        dataPoint.AddMember(rapidjson::StringRef(paraName.c_str()), 
                                            rapidjson::Value(values[i].c_str(), allocator), 
                                            allocator);
                    }
                    else
                    {
                        dataPoint.AddMember(rapidjson::StringRef(paraName.c_str()), 
                                            rapidjson::Value().SetString("", allocator), 
                                            allocator);
                    }
                }

                factoryData.PushBack(dataPoint, allocator); // 将数据点添加到工厂数据数组
            }

            document.AddMember(rapidjson::StringRef(factoryName.c_str()), factoryData, allocator); // 将工厂数据添加到文档
        }
    }

    // 写入到文件
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer); // 将文档写入到缓冲区

    std::ofstream file(outputPath);
    if (file.is_open())
    {
        file << buffer.GetString(); // 写入 JSON 字符串
        file.close();
        InfoLog("JSON 数据已成功写入到[%s]",outputPath.c_str());
    } else 
    {
        ErrorLog("File open error.");
        return ;
    }
}

void PrintIndented(const std::string& text, int level)
{
    std::cout << std::setw(level * 4) << "" << text << std::endl; // 根据层级缩进
}

void CProdReportMonth::PrintMutiArrayMap()
{
    PrintIndented("打印 m_muti_array_map 内容:", 0);

    for (const auto& factoryPair : m_muti_array_map) // 遍历工厂映射
    {
        const std::string& factoryName = factoryPair.first;
        const CResData::ARRAY_MAP* arrayMapPtr = factoryPair.second; // 获取指针

        PrintIndented("工厂名称: [" + factoryName + "]", 1);

        if (arrayMapPtr) // 检查指针是否有效
        {
            const CResData::ARRAY_MAP& arrayMap = *arrayMapPtr; // 解引用指针

            for (const auto& itemPair : arrayMap) // 遍历每个工厂的数组映射
            {
                const std::string& paraName = itemPair.first;
                const CStrVector& values = itemPair.second;

                PrintIndented("参数名称: [" + paraName + "]", 2);
                PrintIndented("值列表:", 2);

                for (const auto& value : values) // 遍历每个参数的值
                {
                    PrintIndented("[" + value + "]", 3);
                }
            }
        }
        else
        {
            PrintIndented("数组映射为空", 2);
        }
    }
    std::cout << "打印 完毕！" << std::endl;
}

void CProdReportMonth::setMessageList(const string& strMessage,const string& strType)
{
    CStr2Map InsertInMap, InsertOutMap;
    InsertInMap["message"] = strMessage;
    InsertInMap["type"] = strType;
    CIdentRelayApi::UpdateMessageList(InsertInMap, InsertOutMap, false);
}
