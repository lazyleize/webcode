/************************************************************
 Desc:     根据固件包唯一id去SVN获取下载包，并同步到共享路径
 Auth:     leize
 Modify:
 data:     2018-08-02
 ***********************************************************/

#include "CIdentAppComm.h"
#include "CGetFirmPack.h"

CIdentAppComm* pIdentAppComm = NULL;

int main(int argc, char** argv)
{
    //初始化
	CStr2Map inMap,outMap;
	char date[8+1]={"\0"};
    if (3 > argc)
    {
        printf("Need to input firmware package ID\n");
        return 1;
    }
	
	if (3 == argc)//获取固件包ID
    {
		char pack_id[16+1]={"\0"};
        strcpy(pack_id, argv[2]);
        inMap["pack_id"] = pack_id;
    }
	
    //设置配置文件路径
    CAppInitEnv initEnv;
    initEnv.conf_path = string(argv[1]);
    initEnv.app_name = "app_GetSVN_Firmware";

    pIdentAppComm = new CIdentAppComm();

    if(NULL == pIdentAppComm)
    {
        //打印日志
        XdErrorLog("进程[%s]初始化异常",initEnv.app_name.c_str());
        return -1;
    }
    pIdentAppComm->InitEnv(initEnv);
    CGetFirmPack GetFirmPack;
   
    XdInfoLog("进程[%s]执行开始BEGIN ", pIdentAppComm->GetAppName().c_str());
    try
    {
        GetFirmPack.Commit(inMap, outMap);
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
