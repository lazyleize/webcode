/************************************************************
 Desc:     SNMP Trap接收器 - 独立线程接收Trap并写入缓存
 Auth:     Auto
 Modify:
 data:     2025-12-08
 ***********************************************************/
#ifndef CTRAPRECEIVER_H_
#define CTRAPRECEIVER_H_

#include <string>
#include <map>
#include <ctime>
#include <pthread.h>
#include <snmp_pp/snmp_pp.h>

using namespace std;
using namespace Snmp_pp;

// Trap信息结构
struct TrapInfo
{
    string sourceIp;         // 来源IP（AC控制器IP）
    string hostname;         // 主机名（从社区字符串提取，如：HUAWEI-NAC）
    time_t receiveTime;      // 接收时间（Unix时间戳）
    string receiveTimeStr;   // 接收时间（字符串格式：YYYY-MM-DD HH:MM:SS）
    string trapOid;          // Trap OID（如：1.3.6.1.4.1.2011.6.139.14.1.1.1）
    string trapType;         // Trap类型（如：ap-online, ap-offline）
    string enterpriseOid;     // Enterprise OID
    map<string, string> vars; // Trap变量绑定（OID -> 值）
    map<string, string> parsedData; // 解析后的业务数据（如：ap_id, ap_name等）
    
    TrapInfo() : receiveTime(0) {}
};

class CTrapReceiver
{
public:
    CTrapReceiver();
    virtual ~CTrapReceiver();
    
    // 初始化（读取配置）
    int Init();
    
    // 启动Trap接收线程
    int Start();
    
    // 停止Trap接收
    void Stop();
    
    // 获取线程ID
    pthread_t GetThreadId() const { return m_threadId; }
    
    // 线程入口函数（静态）
    static void* ThreadProc(void* arg);

private:
    // 线程主循环（接收Trap）
    void* Run();
    
    // 接收并解析Trap消息
    int ReceiveAndParseTrap(TrapInfo& trap);
    
    // 解析Trap PDU
    int ParseTrapPdu(const Pdu& pdu, const UdpAddress& fromAddr, TrapInfo& trap);
    
    // 解析业务数据（根据Trap OID提取业务字段）
    int ParseBusinessData(TrapInfo& trap);
    
    // 保存Trap到Memcached缓存
    int SaveToCache(const TrapInfo& trap);
    
    // JSON序列化
    string ToJson(const TrapInfo& trap) const;
    
    // 获取Trap类型名称（根据OID）
    string GetTrapTypeName(const string& trapOid) const;
    
    // 查找VarBinds中的值（通过字段名或OID模式）
    string FindVarBindValue(const map<string, string>& vars, 
                           const string& fieldName, 
                           const string& defaultValue = "") const;
    
    // 辅助函数：从BER编码的SNMP消息中提取OID（简单的ASN.1解析）
    string ExtractOidFromBer(const unsigned char* data, size_t dataLen) const;
    
    // 辅助函数：解析BER编码的OID并转换为字符串
    string ParseBerOid(const unsigned char* data, size_t dataLen, size_t& consumed) const;
    
    // 完整BER解析：从SNMP消息中提取所有信息（主机名、VarBinds等）
    int ParseBerSnmpMessage(const unsigned char* data, size_t dataLen, TrapInfo& trap) const;
    
    // 解析BER长度字段
    int ParseBerLength(const unsigned char* data, size_t dataLen, size_t& pos, size_t& length) const;
    
    // 解析BER编码的值（OCTET STRING, INTEGER, OID等）
    string ParseBerValue(const unsigned char* data, size_t dataLen, size_t& pos, unsigned char& valueType) const;
    
    // 异常安全的接收循环（捕获所有异常）
    void SafeReceiveLoop();

private:
    bool m_stopped;              // 停止标志
    bool m_initialized;          // 是否已初始化
    pthread_t m_threadId;        // 线程ID
    int m_trapPort;              // Trap监听端口（默认162）
    
    // Memcached缓存配置
    string m_cacheIp;            // 缓存服务器IP
    int m_cachePort;             // 缓存服务器端口
    
    // SNMP对象
    Snmp* m_snmp;                // SNMP对象指针
    int m_snmpStatus;             // SNMP状态
    
    // UDP Socket（用于直接接收Trap）
    int m_udpSocket;              // UDP socket文件描述符
    
    // 线程同步
    pthread_mutex_t m_mutex;      // 互斥锁（保护共享资源）
    
    // 统计信息
    int m_totalReceived;          // 总接收数
    int m_totalSaved;             // 总保存数
    int m_totalErrors;            // 总错误数
};

#endif /* CTRAPRECEIVER_H_ */

