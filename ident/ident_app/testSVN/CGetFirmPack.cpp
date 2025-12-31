/************************************************************
 Desc:     用SVN去获取固件包
 Auth:     leize
 Modify:
 data:     2025-01-20
 ***********************************************************/
#include "adstcp.h"
#include "CGetFirmPack.h"
#include "CIdentRelayApi.h"
#include "CTools.h"
#include "CSmtp.h"

#include <iostream>
#include <dirent.h>
#include <string>
#include <pthread.h>
#include <vector>
#include <atomic>
#include <unistd.h>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <thread>
#include <chrono>

volatile bool downloadThread = true;  // 标志位，表示下载是否在进行中
pthread_mutex_t outputMutex; // 用于保护输出
std::atomic<long> totalDownloaded(0); // 用于跟踪总下载大小
bool done = false; // 用于指示下载是否完成

struct ExportTask {
    std::string svn_url;
    std::string destination;
    std::string username;
    std::string password;
    long totalSize;
};


extern CIdentAppComm* pIdentAppComm;
void* dealSVNPackGet(void* arg);

long getFileSize(const std::string& path) 
{
    struct stat stat_buf;
    if (stat(path.c_str(), &stat_buf) == 0) {
        return stat_buf.st_size;
    }
    return -1; // 如果获取文件大小失败
}

long getDirectorySize(const std::string& dirPath) {
    long totalSize = 0;
    DIR* dir = opendir(dirPath.c_str());
    if (dir == nullptr) {
        return -1; // 打开目录失败
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        // 跳过 "." 和 ".."
        if (entry->d_name[0] == '.') {
            continue;
        }
        
        std::string filePath = dirPath + "/" + entry->d_name;
        if (entry->d_type == DT_DIR) {
            // 如果是目录，递归调用
            long dirSize = getDirectorySize(filePath);
            if (dirSize >= 0) {
                totalSize += dirSize;
            }
        } else {
            // 如果是文件，获取文件大小
            long fileSize = getFileSize(filePath);
            if (fileSize >= 0) {
                totalSize += fileSize;
            }
        }
    }
    closedir(dir);
    return totalSize;
}

void* progressUpdater(void* arg) 
{
    long totalSize = *static_cast<long*>(arg);
    while (!done) {
        sleep(2);
        long downloaded = totalDownloaded.load();
        double progress = static_cast<double>(downloaded) / totalSize * 100.0;
        pthread_mutex_lock(&outputMutex);
        InfoLog("当前进度 [%f: %ld/%ld]",progress,downloaded,totalSize);
        pthread_mutex_unlock(&outputMutex);
    }
    return nullptr;
}

CGetFirmPack::CGetFirmPack()
{
}
CGetFirmPack::~CGetFirmPack()
{
}

int CGetFirmPack::Commit(CStr2Map & inMap, CStr2Map &outMap)
{
    CStr2Map qryInMap,pvInMap,pvOutMap,upInMap,upOutMap,tmpInMap,tmpOutMap;
    int ifor=0;
	int nRet = 0;

    InfoLog("1-PV-申请PV");
    pvInMap["sem_id"] = "1"; //拉取SVN包
    pvInMap["sem_op"] = "P";
    CIdentRelayApi::CommSemPV(pvInMap,pvOutMap,FALSE);
    if(pvOutMap["pv_result"]!="1")
    {
        ErrorLog("获取发起批量信号量失败 id=[1] state=[%s]",pvOutMap["sem_state"].c_str());
        return -1;
    }

    InfoLog("获取固件规则表数据pack_id=[%s]",inMap["pack_id"].c_str());
    vector<CStr2Map> vectmapArray;
    qryInMap["offset"] = "0";
    qryInMap["limit"]  = "10"; //一次查询10笔
	//qryInMap["state"] = "0";  //待生效
    qryInMap["pack_id"] = inMap["pack_id"];  //包ID

    int ret_num = 0;
    std::string strSavePath1="/home/httpd/web/web_pay5/upload/Android-app.zip";
    std::string hashvalue = CIdentPub::calculateFileHash(strSavePath1);
    InfoLog("固件HASH值 hashvalue=[%s]",hashvalue.c_str());
    return 0;
	
	//将容器清空
	vectmapArray.clear(); 
	//查询数据并保存在map容器
	CIdentRelayApi::QueryFirmPreList(qryInMap,outMap,vectmapArray,false);
	InfoLog("获取固件下载规则表数据 vect-size=[%d]",vectmapArray.size());
	ret_num = atoi(outMap["ret_num"].c_str());
	if(ret_num ==0)
	{
		InfoLog("无需处理-或处理完-退出");
		InfoLog("1-PV-释放PV");
    	pvInMap["sem_id"] = "1";
    	pvInMap["sem_op"] = "V";
    	CIdentRelayApi::CommSemPV(pvInMap,pvOutMap,FALSE);
    	if(pvOutMap["pv_result"]!="1")
   	 	{
         	ErrorLog("释放发起批量信号量失败 id=[1] state=[%s]", pvOutMap["sem_state"].c_str());
   	 	}
        return 0;
	}
    char localSavePath[256]={0};
	memcpy(localSavePath,g_mTransactions[GetTid()].m_mVars["basepath"].c_str(),g_mTransactions[GetTid()].m_mVars["basepath"].length());

    //获取SVN用户名密码

    string strSavePath=localSavePath;
	string fileName,fileSizeStr;

    long totalSize = 0;
    // 初始化互斥锁
    pthread_mutex_init(&outputMutex, nullptr);
    // 计算所有包的总大小
    std::vector<pthread_t> threads;
    std::vector<ExportTask> tasks;
    totalSize = 20784334;
    InfoLog("包总大小[%ld]",totalSize);

    for(size_t i = 0;i < vectmapArray.size();i++) 
    {
        CStr2Map returnMap;
	    CIdentPub::DelMapF(vectmapArray[i],returnMap);
        returnMap["svn_path"] = CIdentPub::escape_svn_path(returnMap["svn_path"]);
        //utf-8转GBK执行
        char buff[1024] = {0} ;
        int  iDestLen = sizeof(buff)-1;
        Tools::ConvertCharSet((char*)returnMap["svn_path"].c_str(),buff,iDestLen,"utf-8","GBK");

        ExportTask task = {buff, strSavePath, "leize", "launch*0512", totalSize};
        tasks.push_back(task);
        pthread_t thread;
        pthread_create(&thread, nullptr, dealSVNPackGet, &tasks.back());
        threads.push_back(thread);
    }

    // 设置完成标志
    done = true;
    pthread_join(downloadThread, nullptr);

    InfoLog("所有包导出完成");

    // 销毁互斥锁
    pthread_mutex_destroy(&outputMutex);

    return 0;

}
std::string destination = "/home/httpd/web/web_pay5/upload/svnTmp";

void trackProgress() {
    long previousSize = 0;
    while (downloadThread) {
        long currentSize = getDirectorySize(destination);
        if (currentSize >= 0) {
            pthread_mutex_lock(&outputMutex);
            InfoLog( "已下载: %ld 字节",currentSize ); // 输出当前下载进度
            pthread_mutex_unlock(&outputMutex);
            previousSize = currentSize;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1)); // 每秒检查一次
    }
}

void* dealSVNPackGet(void* arg) 
{
    ExportTask* task = static_cast<ExportTask*>(arg);
    std::string command = "svn export --force --username " + task->username + " --password " + task->password + " " + task->svn_url + " " + task->destination + " --trust-server-cert --non-interactive";

    destination = task->destination;
    InfoLog("command[%s]", command.c_str());
    
    // 启动进度监控线程
    std::thread progressThread(trackProgress);

    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        pthread_mutex_lock(&outputMutex);
        InfoLog("无法打开管道: %s", task->svn_url.c_str());
        pthread_mutex_unlock(&outputMutex);
        return nullptr;
    }

    // 等待命令执行完成
    int result = pclose(pipe);
    //downloading = false; // 下载完成

    // 等待进度监控线程结束
    if (progressThread.joinable()) {
        progressThread.join();
    }

    if (result != 0) {
        pthread_mutex_lock(&outputMutex);
        InfoLog("SVN指令运行错误,错误代码: %d for %s", result, task->svn_url.c_str());
        pthread_mutex_unlock(&outputMutex);
    }

    return nullptr;
}

long CGetFirmPack::getPackageSize(const std::string& svn_url,const string& username, const string& password) 
{
    std::string command = "svn info " + svn_url + " --username " + username + " --password " + password +" --trust-server-cert --non-interactive";
    InfoLog("command[%s] ", command.c_str() );
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        pthread_mutex_lock(&outputMutex);
        InfoLog("无法打开管道[%s] ", svn_url.c_str() );
        pthread_mutex_unlock(&outputMutex);
        return 0;
    }

    char buffer[128];
    long size = 0;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string line(buffer);
        if (line.find("Size:") != std::string::npos) {
            size = std::stol(line.substr(line.find(":") + 1)); // 获取大小
            break;
        }
    }

    pclose(pipe);
    return size;
}