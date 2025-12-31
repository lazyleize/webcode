/************************************************************
 Desc:     根据固件包唯一id去SVN获取下载包，并同步到共享路径
 Auth:     leize
 Modify:
 data:     2018-08-02
 ***********************************************************/
#include <random>

#include <base/datetime.hpp>
#include <base/strHelper.hpp>
#include <base/directory.hpp>
#include <base/file.hpp>

#include "CIdentAppComm.h"
#include "curl/curl.h" 
// 取消宏定义，避免冲突
#undef max
#undef min
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson; 
using namespace aps;

static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)  
{  
    // 将 lpVoid 转换为 std::string*
    std::string* str = static_cast<std::string*>(lpVoid);  
    if (str == nullptr || buffer == nullptr)  
    {  
        return 0;  // 返回 0 表示写入失败
    }  

    // 将 buffer 转换为 char*
    char* pData = static_cast<char*>(buffer);  
    // 将数据追加到 str 中
    str->append(pData, size * nmemb);  
    return size * nmemb;  // 返回写入的字节数
}

CIdentAppComm* pIdentAppComm = NULL;
int GenerateRandomNumber() 
{
    // 随机数生成器
    std::random_device rd;  // 获取随机数种子
    std::mt19937 gen(rd());  // 使用 Mersenne Twister 引擎

    // 生成 2 到 5 位数
    std::uniform_int_distribution<> dis(10, 99999);  // 10 到 99999 的范围

    int randomNumber = dis(gen);
    
    // 确保生成的数是 2 到 5 位
    while (randomNumber < 10 || randomNumber > 99999) {
        randomNumber = dis(gen);
    }

    return randomNumber;
}

int HttpESPost( const std::string & strPost)  
{  
    CURLcode res;  
    string strUrl =  "http://localhost:9200/pack/_doc";
    string strResponse;
    CURL* curl = curl_easy_init();  
    if(NULL == curl)  
    {     
        printf("curl_easy_init fail"); 
        return -1;  
    }  
    //printf("strPost=%s\n",strPost.c_str());
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());  
    curl_easy_setopt(curl, CURLOPT_POST, 1);  
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());  
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);  
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);  
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);  
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30);  
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60);  
    res = curl_easy_perform(curl);  

    long responseCode;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode); // 获取HTTP响应代

    printf("strResponse=%s\n",strResponse.c_str());
    if (res != CURLE_OK) 
    {
        curl_easy_cleanup(curl);
        printf("写入失败");
        return -1;
    }
    
    curl_easy_cleanup(curl);     
    return 0;  
}  

void ModifyFuncList(Document& document) 
{
    if (document.HasMember("funclist") && document["funclist"].IsArray()) 
    {
        const Value& funcList = document["funclist"];
        printf("sizeo=%d\n", funcList.Size());

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 1);

        for (SizeType i = 0; i < funcList.Size(); i++) 
        {
           
            if (funcList[i].IsObject()) 
            {
                Value& funcObj = const_cast<Value&>(funcList[i]);

                // 打印当前对象以检查状态
                StringBuffer buffer;
                Writer<StringBuffer> writer(buffer);
                funcObj.Accept(writer);
                //printf("Current funcObj: %s\n", buffer.GetString());

                // 修改 OnceSuccess
                if (funcObj.HasMember("OnceSuccess") && funcObj["OnceSuccess"].IsString()) 
                {
                    const char* newValue = (dis(gen) == 0) ? "Yes" : "No";
                    //printf("Setting OnceSuccess to: %s\n", newValue);
                    funcObj["OnceSuccess"].SetString(newValue, document.GetAllocator());
                }

                // 修改 result
                if (funcObj.HasMember("result") && funcObj["result"].IsString()) 
                {
                    const char* newValue = (dis(gen) == 0) ? "Success" : "Failure";
                    //printf("Setting result to: %s\n", newValue);
                    funcObj["result"].SetString(newValue, document.GetAllocator());
                }

                // 修改 duration
                if (funcObj.HasMember("duration") && funcObj["duration"].IsInt()) 
                {
                    int newDuration = GenerateRandomNumber(); // 假设这个函数有效
                    //printf("Setting duration to: %d\n", newDuration);
                    funcObj["duration"].SetInt(newDuration);
                }
            }
            else 
            {
                printf("funcList[%d] is not an object.\n", i);
            }
        }
    } 
    else 
    {
        printf("funclist not found or is not an array.");
    }
}


int main(int argc, char** argv)
{
    //初始化
    if (argc < 2)
    {
        printf("Need to input Json-File\n");
        return 1;
    }
	

    std::string jsonFilePath = argv[1];
    //string strFullPath = jsonFilePath;
    //DebugLog("strFullPath=[%s]",strFullPath.c_str());
    std::string Strcount = argv[2];
    int nCount = atol(Strcount.c_str());

	//获取数据，然后解析入库
	/*ifstream inFile(strFullPath.c_str(), std::ios::in | std::ios::binary);
    if (!inFile)
	{
		ErrorLog("文件打开失败[%s]",strFullPath.c_str());
		throw(CTrsExp(ERR_UPFILE_COUNT, "文件打开失败"));
		return 0;
	}
    

	std::ostringstream oss;
    oss << inFile.rdbuf(); // 读取整个文件内容到ostringstream中

    // 关闭文件
    inFile.close();
	string filecontext = oss.str();*/

    // 随机替换 posname
    vector<string> posnames = {"N80", "N86", "N86 Pro", "N82", "N96", "UN20Min"};
    random_device rd;  // 用于随机数生成
    mt19937 gen(rd()); // 随机数生成器
    uniform_int_distribution<> dis(0, posnames.size() - 1);
    string basePid = "WKK641431000";  // 基础 PID

    string baseOrderNumbers[5] = {"WKK241430000", "WKK241430001", "WKK241430002", "WKK241430003", "WKK241430004"};
    int orderIndex = 0;  // 当前订单号索引
    int pidCounter = 0;  // PID计数器

    //判断是否是目录
    bool isDir =  Directory::isDirectory(jsonFilePath);
    if(!isDir)
    {
        ifstream inFile(jsonFilePath.c_str(), std::ios::in | std::ios::binary);
        if (!inFile)
	    {
		    ErrorLog("文件打开失败[%s]",jsonFilePath.c_str());
		    throw(CTrsExp(ERR_UPFILE_COUNT, "文件打开失败"));
		    return 0;
	    }
    

	    std::ostringstream oss;
        oss << inFile.rdbuf(); // 读取整个文件内容到ostringstream中
        // 关闭文件
        inFile.close();
	    string filecontext = oss.str();
        Document document;

        for (int orderLoop = 0; orderLoop < 5; ++orderLoop) 
        { // 循环5个订单号
            for (int nCount = 0; nCount < 1000; ++nCount) 
            { // 每个订单号循环1000次
                
	            if(document.Parse(filecontext.c_str()).HasParseError())
	            {
		            printf("JSON解析失败\r\n");
		            return 0;
	            }
                document["posname"].SetString(posnames[dis(gen)].c_str(), document.GetAllocator());
                //增加时间的修改 timestamp
                if(document.HasMember("timestamp") && document["timestamp"].IsString())
                {
                    Datetime m_dateTime;
                    m_dateTime.setDatetime(2025,8,1,12,0,0,0);
                    m_dateTime.addHours(1);
                    //格式化
                    string strDateTime = m_dateTime.toString();
                    document["timestamp"].SetString(aps::StrHelper::replace(strDateTime," ","T").c_str(),document.GetAllocator());
                }
		    

                ModifyFuncList(document);
                // 自动生成 PID 和订单号
                string newOrderNumber = baseOrderNumbers[orderIndex]; // 获取当前订单号
                string newPid = newOrderNumber + to_string(pidCounter++);
                document["pid"].SetString(newPid.c_str(), document.GetAllocator());

                // 将 Document 转换为字符串
                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                document.Accept(writer);
                std::string jsonString = buffer.GetString();
    
                //写入ES
                if(HttpESPost(jsonString) != 0)
                    break;
                printf("写入成功\r\n");
            }

            orderIndex = (orderIndex + 1) % 5; // 切换到下一个订单号
        }
        printf("进程执行完毕END");
        delete pIdentAppComm;
        return 0;  
    }


    
    printf("进程执行START\n");
    //循环
    while(nCount--)
    {
        vector<FileInfo> files;
        Directory::getSubFiles(jsonFilePath,files);
        if(files.size()==0)
        {
            printf("路径错误,没有JSON文件");
        }
        int nFileNum = files.size();
        for (const auto& file : files) // 遍历每个 FileInfo 对象
        {
            string filePath = file.fullpath();
            Document document;
	        if(document.Parse(filePath.c_str()).HasParseError())
	        {
		        //printf("JSON解析失败,继续...\r\n");
		        continue;
	        }
            document["posname"].SetString(posnames[dis(gen)].c_str(), document.GetAllocator());
            //增加时间的修改 timestamp
            if(document.HasMember("timestamp") && document["timestamp"].IsString())
            {
                Datetime m_dateTime;
                m_dateTime.setDatetime(2025,6,1,12,0,0,0);
                m_dateTime.addHours(1);
                //格式化
                string strDateTime = m_dateTime.toString();
                document["timestamp"].SetString(aps::StrHelper::replace(strDateTime," ","T").c_str(),document.GetAllocator());
            }
		    

            ModifyFuncList(document);

            // 自动累加 pid
            string newPid = basePid + to_string(pidCounter++);
            document["pid"].SetString(newPid.c_str(), document.GetAllocator());

            // 将 Document 转换为字符串
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            document.Accept(writer);
            std::string jsonString = buffer.GetString();
            //printf("%s\n",jsonString.c_str());
        
            //写入ES
            if(HttpESPost(jsonString) != 0)
                break;
            printf("写入成功\r\n");
        }
    }


    printf("进程执行完毕END");
    delete pIdentAppComm;
    return 0;

}



