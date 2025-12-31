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
#include "curl/curl.h"
#include "xml/unicode.h"

class CFirmReport
{
    public:
    CFirmReport();
    virtual ~CFirmReport();
    int Commit(CStr2Map & inMap,CStr2Map &outMap);
    string GetTid(){
        return string("app_FirmReport");
    };

private:
    string CreateRport(CStr2Map & inMap,char* pbasepath);
};
#endif 
