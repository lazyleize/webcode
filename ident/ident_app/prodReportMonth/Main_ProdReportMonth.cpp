/************************************************************
 Desc:     每月1号凌晨1点生成《工厂生产流程报表(月)》
 Auth:     leize
 Modify:
 data:     2025-07-02
 ***********************************************************/

#include "CIdentAppComm.h"
#include "CProdReportMonth.h"

CIdentAppComm* pIdentAppComm = NULL;

int main(int argc, char** argv)
{
    //初始化
	CStr2Map inMap,outMap;
	char date[8+1]={"\0"};
    if (3 > argc)
    {
        printf("Need to input YYYYMM\n");
        return 1;
    }
	
	if (3 == argc)//获取时间
    {
		char build_date[16+1]={"\0"};
        strcpy(build_date, argv[2]);
        inMap["build_date"] = build_date;
    }
	
    //设置配置文件路径
    CAppInitEnv initEnv;
    initEnv.conf_path = string(argv[1]);
    initEnv.app_name = "app_ProdReportMonth";

    pIdentAppComm = new CIdentAppComm();

    if(NULL == pIdentAppComm)
    {
        //打印日志
        XdErrorLog("进程[%s]初始化异常",initEnv.app_name.c_str());
        return -1;
    }
    pIdentAppComm->InitEnv(initEnv);
    CProdReportMonth ProdReportMonth;
   
    XdInfoLog("进程[%s]执行开始BEGIN ", pIdentAppComm->GetAppName().c_str());
    try
    {
        ProdReportMonth.Commit(inMap, outMap);
    } 
    catch (CTrsExp & e)
    {
        XdErrorLog("进程[%s]执行异常 errcode=[%s]errmsg[%s] ",
        pIdentAppComm->GetAppName().c_str(), e.retcode.c_str(),e.retmsg.c_str());//捕获异常，打印日志
    }
    XdInfoLog("进程[%s]执行完毕END",pIdentAppComm->GetAppName().c_str());
    delete pIdentAppComm;
    return 0;

}
