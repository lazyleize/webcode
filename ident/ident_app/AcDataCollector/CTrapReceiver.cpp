/************************************************************
 Desc:     SNMP Trap接收器实现
 Auth:     Auto
 Modify:
 data:     2025-12-08
 ***********************************************************/
#include "CTrapReceiver.h"
#include "CIdentAppComm.h"
#include "transxmlcfg.h"
#include "libmemcached/memcached.h"
#include <sstream>
#include <iomanip>
#include <cstring>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

extern CIdentAppComm* pIdentAppComm;

// 日志宏（使用现有的日志系统）
#define TrapInfoLog(fmt, ...) \
    do { \
        if (pIdentAppComm) { \
            XdInfoLog("[TrapReceiver] " fmt, ##__VA_ARGS__); \
        } \
    } while(0)

#define TrapErrorLog(fmt, ...) \
    do { \
        if (pIdentAppComm) { \
            XdErrorLog("[TrapReceiver] " fmt, ##__VA_ARGS__); \
        } \
    } while(0)

CTrapReceiver::CTrapReceiver() 
    : m_stopped(false), m_initialized(false), m_threadId(0),
      m_trapPort(162), m_cachePort(15210),
      m_snmp(NULL), m_snmpStatus(0), m_udpSocket(-1),
      m_totalReceived(0), m_totalSaved(0), m_totalErrors(0)
{
    m_cacheIp = "127.0.0.1";
    pthread_mutex_init(&m_mutex, NULL);
}

CTrapReceiver::~CTrapReceiver()
{
    Stop();
    if (m_udpSocket >= 0)
    {
        close(m_udpSocket);
        m_udpSocket = -1;
    }
    if (m_snmp)
    {
        delete m_snmp;
        m_snmp = NULL;
    }
    pthread_mutex_destroy(&m_mutex);
}

int CTrapReceiver::Init()
{
    if (m_initialized)
    {
        TrapErrorLog("Trap接收器已经初始化");
        return -1;
    }
    
    // 从配置读取缓存配置（复用CAcDataCollector的配置）
    string tid = "app_AcDataCollector";
    if (g_mTransactions.find(tid) != g_mTransactions.end())
    {
        CStr2Map& mVars = g_mTransactions[tid].m_mVars;
        if (mVars.find("cache_ip") != mVars.end())
        {
            m_cacheIp = mVars["cache_ip"];
        }
        if (mVars.find("cache_port") != mVars.end())
        {
            m_cachePort = atoi(mVars["cache_port"].c_str());
        }
        // 可选：Trap监听端口配置
        if (mVars.find("trap_port") != mVars.end())
        {
            m_trapPort = atoi(mVars["trap_port"].c_str());
        }
    }
    
    TrapInfoLog("Trap接收器初始化: 监听端口=%d, 缓存服务器=%s:%d",m_trapPort, m_cacheIp.c_str(), m_cachePort);
    
    m_initialized = true;
    return 0;
}

int CTrapReceiver::Start()
{
    if (!m_initialized)
    {
        TrapErrorLog("Trap接收器未初始化，无法启动");
        return -1;
    }
    
    if (m_threadId != 0)
    {
        TrapErrorLog("Trap接收线程已在运行");
        return -1;
    }
    
    m_stopped = false;
    
    // 创建线程
    int ret = pthread_create(&m_threadId, NULL, ThreadProc, this);
    if (ret != 0)
    {
        TrapErrorLog("创建Trap接收线程失败: %s", strerror(ret));
        return -1;
    }
    
    TrapInfoLog("Trap接收线程已启动，线程ID=%lu", (unsigned long)m_threadId);
    return 0;
}

void CTrapReceiver::Stop()
{
    if (m_threadId == 0)
    {
        return;
    }
    
    TrapInfoLog("停止Trap接收线程...");
    m_stopped = true;
    
    // 等待线程结束
    void* threadRet = NULL;
    int ret = pthread_join(m_threadId, &threadRet);
    if (ret != 0)
    {
        TrapErrorLog("等待Trap接收线程结束失败: %s", strerror(ret));
    }
    
    m_threadId = 0;
    TrapInfoLog("Trap接收线程已停止，统计: 接收=%d, 保存=%d, 错误=%d",m_totalReceived, m_totalSaved, m_totalErrors);
}

void* CTrapReceiver::ThreadProc(void* arg)
{
    CTrapReceiver* pReceiver = static_cast<CTrapReceiver*>(arg);
    if (pReceiver == NULL)
    {
        return NULL;
    }
    
    // 设置线程名称（便于调试，如果系统支持）
    #ifdef __GLIBC__
    pthread_setname_np(pthread_self(), "TrapReceiver");
    #endif
    
    // 异常安全的执行
    try
    {
        pReceiver->SafeReceiveLoop();
    }
    catch (const std::exception& e)
    {
        TrapErrorLog("Trap接收线程捕获标准异常: %s", e.what());
    }
    catch (...)
    {
        TrapErrorLog("Trap接收线程捕获未知异常");
    }
    
    return NULL;
}

void CTrapReceiver::SafeReceiveLoop()
{
    TrapInfoLog("Trap接收线程开始运行，准备初始化UDP Socket...");
    
    // 初始化UDP Socket（直接接收Trap，不依赖SNMP++库的特定API）
    try
    {
        // 创建UDP socket
        m_udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (m_udpSocket < 0)
        {
            TrapErrorLog("创建UDP socket失败: %s", strerror(errno));
            return;
        }
        
        // 设置socket选项：允许地址重用
        int opt = 1;
        if (setsockopt(m_udpSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        {
            TrapErrorLog("设置SO_REUSEADDR失败: %s", strerror(errno));
            close(m_udpSocket);
            m_udpSocket = -1;
            return;
        }
        
        // 绑定到监听地址
        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;  // 监听所有接口
        serverAddr.sin_port = htons(m_trapPort);
        
        if (bind(m_udpSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
        {
            TrapErrorLog("绑定UDP socket失败 (端口%d): %s", m_trapPort, strerror(errno));
            close(m_udpSocket);
            m_udpSocket = -1;
            return;
        }
        
        // 设置socket为非阻塞模式（可选，如果需要超时控制）
        // int flags = fcntl(m_udpSocket, F_GETFL, 0);
        // fcntl(m_udpSocket, F_SETFL, flags | O_NONBLOCK);
        
        TrapInfoLog("UDP socket已成功绑定到端口: %d，开始监听Trap数据包...", m_trapPort);
        
        // 初始化SNMP++库（用于解析PDU）
        Snmp::socket_startup();
        m_snmp = new Snmp(m_snmpStatus);
        if (m_snmpStatus != SNMP_CLASS_SUCCESS)
        {
            TrapErrorLog("创建SNMP对象失败: %s", m_snmp->error_msg(m_snmpStatus));
            delete m_snmp;
            m_snmp = NULL;
            close(m_udpSocket);
            m_udpSocket = -1;
            return;
        }
        TrapInfoLog("SNMP对象创建成功，Trap接收器已就绪");
    }
    catch (const std::exception& e)
    {
        TrapErrorLog("初始化时捕获异常: %s", e.what());
        if (m_udpSocket >= 0)
        {
            close(m_udpSocket);
            m_udpSocket = -1;
        }
        if (m_snmp)
        {
            delete m_snmp;
            m_snmp = NULL;
        }
        return;
    }
    catch (...)
    {
        TrapErrorLog("初始化时捕获未知异常");
        if (m_udpSocket >= 0)
        {
            close(m_udpSocket);
            m_udpSocket = -1;
        }
        if (m_snmp)
        {
            delete m_snmp;
            m_snmp = NULL;
        }
        return;
    }
    
    // 主接收循环（异常安全）
    // 使用UDP socket直接接收Trap数据包
    while (!m_stopped && m_udpSocket >= 0)
    {
        try
        {
            TrapInfo trap;
            int ret = ReceiveAndParseTrap(trap);
            
            if (ret == 0)
            {
                // 成功接收到Trap，保存到缓存（异常安全）
                try
                {
                    int saveResult = SaveToCache(trap);
                    if (saveResult == 0)
                    {
                        pthread_mutex_lock(&m_mutex);
                        m_totalSaved++;
                        pthread_mutex_unlock(&m_mutex);
                    }
                    else
                    {
                        pthread_mutex_lock(&m_mutex);
                        m_totalErrors++;
                        pthread_mutex_unlock(&m_mutex);
                        TrapErrorLog("Trap保存到缓存失败，totalErrors=%d", m_totalErrors);
                    }
                }
                catch (const std::exception& e)
                {
                    TrapErrorLog("保存Trap到缓存时捕获异常: %s", e.what());
                    pthread_mutex_lock(&m_mutex);
                    m_totalErrors++;
                    pthread_mutex_unlock(&m_mutex);
                }
                catch (...)
                {
                    TrapErrorLog("保存Trap到缓存时捕获未知异常");
                    pthread_mutex_lock(&m_mutex);
                    m_totalErrors++;
                    pthread_mutex_unlock(&m_mutex);
                }
            }
            else if (ret == -2)
            {
                // 超时或其他非致命错误，继续循环
                // trap_receive可能阻塞，这是正常的
                continue;
            }
            else
            {
                // 其他错误，短暂休眠避免CPU占用过高
                usleep(100000);  // 100ms
            }
        }
        catch (const std::exception& e)
        {
            TrapErrorLog("接收Trap时捕获异常: %s", e.what());
            pthread_mutex_lock(&m_mutex);
            m_totalErrors++;
            pthread_mutex_unlock(&m_mutex);
            
            // 短暂休眠，避免异常循环
            usleep(100000);  // 100ms
        }
        catch (...)
        {
            TrapErrorLog("接收Trap时捕获未知异常");
            pthread_mutex_lock(&m_mutex);
            m_totalErrors++;
            pthread_mutex_unlock(&m_mutex);
            
            // 短暂休眠，避免异常循环
            usleep(100000);  // 100ms
        }
    }
    
    TrapInfoLog("Trap接收线程退出");
}

int CTrapReceiver::ReceiveAndParseTrap(TrapInfo& trap)
{
    if (m_udpSocket < 0 || m_snmp == NULL)
    {
        return -1;
    }
    
    // 使用UDP socket接收Trap数据包
    char buffer[65536];  // SNMP消息最大长度
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    
    // 接收UDP数据包（阻塞模式）
    ssize_t recvLen = recvfrom(m_udpSocket, buffer, sizeof(buffer) - 1, 0,
                               (struct sockaddr*)&clientAddr, &clientAddrLen);
    
    if (recvLen < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return -2;  // 超时（非阻塞模式）
        }
        // 总是记录接收错误（重要调试信息）
        TrapErrorLog("接收UDP数据包失败: %s (socket=%d, port=%d)",strerror(errno), m_udpSocket, m_trapPort);
        return -1;
    }
    
    if (recvLen == 0)
    {
        TrapInfoLog("收到空UDP数据包");
        return -2;  // 空数据包
    }
    
    // 获取来源IP
    char ipStr[64];
    inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr, sizeof(ipStr));
    trap.sourceIp = string(ipStr);
    
    // 总是记录接收到数据包（重要调试信息）
        // 收到UDP数据包（日志已移除，减少输出）
    
    // 使用SNMP++库解析PDU
    // 将UDP数据包转换为OctetStr，然后解析为PDU
    // 注意：OctetStr构造函数需要const unsigned char*
    OctetStr octetStr((const unsigned char*)buffer, (unsigned long)recvLen);
    Pdu pdu;
    
    // 尝试解析SNMP消息
    // SNMP++库的Snmp对象可能有receive或decode方法
    // 如果不存在，我们尝试使用简单的ASN.1解析来提取关键字段
    try
    {
        // 方法1: 尝试使用Snmp对象的receive方法（但需要socket，我们已经有了数据）
        // 方法2: 尝试简单的ASN.1解析来提取Trap OID
        
        // 记录基本信息
        trap.receiveTime = time(NULL);
        struct tm* tmPtr = localtime(&trap.receiveTime);
        if (tmPtr)
        {
            char timeStr[64];
            snprintf(timeStr, sizeof(timeStr), "%04d-%02d-%02d %02d:%02d:%02d",
                     tmPtr->tm_year + 1900, tmPtr->tm_mon + 1, tmPtr->tm_mday,
                     tmPtr->tm_hour, tmPtr->tm_min, tmPtr->tm_sec);
            trap.receiveTimeStr = string(timeStr);
        }
        
        // 使用完整的BER解析器提取所有信息（主机名、VarBinds等）
        const unsigned char* data = (const unsigned char*)buffer;
        size_t dataLen = recvLen;
        
        // 尝试使用完整的BER解析器解析SNMP消息
        int berParseResult = ParseBerSnmpMessage(data, dataLen, trap);
        
        if (berParseResult != 0)
        {
            // BER解析失败，尝试简单的OID提取作为后备
            string extractedOid = ExtractOidFromBer(data, dataLen);
            if (!extractedOid.empty() && extractedOid != "unknown")
            {
                trap.trapOid = extractedOid;
                trap.trapType = GetTrapTypeName(extractedOid);
                // 从BER编码中提取到Trap OID
            }
            else
            {
                // 如果BER解析失败，尝试字符串匹配（作为后备方案）
                string dataStr((const char*)data, dataLen);
                size_t oidPos = dataStr.find("1.3.6.1.4.1.2011.6.139");
                if (oidPos != string::npos)
                {
                    size_t start = oidPos;
                    size_t end = start;
                    while (end < dataStr.length() && 
                           (dataStr[end] == '.' || (dataStr[end] >= '0' && dataStr[end] <= '9')))
                    {
                        end++;
                    }
                    if (end > start && end - start < 100)
                    {
                        string foundOid = dataStr.substr(start, end - start);
                        trap.trapOid = foundOid;
                        trap.trapType = GetTrapTypeName(foundOid);
                        // 通过字符串匹配找到Trap OID
                    }
                }
                
                // 如果还是没找到，标记为未知
                if (trap.trapOid.empty() || trap.trapOid == "unknown")
                {
                    trap.trapOid = "unknown";
                    trap.trapType = "huawei-unknown";
                    // 未找到Trap OID，标记为unknown
                }
            }
        }
        
        // 如果Trap OID已提取，设置Trap类型
        if (!trap.trapOid.empty() && trap.trapOid != "unknown" && trap.trapType.empty())
        {
            trap.trapType = GetTrapTypeName(trap.trapOid);
        }
        
        // 保存原始数据长度
        trap.vars["raw_data_length"] = std::to_string(recvLen);
        
        // Trap解析完成（详细日志已移除，减少输出）
        // 由于无法直接解析PDU，我们创建一个空的PDU用于后续处理
        // ParseTrapPdu函数会处理这种情况
    }
    catch (const std::exception& e)
    {
        if (m_totalReceived == 0 || m_totalErrors < 10)
        {
            TrapErrorLog("处理SNMP消息时发生异常: %s", e.what());
        }
        // 记录基本信息
        trap.trapOid = "unknown";
        trap.trapType = "unknown";
        trap.receiveTime = time(NULL);
        struct tm* tmPtr = localtime(&trap.receiveTime);
        if (tmPtr)
        {
            char timeStr[64];
            snprintf(timeStr, sizeof(timeStr), "%04d-%02d-%02d %02d:%02d:%02d",
                     tmPtr->tm_year + 1900, tmPtr->tm_mon + 1, tmPtr->tm_mday,
                     tmPtr->tm_hour, tmPtr->tm_min, tmPtr->tm_sec);
            trap.receiveTimeStr = string(timeStr);
        }
        trap.vars["raw_data_length"] = std::to_string(recvLen);
        trap.vars["parse_exception"] = e.what();
        return 0;
    }
    catch (...)
    {
        if (m_totalReceived == 0 || m_totalErrors < 10)
        {
            TrapErrorLog("处理SNMP消息时发生未知异常");
        }
        // 记录基本信息
        trap.trapOid = "unknown";
        trap.trapType = "unknown";
        trap.receiveTime = time(NULL);
        struct tm* tmPtr = localtime(&trap.receiveTime);
        if (tmPtr)
        {
            char timeStr[64];
            snprintf(timeStr, sizeof(timeStr), "%04d-%02d-%02d %02d:%02d:%02d",
                     tmPtr->tm_year + 1900, tmPtr->tm_mon + 1, tmPtr->tm_mday,
                     tmPtr->tm_hour, tmPtr->tm_min, tmPtr->tm_sec);
            trap.receiveTimeStr = string(timeStr);
        }
        trap.vars["raw_data_length"] = std::to_string(recvLen);
        return 0;
    }
    
    // 创建UdpAddress对象用于ParseTrapPdu
    // 从sockaddr_in中提取IP地址字符串
    TrapInfoLog("准备调用ParseTrapPdu解析PDU...");
    UdpAddress fromAddr(ipStr);
    
    // 解析Trap PDU（如果PDU为空，ParseTrapPdu会处理）
    // 由于我们无法直接解析PDU，ParseTrapPdu需要能够处理空PDU的情况
    int pduResult = ParseTrapPdu(pdu, fromAddr, trap);
    TrapInfoLog("ParseTrapPdu返回: result=%d, trapType=%s", pduResult, trap.trapType.c_str());
    
    if (pduResult != 0)
    {
        // 即使解析失败，也返回0，因为我们已经记录了基本信息
        TrapInfoLog("ParseTrapPdu返回非0，但继续处理");
        return 0;
    }
    
    TrapInfoLog("准备更新统计信息...");
    pthread_mutex_lock(&m_mutex);
    m_totalReceived++;
    pthread_mutex_unlock(&m_mutex);
    TrapInfoLog("统计信息已更新: totalReceived=%d", m_totalReceived);
    
    // 记录Trap信息
    if (trap.trapType == "unknown" || trap.trapType == "huawei-unknown")
    {
        // 未知Trap类型，记录详细信息便于后续分析
        TrapInfoLog("收到未知类型Trap: 来源=%s, OID=%s, 类型=%s, VarBinds数量=%zu", 
                    trap.sourceIp.c_str(), trap.trapOid.c_str(), trap.trapType.c_str(), trap.vars.size());
        // 记录所有VarBinds的OID和值，便于分析
        int count = 0;
        for (map<string, string>::const_iterator it = trap.vars.begin(); 
             it != trap.vars.end() && count < 10; ++it, ++count)
        {
            TrapInfoLog("  VarBind[%d]: OID=%s, Value=%s",count, it->first.c_str(), it->second.c_str());
        }
        if (trap.vars.size() > 10)
        {
            TrapInfoLog("  ... (还有 %zu 个VarBinds未显示)", trap.vars.size() - 10);
        }
    }
    else
    {
        TrapInfoLog("收到Trap: 来源=%s, OID=%s, 类型=%s",trap.sourceIp.c_str(), trap.trapOid.c_str(), trap.trapType.c_str());
    }
    
    TrapInfoLog("ReceiveAndParseTrap处理完成，准备返回");
    return 0;
}

int CTrapReceiver::ParseTrapPdu(const Pdu& pdu, const UdpAddress& fromAddr, TrapInfo& trap)
{
    // 获取来源IP（get_printable()无参数，返回const char*）
    const char* ipStr = fromAddr.get_printable();
    if (ipStr)
    {
        // 如果trap.sourceIp还没有设置，使用fromAddr的IP
        if (trap.sourceIp.empty() || trap.sourceIp == "unknown")
        {
            trap.sourceIp = string(ipStr);
        }
    }
    else if (trap.sourceIp.empty())
    {
        trap.sourceIp = "unknown";
    }
    
    // 如果接收时间还没有设置，设置它
    if (trap.receiveTime == 0)
    {
        trap.receiveTime = time(NULL);
        struct tm* tmPtr = localtime(&trap.receiveTime);
        if (tmPtr)
        {
            char timeStr[64];
            snprintf(timeStr, sizeof(timeStr), "%04d-%02d-%02d %02d:%02d:%02d",
                     tmPtr->tm_year + 1900, tmPtr->tm_mon + 1, tmPtr->tm_mday,
                     tmPtr->tm_hour, tmPtr->tm_min, tmPtr->tm_sec);
            trap.receiveTimeStr = string(timeStr);
        }
    }
    
    // 提取VarBinds（如果PDU为空，vbCount为0，循环不会执行）
    int vbCount = pdu.get_vb_count();
    if (vbCount > 0)
    {
        for (int i = 0; i < vbCount; ++i)
        {
            Vb vb;
            pdu.get_vb(vb, i);
            
            Oid oid;
            vb.get_oid(oid);
            string oidStr = oid.get_printable();
            string valueStr = vb.get_printable_value();
            
            trap.vars[oidStr] = valueStr;
            
            // 查找Trap OID（snmpTrapOID.0）
            if (oidStr == "1.3.6.1.6.3.1.1.4.1.0" || oidStr.find("snmpTrapOID") != string::npos)
            {
                trap.trapOid = valueStr;
                trap.trapType = GetTrapTypeName(trap.trapOid);
            }
            
            // 查找Enterprise OID
            if (oidStr == "1.3.6.1.6.3.1.1.4.3.0" || oidStr.find("snmpTrapEnterprise") != string::npos)
            {
                trap.enterpriseOid = valueStr;
            }
        }
    }
    else
    {
        // PDU为空，说明无法解析SNMP消息
        // 如果trapOid还是unknown，标记为需要手动解析
        if (trap.trapOid == "unknown")
        {
            trap.trapType = "huawei-unknown";
        }
    }
    
    // 如果没有找到Trap OID，尝试从VarBinds推断
    if (trap.trapOid.empty() && !trap.vars.empty())
    {
        // 查找可能的Trap OID（华为AC的Trap OID格式）
        // 优先查找标准SNMP Trap OID
        for (map<string, string>::const_iterator it = trap.vars.begin(); 
             it != trap.vars.end(); ++it)
        {
            string oid = it->first;
            // 查找华为设备OID（1.3.6.1.4.1.2011）
            if (oid.find("1.3.6.1.4.1.2011") != string::npos)
            {
                // 检查是否是Trap OID（通常包含特定的后缀模式）
                // 华为AC Trap OID常见模式：
                // - 1.3.6.1.4.1.2011.6.139.13.1.1.x (AP相关)
                // - 1.3.6.1.4.1.2011.6.139.16.1.1.1.x (射频相关)
                // - 1.3.6.1.4.1.2011.5.25.x (实体相关)
                if (oid.find(".1.1.") != string::npos || 
                    oid.find(".2.10.") != string::npos ||
                    oid.find(".2.14.") != string::npos ||
                    oid.find(".2.15.") != string::npos ||
                    oid.find(".2.24.") != string::npos ||
                    oid.find(".3.1.") != string::npos ||
                    oid.find(".4.") != string::npos)
                {
                    trap.trapOid = oid;
                    trap.trapType = GetTrapTypeName(trap.trapOid);
                    break;
                }
            }
        }
        
        // 如果仍未找到，记录所有VarBinds的OID以便分析
        if (trap.trapOid.empty())
        {
            TrapInfoLog("未找到Trap OID，记录所有VarBinds以便分析，来源=%s", trap.sourceIp.c_str());
            int count = 0;
            for (map<string, string>::const_iterator it = trap.vars.begin(); 
                 it != trap.vars.end() && count < 10; ++it, ++count)
            {
                TrapInfoLog("  VarBind[%d]: OID=%s, Value=%s",count, it->first.c_str(), it->second.c_str());
            }
            // 使用第一个VarBind的OID作为临时Trap OID
            if (!trap.vars.empty())
            {
                trap.trapOid = trap.vars.begin()->first;
                trap.trapType = "unknown";
            }
        }
    }
    
    // 解析业务数据
    ParseBusinessData(trap);
    
    return 0;
}

// ParseBusinessData函数实现已移至TrapDataParse.cpp
// 该文件包含所有Trap类型的详细解析逻辑
// 注意：TrapDataParse.cpp中包含了完整的ParseBusinessData实现

int CTrapReceiver::SaveToCache(const TrapInfo& trap)
{
    // 创建Memcached连接
    memcached_st* pMemc = memcached_create(NULL);
    if (pMemc == NULL)
    {
        TrapErrorLog("memcached_create失败");
        return -1;
    }
    
    memcached_return_t rc = memcached_server_add(pMemc, m_cacheIp.c_str(), m_cachePort);
    if (rc != MEMCACHED_SUCCESS)
    {
        TrapErrorLog("连接Memcached失败: %s", memcached_strerror(pMemc, rc));
        memcached_free(pMemc);
        return -1;
    }
    
    // ========== 去重检查：5分钟内相同的告警只保存一次 ==========
    // 生成去重key：基于 trap_type + source_ip + 关键字段（ap_id/ap_mac） + 时间窗口（5分钟）
    // 时间窗口：每5分钟一个窗口（timestamp / 300）
    time_t timeWindow = trap.receiveTime / 300;  // 5分钟窗口（300秒）
    
    // 提取关键字段用于去重（优先使用ap_id，其次ap_mac，最后使用trap_oid）
    string dedupKey = "ac_trap_dedup_" + trap.sourceIp + "_" + trap.trapType + "_";
    if (trap.parsedData.find("ap_id") != trap.parsedData.end() && 
        !trap.parsedData.at("ap_id").empty())
    {
        dedupKey += trap.parsedData.at("ap_id");
    }
    else if (trap.parsedData.find("ap_mac") != trap.parsedData.end() && 
             !trap.parsedData.at("ap_mac").empty())
    {
        dedupKey += trap.parsedData.at("ap_mac");
    }
    else
    {
        // 如果没有关键字段，使用trap_oid作为去重标识
        dedupKey += trap.trapOid;
    }
    dedupKey += "_" + std::to_string(timeWindow);
    
    // 检查是否已存在（去重标记过期时间6分钟，略大于时间窗口）
    size_t dedupValueLen = 0;
    uint32_t dedupFlags = 0;
    char* dedupValue = memcached_get(pMemc, dedupKey.c_str(), dedupKey.length(),&dedupValueLen, &dedupFlags, &rc);
    
    if (rc == MEMCACHED_SUCCESS && dedupValue != NULL)
    {
        // 已存在，跳过保存（去重）
        free(dedupValue);
        TrapInfoLog("Trap重复告警已跳过（去重）: 来源=%s, 类型=%s, 时间窗口=%ld",trap.sourceIp.c_str(), trap.trapType.c_str(), timeWindow);
        memcached_free(pMemc);
        return 0;  // 返回成功，但不保存
    }
    
    if (dedupValue != NULL)
        free(dedupValue);
    
    // 设置去重标记（过期时间6分钟）
    string dedupMark = "1";
    rc = memcached_set(pMemc, dedupKey.c_str(), dedupKey.length(),dedupMark.c_str(), dedupMark.length(),360, 0);  // 6分钟过期（360秒）
    
    if (rc != MEMCACHED_SUCCESS && rc != MEMCACHED_STORED)
    {
        TrapErrorLog("设置去重标记失败: key=%s, error=%s",dedupKey.c_str(), memcached_strerror(pMemc, rc));
        // 继续保存，不因为去重标记失败而放弃
    }
    
    // ========== 保存Trap数据 ==========
    
    // 转换为JSON
    string jsonData = ToJson(trap);
    
    // 1. 保存单条Trap详情
    char timestampStr[32];
    snprintf(timestampStr, sizeof(timestampStr), "%ld", trap.receiveTime);
    string detailKey = "ac_trap_" + trap.sourceIp + "_" + timestampStr;
    
    rc = memcached_set(pMemc, detailKey.c_str(), detailKey.length(),jsonData.c_str(), jsonData.length(),86400, 0);  // 缓存24小时
    
    if (rc != MEMCACHED_SUCCESS && rc != MEMCACHED_STORED)
    {
        TrapErrorLog("保存Trap详情失败: key=%s, error=%s",detailKey.c_str(), memcached_strerror(pMemc, rc));
    }
    
    // 2. 追加到Trap列表
    string listKey = "ac_trap_list_" + trap.sourceIp;
    string listValue = jsonData + "\n";
    
    rc = memcached_append(pMemc, listKey.c_str(), listKey.length(),listValue.c_str(), listValue.length(),86400, 0);
    
    if (rc == MEMCACHED_NOTSTORED)
    {
        // 如果key不存在，使用set创建
        rc = memcached_set(pMemc, listKey.c_str(), listKey.length(),listValue.c_str(), listValue.length(),86400, 0);
    }
    
    if (rc != MEMCACHED_SUCCESS && rc != MEMCACHED_STORED)
    {
        TrapErrorLog("追加Trap到列表失败: key=%s, error=%s",listKey.c_str(), memcached_strerror(pMemc, rc));
        memcached_free(pMemc);
        return -1;  // 保存失败，返回错误
    }
    else
    {
        // 无论OID是否解析成功，都记录保存成功
        if (trap.trapOid == "unknown" || trap.trapType == "huawei-unknown")
        {
            TrapInfoLog("Trap已保存到缓存（OID未解析）: 来源=%s, OID=%s, 类型=%s",trap.sourceIp.c_str(), trap.trapOid.c_str(), trap.trapType.c_str());
        }
        else
        {
            TrapInfoLog("Trap已保存到缓存: 来源=%s, OID=%s, 类型=%s",trap.sourceIp.c_str(), trap.trapOid.c_str(), trap.trapType.c_str());
        }
    }
    
    memcached_free(pMemc);
    return 0;
}

string CTrapReceiver::ToJson(const TrapInfo& trap) const
{
    ostringstream oss;
    oss << "{";
    oss << "\"source_ip\":\"" << trap.sourceIp << "\",";
    if (!trap.hostname.empty())
    {
        oss << "\"hostname\":\"" << trap.hostname << "\",";
    }
    oss << "\"receive_time\":\"" << trap.receiveTimeStr << "\",";
    oss << "\"receive_timestamp\":" << trap.receiveTime << ",";
    oss << "\"trap_oid\":\"" << trap.trapOid << "\",";
    oss << "\"trap_type\":\"" << trap.trapType << "\",";
    oss << "\"enterprise_oid\":\"" << trap.enterpriseOid << "\",";
    
    // 变量绑定
    oss << "\"vars\":{";
    bool first = true;
    for (map<string, string>::const_iterator it = trap.vars.begin(); 
         it != trap.vars.end(); ++it)
    {
        if (!first) oss << ",";
        oss << "\"" << it->first << "\":\"" << it->second << "\"";
        first = false;
    }
    oss << "},";
    
    // 解析后的业务数据
    oss << "\"parsed_data\":{";
    first = true;
    for (map<string, string>::const_iterator it = trap.parsedData.begin(); 
         it != trap.parsedData.end(); ++it)
    {
        if (!first) oss << ",";
        oss << "\"" << it->first << "\":\"" << it->second << "\"";
        first = false;
    }
    oss << "}";
    
    oss << "}";
    return oss.str();
}

// 辅助函数：查找VarBinds中的值（通过字段名或OID模式）
string CTrapReceiver::FindVarBindValue(const map<string, string>& vars,const string& fieldName,const string& defaultValue) const
{
    // 首先尝试精确匹配字段名（不区分大小写）
    for (map<string, string>::const_iterator it = vars.begin(); it != vars.end(); ++it)
    {
        string oid = it->first;
        string value = it->second;
        
        // 检查OID是否包含字段名（不区分大小写）
        string oidLower = oid;
        string fieldLower = fieldName;
        for (size_t i = 0; i < oidLower.length(); ++i) oidLower[i] = tolower(oidLower[i]);
        for (size_t i = 0; i < fieldLower.length(); ++i) fieldLower[i] = tolower(fieldLower[i]);
        
        if (oidLower.find(fieldLower) != string::npos)
        {
            return value;
        }
    }
    
    return defaultValue;
}

string CTrapReceiver::GetTrapTypeName(const string& trapOid) const
{
    // ========== IFNET模块Trap（网络接口相关）==========
    if (trapOid.find("1.3.6.1.4.1.2011.5.25.41.3.5") != string::npos)
        return "if-flow-down";
    else if (trapOid.find("1.3.6.1.4.1.2011.5.25.41.3.6") != string::npos)
        return "if-flow-up";
    else if (trapOid.find("1.3.6.1.4.1.2011.5.25.41.4.5") != string::npos)
        return "if-input-rate-rising";
    else if (trapOid.find("1.3.6.1.4.1.2011.5.25.41.4.6") != string::npos)
        return "if-input-rate-resume";
    else if (trapOid.find("1.3.6.1.4.1.2011.5.25.41.4.7") != string::npos)
        return "if-output-rate-rising";
    else if (trapOid.find("1.3.6.1.4.1.2011.5.25.41.4.8") != string::npos)
        return "if-output-rate-resume";
    
    // ========== ENTITY模块Trap（实体/设备相关）==========
    else if (trapOid.find("1.3.6.1.4.1.2011.5.25.219.2.10.13") != string::npos)
        return "entity-brd-temp-alarm";
    else if (trapOid.find("1.3.6.1.4.1.2011.5.25.219.2.10.14") != string::npos)
        return "entity-brd-temp-resume";
    else if (trapOid.find("1.3.6.1.4.1.2011.5.25.219.2.10.15") != string::npos)
        return "entity-brd-temp-fatal";
    else if (trapOid.find("1.3.6.1.4.1.2011.5.25.219.2.14.1") != string::npos)
        return "entity-cpu-rising";
    else if (trapOid.find("1.3.6.1.4.1.2011.5.25.219.2.14.2") != string::npos)
        return "entity-cpu-resume";
    else if (trapOid.find("1.3.6.1.4.1.2011.5.25.219.2.14.5") != string::npos)
        return "entity-fwd-cpu-rising";
    else if (trapOid.find("1.3.6.1.4.1.2011.5.25.219.2.14.6") != string::npos)
        return "entity-fwd-cpu-resume";
    else if (trapOid.find("1.3.6.1.4.1.2011.5.25.219.2.15.1") != string::npos)
        return "entity-mem-rising";
    else if (trapOid.find("1.3.6.1.4.1.2011.5.25.219.2.15.2") != string::npos)
        return "entity-mem-resume";
    else if (trapOid.find("1.3.6.1.4.1.2011.5.25.219.2.24.1") != string::npos)
        return "entity-disk-rising";
    else if (trapOid.find("1.3.6.1.4.1.2011.5.25.219.2.24.2") != string::npos)
        return "entity-disk-resume";
    
    // ========== 标准SNMP通用Trap ==========
    else if (trapOid.find("1.3.6.1.6.3.1.1.5.3") != string::npos || 
             trapOid.find("linkDown") != string::npos)
        return "link-down";
    else if (trapOid.find("1.3.6.1.6.3.1.1.5.4") != string::npos || 
             trapOid.find("linkUp") != string::npos)
        return "link-up";
    
    // ========== 无线用户数告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.12.3.1.7") != string::npos)
        return "sta-num-warning";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.12.3.1.8") != string::npos)
        return "sta-num-warning-restore";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.12.3.1.9") != string::npos)
        return "sta-num-max";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.12.3.1.10") != string::npos)
        return "sta-num-max-restore";
    
    // ========== AP通讯故障告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.1") != string::npos)
        return "ap-fault";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.2") != string::npos)
        return "ap-normal";
    
    // ========== AP冷启动/热启动Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.30") != string::npos)
        return "ap-cold-boot";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.31") != string::npos)
        return "ap-cold-boot-restore";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.32") != string::npos)
        return "ap-hot-boot";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.33") != string::npos)
        return "ap-hot-boot-restore";
    
    // ========== 未认证AP告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.5") != string::npos)
        return "unauthorized-ap-exist";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.6") != string::npos)
        return "unauthorized-ap-clear";
    
    // ========== AP CPU/内存利用率告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.7") != string::npos)
        return "ap-cpu-overload";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.8") != string::npos)
        return "ap-cpu-overload-restore";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.9") != string::npos)
        return "ap-memory-overload";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.10") != string::npos)
        return "ap-memory-overload-restore";
    
    // ========== AP用户数已满告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.11") != string::npos)
        return "ap-sta-full";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.12") != string::npos)
        return "ap-sta-full-recover";
    
    // ========== AP温度告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.17") != string::npos)
        return "ap-temp-too-low";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.18") != string::npos)
        return "ap-temp-too-low-restore";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.19") != string::npos)
        return "ap-temp-too-high";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.20") != string::npos)
        return "ap-temp-too-high-restore";
    
    // ========== AP光模块接收功率告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.21") != string::npos)
        return "ap-optical-rx-power-too-high";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.22") != string::npos)
        return "ap-optical-rx-power-too-high-restore";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.23") != string::npos)
        return "ap-optical-rx-power-too-low";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.24") != string::npos)
        return "ap-optical-rx-power-too-low-restore";
    
    // ========== AP光模块温度告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.25") != string::npos)
        return "ap-optical-temp-too-high";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.26") != string::npos)
        return "ap-optical-temp-too-high-restore";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.27") != string::npos)
        return "ap-optical-temp-too-low";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.28") != string::npos)
        return "ap-optical-temp-too-low-restore";
    
    // ========== AP光模块发送功率告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.53") != string::npos)
        return "ap-optical-tx-power-too-high";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.54") != string::npos)
        return "ap-optical-tx-power-too-high-restore";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.55") != string::npos)
        return "ap-optical-tx-power-too-low";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.56") != string::npos)
        return "ap-optical-tx-power-too-low-restore";
    
    // ========== AP子固件版本不匹配告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.86") != string::npos)
        return "ap-sub-firmware-mismatch";
    
    // ========== AP风扇故障告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.58") != string::npos)
        return "ap-fan-invalid";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.59") != string::npos)
        return "ap-fan-invalid-restore";
    
    // ========== AP存储卡拔出告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.60") != string::npos)
        return "ap-storage-dev-remove";
    
    // ========== AP光模块功能异常告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.72") != string::npos)
        return "ap-optical-invalid";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.73") != string::npos)
        return "ap-optical-invalid-restore";
    
    // ========== AP供电不足告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.94") != string::npos)
        return "ap-power-insufficient";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.95") != string::npos)
        return "ap-power-insufficient-resume";
    
    // ========== 端口安全MAC地址告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.96") != string::npos)
        return "port-vlan-secure-mac";
    
    // ========== AP网线质量告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.91") != string::npos)
        return "ap-cable-snr-normal";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.92") != string::npos)
        return "ap-cable-snr-abnormal";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.93") != string::npos)
        return "ap-cable-snr-detect-not-support";
    
    // ========== AP版本不配套告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.102") != string::npos)
        return "ap-version-not-recommended";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.103") != string::npos)
        return "ap-version-not-recommended-restore";
    
    // ========== AP磁盘空间告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.104") != string::npos)
        return "ap-disk-overload";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.105") != string::npos)
        return "ap-disk-overload-restore";
    
    // ========== AP供电不足受限模式告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.106") != string::npos)
        return "ap-power-limited";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.107") != string::npos)
        return "ap-power-limited-resume";
    
    // ========== AP IP地址冲突告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.108") != string::npos)
        return "ap-ip-conflict";
    
    // ========== AP在线个数告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.109") != string::npos)
        return "ap-num-reach-max";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.110") != string::npos)
        return "ap-num-reach-max-resume";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.111") != string::npos)
        return "ap-num-reach-warning";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.112") != string::npos)
        return "ap-num-reach-warning-resume";
    
    // ========== AP风扇模块告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.117") != string::npos)
        return "ap-fan-remove";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.118") != string::npos)
        return "ap-fan-insert";
    
    // ========== AP电源模块告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.119") != string::npos)
        return "ap-power-remove";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.120") != string::npos)
        return "ap-power-insert";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.121") != string::npos)
        return "ap-power-fail";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.122") != string::npos)
        return "ap-power-fail-resume";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.123") != string::npos)
        return "ap-power-invalid";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.124") != string::npos)
        return "ap-power-invalid-resume";
    
    // ========== AP光模块电压告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.127") != string::npos)
        return "ap-optical-voltage-too-high";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.128") != string::npos)
        return "ap-optical-voltage-too-high-restore";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.129") != string::npos)
        return "ap-optical-voltage-too-low";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.130") != string::npos)
        return "ap-optical-voltage-too-low-restore";
    
    // ========== AP光模块电流告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.132") != string::npos)
        return "ap-optical-current-too-high-restore";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.133") != string::npos)
        return "ap-optical-current-too-low";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13.1.1.134") != string::npos)
        return "ap-optical-current-too-low-restore";
    
    // ========== 射频信号环境告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.16.1.1.1.1") != string::npos)
        return "radio-channel-changed";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.16.1.1.1.2") != string::npos)
        return "radio-signal-env-deterioration";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.16.1.1.1.3") != string::npos)
        return "radio-signal-env-resume";
    
    // ========== 同频/邻频AP干扰告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.16.1.1.1.5") != string::npos)
        return "ap-co-interf-detected";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.16.1.1.1.6") != string::npos)
        return "ap-co-interf-clear";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.16.1.1.1.7") != string::npos)
        return "ap-neighbor-interf-detected";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.16.1.1.1.8") != string::npos)
        return "ap-neighbor-interf-clear";
    
    // ========== 终端干扰告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.16.1.1.1.9") != string::npos)
        return "sta-interf-detected";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.16.1.1.1.10") != string::npos)
        return "sta-interf-clear";
    
    // ========== 其他设备干扰告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.16.1.1.1.11") != string::npos)
        return "other-device-interf-detected";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.16.1.1.1.12") != string::npos)
        return "other-device-interf-clear";
    
    // ========== 射频最大用户数能力告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.16.1.1.1.30") != string::npos)
        return "radio-max-sta-reach";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.16.1.1.1.31") != string::npos)
        return "radio-max-sta-clear";
    
    // ========== 用户IP地址与网关地址冲突告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.18.1.1.1.12") != string::npos)
        return "station-ip-conflict";
    
    // ========== WIDS非Wi-Fi设备检测告警Trap ==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.15.1.1.15") != string::npos)
        return "wids-non-wifi-device";
    
    // ========== 其他WLAN事件（通用匹配）==========
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.14") != string::npos)
        return "wlan-event";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.13") != string::npos)
        return "wlan-ap-event";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.12") != string::npos)
        return "wlan-stats-event";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.15") != string::npos)
        return "wlan-wids-event";  // WIDS（无线入侵检测系统）事件
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.16") != string::npos)
        return "wlan-radio-event";
    else if (trapOid.find("1.3.6.1.4.1.2011.6.139.18") != string::npos)
        return "wlan-station-event";
    else if (trapOid.find("1.3.6.1.4.1.2011.5.25") != string::npos)
        return "huawei-entity-event";
    else if (trapOid.find("1.3.6.1.4.1.2011.5.25.41") != string::npos)
        return "huawei-ifnet-event";
    
    // ========== 未识别的OID，记录日志 ==========
    if (trapOid.find("1.3.6.1.4.1.2011") != string::npos)
    {
        // 华为设备OID，但未识别，记录日志
        TrapInfoLog("未识别的华为Trap OID: %s", trapOid.c_str());
        return "huawei-unknown";
    }
    
    return "unknown";
}

// 从BER编码的SNMP消息中提取OID（简单的ASN.1解析）
string CTrapReceiver::ExtractOidFromBer(const unsigned char* data, size_t dataLen) const
{
    if (data == NULL || dataLen < 10)
    {
        return "unknown";
    }
    
    // SNMP v2c Trap消息的BER编码结构：
    // SEQUENCE (0x30) {
    //   INTEGER version (0x02)
    //   OCTET STRING community (0x04)
    //   PDU (0xa7 for SNMPv2-Trap-PDU) {
    //     INTEGER request-id
    //     INTEGER error-status
    //     INTEGER error-index
    //     SEQUENCE VarBindList {
    //       SEQUENCE VarBind {
    //         OID snmpTrapOID.0 (0x06)
    //         OID trap-oid-value (0x06)
    //       }
    //     }
    //   }
    // }
    
    // 简化方法：在数据中查找OID类型标记 (0x06) 并尝试解析
    for (size_t i = 0; i < dataLen - 5; ++i)
    {
        // 查找OID类型标记
        if (data[i] == 0x06)
        {
            // 找到OID，尝试解析
            size_t oidPos = i + 1;
            
            if (oidPos >= dataLen)
            {
                continue;
            }
            
            // 读取OID长度
            size_t oidLen = data[oidPos];
            oidPos++;
            
            if (oidLen > 50 || oidPos + oidLen > dataLen)  // 限制OID长度
            {
                continue;
            }
            
            // 尝试解析OID
            size_t consumed = 0;
            string oidStr = ParseBerOid(data + oidPos, oidLen, consumed);
            
            // 检查是否是华为AC的Trap OID
            if (!oidStr.empty() && oidStr.find("1.3.6.1.4.1.2011.6.139") == 0)
            {
                return oidStr;
            }
        }
    }
    
    return "unknown";
}

// 解析BER编码的OID并转换为字符串
string CTrapReceiver::ParseBerOid(const unsigned char* data, size_t dataLen, size_t& consumed) const
{
    if (data == NULL || dataLen == 0)
    {
        consumed = 0;
        return "";
    }
    
    vector<unsigned long> oidComponents;
    size_t pos = 0;
    
    // 解析第一个字节（前两个数字）
    if (pos >= dataLen)
    {
        consumed = 0;
        return "";
    }
    
    unsigned char firstByte = data[pos++];
    oidComponents.push_back(firstByte / 40);
    oidComponents.push_back(firstByte % 40);
    
    // 解析后续字节（每个数字可能跨多个字节）
    unsigned long currentValue = 0;
    while (pos < dataLen)
    {
        unsigned char byte = data[pos++];
        currentValue = (currentValue << 7) | (byte & 0x7f);
        
        if ((byte & 0x80) == 0)
        {
            // 这个数字结束了
            oidComponents.push_back(currentValue);
            currentValue = 0;
        }
    }
    
    // 转换为字符串
    if (oidComponents.empty())
    {
        consumed = pos;
        return "";
    }
    
    ostringstream oss;
    for (size_t i = 0; i < oidComponents.size(); ++i)
    {
        if (i > 0)
        {
            oss << ".";
        }
        oss << oidComponents[i];
    }
    
    consumed = pos;
    return oss.str();
}

// 解析BER长度字段
int CTrapReceiver::ParseBerLength(const unsigned char* data, size_t dataLen, size_t& pos, size_t& length) const
{
    if (pos >= dataLen)
    {
        return -1;
    }
    
    unsigned char firstByte = data[pos++];
    
    if ((firstByte & 0x80) == 0)
    {
        // 短格式：长度直接编码在第一个字节
        length = firstByte;
        return 0;
    }
    else
    {
        // 长格式：后续字节表示长度
        size_t lengthBytes = firstByte & 0x7f;
        if (lengthBytes == 0 || lengthBytes > 4)
        {
            return -1;  // 无效的长度格式
        }
        
        if (pos + lengthBytes > dataLen)
        {
            return -1;
        }
        
        length = 0;
        for (size_t i = 0; i < lengthBytes; ++i)
        {
            length = (length << 8) | data[pos++];
        }
        
        return 0;
    }
}

// 解析BER编码的值（OCTET STRING, INTEGER, OID等）
string CTrapReceiver::ParseBerValue(const unsigned char* data, size_t dataLen, size_t& pos, unsigned char& valueType) const
{
    if (pos >= dataLen)
    {
        return "";
    }
    
    valueType = data[pos++];
    
    size_t valueLength = 0;
    if (ParseBerLength(data, dataLen, pos, valueLength) != 0)
    {
        return "";
    }
    
    if (pos + valueLength > dataLen)
    {
        return "";
    }
    
    string result;
    
    if (valueType == 0x04)  // OCTET STRING
    {
        // 字符串值
        result = string((const char*)(data + pos), valueLength);
    }
    else if (valueType == 0x02)  // INTEGER
    {
        // 整数：转换为字符串
        if (valueLength > 0)
        {
            long long intValue = 0;
            bool negative = false;
            if (valueLength > 0 && (data[pos] & 0x80))
            {
                negative = true;
            }
            
            for (size_t i = 0; i < valueLength; ++i)
            {
                intValue = (intValue << 8) | data[pos + i];
            }
            
            if (negative && valueLength < 8)
            {
                // 符号扩展
                intValue |= ((long long)-1) << (valueLength * 8);
            }
            
            char buf[64];
            snprintf(buf, sizeof(buf), "%lld", intValue);
            result = string(buf);
        }
    }
    else if (valueType == 0x06)  // OID
    {
        // OID：解析为点分十进制字符串
        size_t consumed = 0;
        result = ParseBerOid(data + pos, valueLength, consumed);
    }
    else
    {
        // 其他类型：转换为十六进制字符串
        ostringstream oss;
        for (size_t i = 0; i < valueLength && i < 32; ++i)  // 限制长度
        {
            char hex[4];
            snprintf(hex, sizeof(hex), "%02x", data[pos + i]);
            oss << hex;
        }
        result = oss.str();
    }
    
    pos += valueLength;
    return result;
}

// 完整BER解析：从SNMP消息中提取所有信息（主机名、VarBinds等）
int CTrapReceiver::ParseBerSnmpMessage(const unsigned char* data, size_t dataLen, TrapInfo& trap) const
{
    if (data == NULL || dataLen < 10)
    {
        return -1;
    }
    
    size_t pos = 0;
    
    // 1. 解析外层SEQUENCE (0x30)
    if (pos >= dataLen || data[pos] != 0x30)
    {
        return -1;
    }
    pos++;
    
    size_t seqLength = 0;
    if (ParseBerLength(data, dataLen, pos, seqLength) != 0)
    {
        return -1;
    }
    
    // 2. 解析版本号 (INTEGER, 0x02)
    if (pos >= dataLen || data[pos] != 0x02)
    {
        return -1;
    }
    pos++;
    
    size_t verLength = 0;
    if (ParseBerLength(data, dataLen, pos, verLength) != 0 || verLength != 1)
    {
        return -1;
    }
    pos++;  // 跳过版本号值
    
    // 3. 解析社区字符串 (OCTET STRING, 0x04) - 这就是主机名
    if (pos >= dataLen || data[pos] != 0x04)
    {
        return -1;
    }
    pos++;
    
    size_t commLength = 0;
    if (ParseBerLength(data, dataLen, pos, commLength) != 0)
    {
        return -1;
    }
    
    if (pos + commLength <= dataLen)
    {
        // 提取社区字符串作为主机名
        trap.hostname = string((const char*)(data + pos), commLength);
    }
    pos += commLength;
    
    // 4. 解析PDU类型
    if (pos >= dataLen)
    {
        return -1;
    }
    
    unsigned char pduType = data[pos++];
    if (pduType != 0xa7)  // SNMPv2-Trap-PDU
    {
        // 不是SNMPv2-Trap，但继续解析
    }
    
    size_t pduLength = 0;
    if (ParseBerLength(data, dataLen, pos, pduLength) != 0)
    {
        return -1;
    }
    
    size_t pduEnd = pos + pduLength;
    
    // 5. 跳过request-id, error-status, error-index (都是INTEGER)
    for (int i = 0; i < 3 && pos < pduEnd; ++i)
    {
        if (pos >= dataLen || data[pos] != 0x02)
        {
            break;
        }
        pos++;
        size_t intLength = 0;
        if (ParseBerLength(data, dataLen, pos, intLength) != 0)
        {
            break;
        }
        pos += intLength;
    }
    
    // 6. 解析VarBindList (SEQUENCE, 0x30)
    if (pos >= dataLen || data[pos] != 0x30)
    {
        return -1;
    }
    pos++;
    
    size_t vblLength = 0;
    if (ParseBerLength(data, dataLen, pos, vblLength) != 0)
    {
        return -1;
    }
    
    size_t vblEnd = pos + vblLength;
    
    // 7. 解析每个VarBind
    int varBindCount = 0;
    while (pos < vblEnd && pos < dataLen)
    {
        // VarBind是SEQUENCE
        if (data[pos] != 0x30)
        {
            break;
        }
        pos++;
        
        size_t vbLength = 0;
        if (ParseBerLength(data, dataLen, pos, vbLength) != 0)
        {
            break;
        }
        
        size_t vbEnd = pos + vbLength;
        
        // 解析OID (0x06)
        if (pos >= vbEnd || pos >= dataLen || data[pos] != 0x06)
        {
            pos = vbEnd;
            continue;
        }
        
        size_t oidPos = pos;  // 保存OID解析前的位置
        unsigned char oidType = 0;
        string oidStr = ParseBerValue(data, dataLen, pos, oidType);
        if (oidStr.empty() || pos > vbEnd)
        {
            pos = vbEnd;
            continue;
        }
        
        // 解析Value
        if (pos >= vbEnd || pos >= dataLen)
        {
            pos = vbEnd;
            continue;
        }
        
        size_t valuePos = pos;  // 保存Value解析前的位置
        unsigned char valueType = 0;
        string valueStr = ParseBerValue(data, dataLen, pos, valueType);
        if (pos > vbEnd)
        {
            // 如果解析超出范围，回退到vbEnd
            pos = vbEnd;
            continue;
        }
        
        // 存储VarBind
        trap.vars[oidStr] = valueStr;
        varBindCount++;
        
        // 检查是否是关键字段
        if (oidStr == "1.3.6.1.6.3.1.1.4.1.0" || oidStr.find("snmpTrapOID") != string::npos)
        {
            // 这是Trap OID
            trap.trapOid = valueStr;
            trap.trapType = GetTrapTypeName(trap.trapOid);
        }
        else if (oidStr == "1.3.6.1.6.3.1.1.4.3.0" || oidStr.find("snmpTrapEnterprise") != string::npos)
        {
            // 这是Enterprise OID
            trap.enterpriseOid = valueStr;
        }
        
        pos = vbEnd;
    }
    
    return 0;  // 解析成功
}

