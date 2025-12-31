/************************************************************
 Desc:     每月1号凌晨1点生成《工厂生产流程报表(月)》 cmd:./app_ProdReportMonth /home/httpd/web/web_pay5/conf/app_conf.xml 202506
 Auth:     leize
 Modify:
 data:     2024-12-26
 ***********************************************************/
#ifndef CPRODREPORTMONTH_H_
#define CPRODREPORTMONTH_H_

#include <base/file.hpp>
#include <base/datetime.hpp>
#include "CIdentAppComm.h"
#include "xml/unicode.h"

class CProdReportMonth
{
    public:
    CProdReportMonth();
    virtual ~CProdReportMonth();
    int Commit(CStr2Map & inMap,CStr2Map &outMap);
    string GetTid(){
        return string("app_ProdReportMonth");
    };
 
    void DelMapF(CStr2Map& dataMap,CStr2Map& outMap);
    bool GenerateMonthReport(CStr2Map& inMap,string& strOutPath);
    bool YearReport(CStr2Map& inMap,const string& StrBasePath);

private:
   void SetPara(const string& paraName,const string& paraValue);

   void SetArray(const string& factoryName, const CStr2Map& resultMap) ;
   void SetArray(const string& strArrayName,const string& paraName,const string& paraValue);
   void OutputFileMutiArrayMap(const string& filePath) ;

   void PrintMutiArrayMap();
   void setMessageList(const string& strMessage,const string& strType);
private:
    CResData::MUTI_ARRAY_MAP m_muti_array_map ;
    vector<CStr2Map> m_factoryData ;
};

#endif 
