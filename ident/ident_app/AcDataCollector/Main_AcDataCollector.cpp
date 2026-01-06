/************************************************************
 Desc:     AC控制器数据采集服务 - 定时采集AC控制器和AP设备数据
 Auth:     Auto
 Modify:
 data:     2025-01-29
 ***********************************************************/

#include "CIdentAppComm.h"
#include "CAcDataCollector.h"
#include "CTrapReceiver.h"
#include <signal.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>

CIdentAppComm* pIdentAppComm = NULL;
CAcDataCollector* pAcDataCollector = NULL;
CTrapReceiver* pTrapReceiver = NULL;
bool g_running = true;

// 信号处理函数
void SignalHandler(int sig)
{
    if (sig == SIGINT || sig == SIGTERM)
    {
        XdInfoLog("收到退出信号，准备停止采集服务...");
        g_running = false;
        if (pAcDataCollector)
        {
            pAcDataCollector->Stop();
        }
        if (pTrapReceiver)
        {
            pTrapReceiver->Stop();
        }
    }
}

int main(int argc, char** argv)
{
    // 尝试禁用SNMP++库的调试输出
    // 方法1：设置环境变量（如果snmp_pp库支持）
    setenv("SNMP_DEBUG", "0", 1);
    // 方法2：如果库支持，可以尝试其他环境变量
    // setenv("SNMP_LOG_LEVEL", "0", 1);
    
    // 注意：如果环境变量不起作用，建议在启动脚本中过滤：
    // ./app_AcDataCollector 2>&1 | grep -v "SNMPMessage"
    
    // 初始化
    CStr2Map inMap, outMap;
    
    if (argc < 2)
    {
        printf("Usage: %s <config_path> [collect_interval]\n", argv[0]);
        printf("  config_path: 配置文件路径\n");
        printf("  collect_interval: 采集间隔（秒），默认5\n");
        return 1;
    }

    // 设置配置文件路径
    CAppInitEnv initEnv;
    initEnv.conf_path = string(argv[1]);
    initEnv.app_name = "app_AcDataCollector";
    // 采集间隔（秒），默认5秒
    int collectInterval = 5;
    if (argc >= 3)
    {
        collectInterval = atoi(argv[2]);
        if (collectInterval < 5)
        {
            collectInterval = 5;  // 最小5秒
        }
        if (collectInterval > 300)
        {
            collectInterval = 300;  // 最大300秒
        }
    }
    pIdentAppComm = new CIdentAppComm();
    if (NULL == pIdentAppComm)
    {
        XdErrorLog("进程[%s]初始化异常", initEnv.app_name.c_str());
        return -1;
    }
    
    pIdentAppComm->InitEnv(initEnv);
    
    // 注册信号处理
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);
    
    pAcDataCollector = new CAcDataCollector();
    if (NULL == pAcDataCollector)
    {
        XdErrorLog("创建数据采集对象失败");
        delete pIdentAppComm;
        return -1;
    }

    try
    {
        // 初始化采集器
        if (pAcDataCollector->Init() != 0)
        {
            XdErrorLog("数据采集器初始化失败");
            delete pAcDataCollector;
            delete pIdentAppComm;
            return -1;
        }

        // 初始化并启动Trap接收器（独立线程，不影响采集业务）
        pTrapReceiver = new CTrapReceiver();
        if (pTrapReceiver != NULL)
        {
            if (pTrapReceiver->Init() == 0)
            {
                if (pTrapReceiver->Start() == 0)
                {
                    XdInfoLog("Trap接收器已启动（独立线程）");
                }
                else
                {
                    XdErrorLog("Trap接收器启动失败，继续运行采集业务");
                    delete pTrapReceiver;
                    pTrapReceiver = NULL;
                }
            }
            else
            {
                XdErrorLog("Trap接收器初始化失败，继续运行采集业务");
                delete pTrapReceiver;
                pTrapReceiver = NULL;
            }
        }
        else
        {
            XdErrorLog("创建Trap接收器失败，继续运行采集业务");
        }

        // 进入采集循环
        int loopCount = 0;
        while (g_running)
        {
            loopCount++;
            XdInfoLog("========== 开始第 %d 次数据采集 ==========", loopCount);
            
            try
            {
                // 执行数据采集
                pAcDataCollector->CollectData(inMap, outMap);
                
                XdInfoLog("第 %d 次数据采集完成", loopCount);
            }
            catch (CTrsExp & e)
            {
                XdErrorLog("第 %d 次数据采集异常 errcode=[%s] errmsg=[%s]", 
                          loopCount, e.retcode.c_str(), e.retmsg.c_str());
            }
            catch (...)
            {
                XdErrorLog("第 %d 次数据采集发生未知异常", loopCount);
            }

            // 等待指定时间后继续下一次采集
            if (g_running)
            {
                XdInfoLog("等待 %d 秒后进行下一次采集...", collectInterval);
                for (int i = 0; i < collectInterval && g_running; ++i)
                {
                    sleep(1);  // 每秒检查一次运行标志
                }
            }
        }
    }
    catch (CTrsExp & e)
    {
        XdErrorLog("进程[%s]执行异常 errcode=[%s] errmsg=[%s]", 
                  pIdentAppComm->GetAppName().c_str(), 
                  e.retcode.c_str(), e.retmsg.c_str());
    }
    catch (...)
    {
        XdErrorLog("进程[%s]发生未知异常", pIdentAppComm->GetAppName().c_str());
    }

    // 清理资源
    if (pTrapReceiver)
    {
        XdInfoLog("停止Trap接收器...");
        pTrapReceiver->Stop();
        delete pTrapReceiver;
        pTrapReceiver = NULL;
    }
    
    if (pAcDataCollector)
    {
        pAcDataCollector->Cleanup();
        delete pAcDataCollector;
    }
    
    XdInfoLog("AC数据采集服务[%s]停止", pIdentAppComm->GetAppName().c_str());
    delete pIdentAppComm;
    
    return 0;
}

