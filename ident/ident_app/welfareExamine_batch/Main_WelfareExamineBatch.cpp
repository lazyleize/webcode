/************************************************************
 Desc:     根据t_gypay_cuc_trans流水表发起卡激活交易
 Auth:     leize
 Modify:
 data:     2018-08-02
 ***********************************************************/

#include "CIdentAppComm.h"
#include "CWelfareExamineBatch.h"

CIdentAppComm* pIdentAppComm = NULL;

int main(int argc, char** argv)
{
    /**
     * 环境初始化
     */
	CStr2Map inMap,outMap;
	char date[8+1]={"\0"};
    if (3 > argc)
    {
        printf("need conf_file_path,and svr_ip,reult,YYYYMMDD\n");
        return 1;
    }
	
	if (4 == argc)
    {
		char local_date[8+1]={"\0"};
        ads_getdate(local_date);
        ads_div_day(local_date,-1,date);
    }
    if (5 == argc)
    {
		memcpy(date,argv[4],8);
    }
	
    //设置配置文件路径
    CAppInitEnv initEnv;
    initEnv.conf_path = string(argv[1]);
    initEnv.svr_ip = string(argv[2]);
    initEnv.app_name = "welfare_examine_batch";

	inMap["batch_id"] = string(argv[3]);//交易状态
    pIdentAppComm = new CIdentAppComm();

    if(NULL == pIdentAppComm)
    {
            //打印日志
            XdErrorLog("进程[%s]初始化异常",initEnv.app_name.c_str());
            return -1;
    }
    pIdentAppComm->InitEnv(initEnv);
    CWelfareExamineBatch WelfareExamine;
   
    XdInfoLog("进程[%s]执行开始BEGIN ", pIdentAppComm->GetAppName().c_str());
    try
    {
        WelfareExamine.Commit(inMap, outMap);
    } catch (CTrsExp & e)
      {
          XdErrorLog("进程[%s]执行异常 errcode=[%s]errmsg[%s] ",
          pIdentAppComm->GetAppName().c_str(), e.retcode.c_str(),e.retmsg.c_str());//捕获异常，打印日志
      }
    XdInfoLog("进程[%s]执行完毕END",pIdentAppComm->GetAppName().c_str());
    delete pIdentAppComm;
    return 0;
}
