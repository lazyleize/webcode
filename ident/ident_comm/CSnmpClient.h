/*
 * @file    CSnmpClient.h
 * @brief   SNMP操作封装类
 * @author  Auto
 */
#ifndef _CSNMPCLIENT_H_
#define _CSNMPCLIENT_H_

#include <string>
#include <vector>
#include <map>
#include <snmp_pp/snmp_pp.h>

using namespace Snmp_pp;
using namespace std;

/**
 * @brief SNMP客户端封装类
 * 
 * 提供SNMP GET、SET、WALK等常用操作的简单封装
 */
class CSnmpClient
{
public:
    /**
     * @brief 构造函数
     * @param ip 目标设备IP地址
     * @param port SNMP端口，默认161
     * @param community 团体名，默认"public"
     * @param version SNMP版本，默认version2c
     */
    CSnmpClient(const string& ip, int port = 161, 
                const string& community = "public", 
                snmp_version version = version2c);
    
    /**
     * @brief 析构函数
     */
    ~CSnmpClient();

    /**
     * @brief 初始化SNMP连接
     * @return 0成功，-1失败
     */
    int Init();

    /**
     * @brief 清理SNMP资源
     */
    void Cleanup();

    /**
     * @brief SNMP GET操作 - 获取单个OID的值
     * @param oid OID字符串，如"1.3.6.1.2.1.1.1.0"
     * @param value 输出参数，返回OID的值（字符串格式）
     * @return 0成功，-1失败
     */
    int Get(const string& oid, string& value);

    /**
     * @brief SNMP GET操作 - 获取单个OID的值（整数类型）
     * @param oid OID字符串
     * @param value 输出参数，返回OID的值（整数格式）
     * @return 0成功，-1失败
     */
    int GetInt(const string& oid, int64_t& value);

    /**
     * @brief SNMP GET操作 - 获取单个OID的值（无符号整数类型）
     * @param oid OID字符串
     * @param value 输出参数，返回OID的值（无符号整数格式）
     * @return 0成功，-1失败
     */
    int GetUInt64(const string& oid, uint64_t& value);

    /**
     * @brief SNMP GET操作 - 批量获取多个OID的值
     * @param oids OID字符串数组
     * @param values 输出参数，返回OID和值的映射
     * @return 0成功，-1失败
     */
    int GetBulk(const vector<string>& oids, map<string, string>& values);

    /**
     * @brief SNMP WALK操作 - 遍历OID树
     * @param oid 起始OID字符串
     * @param values 输出参数，返回所有子OID和值的映射
     * @return 0成功，-1失败
     */
    int Walk(const string& oid, map<string, string>& values);

    /**
     * @brief SNMP SET操作 - 设置OID的值（字符串）
     * @param oid OID字符串
     * @param value 要设置的值（字符串）
     * @return 0成功，-1失败
     */
    int Set(const string& oid, const string& value);

    /**
     * @brief SNMP SET操作 - 设置OID的值（整数）
     * @param oid OID字符串
     * @param value 要设置的值（整数）
     * @return 0成功，-1失败
     */
    int SetInt(const string& oid, int value);

    /**
     * @brief 获取最后一次错误信息
     * @return 错误信息字符串
     */
    string GetLastError() const;

    /**
     * @brief 获取最后一次错误码
     * @return SNMP错误码
     */
    int GetLastErrorCode() const;

    /**
     * @brief 设置超时时间（毫秒）
     * @param timeout 超时时间，默认3000毫秒
     */
    void SetTimeout(int timeout);

    /**
     * @brief 设置重试次数
     * @param retries 重试次数，默认1
     */
    void SetRetries(int retries);

    /**
     * @brief 检查设备是否可达
     * @return true可达，false不可达
     */
    bool IsReachable();

private:
    string      m_ip;           // 目标设备IP
    int         m_port;         // SNMP端口
    string      m_community;    // 团体名
    snmp_version m_version;     // SNMP版本
    int         m_timeout;      // 超时时间（毫秒）
    int         m_retries;      // 重试次数
    
    Snmp*       m_snmp;         // SNMP对象指针
    CTarget*    m_target;       // SNMP目标对象指针
    UdpAddress* m_address;      // UDP地址对象指针
    
    string      m_lastError;    // 最后一次错误信息
    int         m_lastErrorCode;// 最后一次错误码
    
    bool        m_initialized;  // 是否已初始化

    /**
     * @brief 设置错误信息
     */
    void SetError(const string& error, int code = -1);

    /**
     * @brief 创建SNMP对象
     */
    int CreateSnmp();

    /**
     * @brief 创建目标对象
     */
    int CreateTarget();
};

#endif // _CSNMPCLIENT_H_


