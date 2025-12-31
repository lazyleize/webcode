/*
 * @file    CSnmpClient.cpp
 * @brief   SNMP操作封装类实现
 * @author  Auto
 */
#include "CSnmpClient.h"
#include <sstream>
#include <cstring>

CSnmpClient::CSnmpClient(const string& ip, int port, 
                         const string& community, 
                         snmp_version version)
    : m_ip(ip)
    , m_port(port)
    , m_community(community)
    , m_version(version)
    , m_timeout(3000)
    , m_retries(1)
    , m_snmp(NULL)
    , m_target(NULL)
    , m_address(NULL)
    , m_lastError("")
    , m_lastErrorCode(0)
    , m_initialized(false)
{
}

CSnmpClient::~CSnmpClient()
{
    Cleanup();
}

int CSnmpClient::Init()
{
    if (m_initialized)
    {
        return 0;
    }

    // 初始化SNMP++库
    Snmp::socket_startup();
    
    // 禁用SNMP++库的调试输出
    // 注意：不同版本的snmp_pp库API可能不同
    // 如果编译错误，可以注释掉这部分代码，调试信息不会影响功能
    // 或者通过重定向stderr来过滤调试信息
    #if 0
    // 尝试禁用日志（根据实际库版本调整）
    // DefaultLog::set_log_level(LOG_NONE);
    // DefaultLog::set_log_status(LOG_TYPE_ALL, false);
    #endif

    if (CreateSnmp() != 0)
    {
        return -1;
    }

    if (CreateTarget() != 0)
    {
        return -1;
    }

    m_initialized = true;
    return 0;
}

void CSnmpClient::Cleanup()
{
    if (m_target)
    {
        delete m_target;
        m_target = NULL;
    }

    if (m_address)
    {
        delete m_address;
        m_address = NULL;
    }

    if (m_snmp)
    {
        delete m_snmp;
        m_snmp = NULL;
    }

    if (m_initialized)
    {
        Snmp::socket_cleanup();
        m_initialized = false;
    }
}

int CSnmpClient::CreateSnmp()
{
    int status = 0;
    m_snmp = new Snmp(status);
    
    if (status != SNMP_CLASS_SUCCESS)
    {
        SetError("创建SNMP对象失败", status);
        delete m_snmp;
        m_snmp = NULL;
        return -1;
    }

    return 0;
}

int CSnmpClient::CreateTarget()
{
    m_address = new UdpAddress(m_ip.c_str());
    if (!m_address->valid())
    {
        SetError("无效的IP地址: " + m_ip);
        delete m_address;
        m_address = NULL;
        return -1;
    }
    
    m_address->set_port(m_port);

    m_target = new CTarget(*m_address);
    m_target->set_version(m_version);
    m_target->set_readcommunity(m_community.c_str());
    m_target->set_writecommunity(m_community.c_str());
    m_target->set_timeout(m_timeout);
    m_target->set_retry(m_retries);

    return 0;
}

int CSnmpClient::Get(const string& oid, string& value)
{
    value.clear();

    if (!m_initialized && Init() != 0)
    {
        return -1;
    }

    Oid oidObj(oid.c_str());
    if (!oidObj.valid())
    {
        SetError("无效的OID: " + oid);
        return -1;
    }

    Pdu pdu;
    pdu += oidObj;

    int status = m_snmp->get(pdu, *m_target);
    
    if (status != SNMP_CLASS_SUCCESS)
    {
        SetError(m_snmp->error_msg(status), status);
        return -1;
    }

    if (pdu.get_error_status() != SNMP_CLASS_SUCCESS)
    {
        ostringstream oss;
        oss << "SNMP GET失败，错误码: " << pdu.get_error_status();
        SetError(oss.str(), pdu.get_error_status());
        return -1;
    }

    Vb vb;
    if (pdu.get_vb_count() > 0)
    {
        pdu.get_vb(vb, 0);
        value = vb.get_printable_value();
        return 0;
    }

    SetError("SNMP响应中没有数据");
    return -1;
}

int CSnmpClient::GetInt(const string& oid, int64_t& value)
{
    string strValue;
    if (Get(oid, strValue) != 0)
    {
        return -1;
    }

    // 尝试转换为整数
    istringstream iss(strValue);
    int64_t tempValue = 0;
    if (!(iss >> tempValue))
    {
        SetError("无法将OID值转换为整数: " + strValue);
        return -1;
    }

    value = tempValue;
    return 0;
}

int CSnmpClient::GetUInt64(const string& oid, uint64_t& value)
{
    if (!m_initialized && Init() != 0)
    {
        return -1;
    }

    Oid oidObj(oid.c_str());
    if (!oidObj.valid())
    {
        SetError("无效的OID: " + oid);
        return -1;
    }

    Pdu pdu;
    Vb vb(oidObj);
    pdu += vb;

    int status = m_snmp->get(pdu, *m_target);
    
    if (status != SNMP_CLASS_SUCCESS)
    {
        SetError(m_snmp->error_msg(status), status);
        return -1;
    }

    if (pdu.get_error_status() != SNMP_CLASS_SUCCESS)
    {
        ostringstream oss;
        oss << "SNMP GET失败，错误码: " << pdu.get_error_status();
        SetError(oss.str(), pdu.get_error_status());
        return -1;
    }

    if (pdu.get_vb_count() > 0)
    {
        pdu.get_vb(vb, 0);
        value = vb.get_value(uint64_t());
        return 0;
    }

    SetError("SNMP响应中没有数据");
    return -1;
}

int CSnmpClient::GetBulk(const vector<string>& oids, map<string, string>& values)
{
    values.clear();

    if (!m_initialized && Init() != 0)
    {
        return -1;
    }

    Pdu pdu;
    for (size_t i = 0; i < oids.size(); ++i)
    {
        Oid oidObj(oids[i].c_str());
        if (!oidObj.valid())
        {
            SetError("无效的OID: " + oids[i]);
            return -1;
        }
        pdu += oidObj;
    }

    int status = m_snmp->get(pdu, *m_target);
    
    if (status != SNMP_CLASS_SUCCESS)
    {
        SetError(m_snmp->error_msg(status), status);
        return -1;
    }

    if (pdu.get_error_status() != SNMP_CLASS_SUCCESS)
    {
        ostringstream oss;
        oss << "SNMP GET失败，错误码: " << pdu.get_error_status();
        SetError(oss.str(), pdu.get_error_status());
        return -1;
    }

    for (int i = 0; i < pdu.get_vb_count() && i < (int)oids.size(); ++i)
    {
        Vb vb;
        pdu.get_vb(vb, i);
        values[oids[i]] = vb.get_printable_value();
    }

    return 0;
}

int CSnmpClient::Walk(const string& oid, map<string, string>& values)
{
    values.clear();

    if (!m_initialized && Init() != 0)
    {
        return -1;
    }

    Oid rootOid(oid.c_str());
    if (!rootOid.valid())
    {
        SetError("无效的OID: " + oid);
        return -1;
    }

    Oid currentOid = rootOid;
    int maxRepetitions = 50; // 最大重复次数，防止无限循环

    for (int i = 0; i < maxRepetitions; ++i)
    {
        Pdu pdu;
        Vb vb(currentOid);
        pdu += vb;

        int status = m_snmp->get_next(pdu, *m_target);
        
        if (status != SNMP_CLASS_SUCCESS)
        {
            if (i == 0)
            {
                SetError(m_snmp->error_msg(status), status);
                return -1;
            }
            // 后续失败可能是已经遍历完，不算错误
            break;
        }

        if (pdu.get_error_status() != SNMP_CLASS_SUCCESS)
        {
            if (i == 0)
            {
                ostringstream oss;
                oss << "SNMP GET_NEXT失败，错误码: " << pdu.get_error_status();
                SetError(oss.str(), pdu.get_error_status());
                return -1;
            }
            break;
        }

        Vb resultVb;
        pdu.get_vb(resultVb, 0);
        
        Oid resultOid;
        resultVb.get_oid(resultOid);

        // 检查是否还在OID树下
        if (resultOid.nCompare(rootOid.len(), rootOid) != 0)
        {
            // 已经超出OID树范围，结束遍历
            break;
        }

        string oidStr = resultOid.get_printable();
        string valueStr = resultVb.get_printable_value();
        values[oidStr] = valueStr;

        currentOid = resultOid;
    }

    return 0;
}

int CSnmpClient::Set(const string& oid, const string& value)
{
    if (!m_initialized && Init() != 0)
    {
        return -1;
    }

    Oid oidObj(oid.c_str());
    if (!oidObj.valid())
    {
        SetError("无效的OID: " + oid);
        return -1;
    }

    Pdu pdu;
    Vb vb(oidObj);
    OctetStr octetValue(value.c_str());
    vb.set_value(octetValue);
    pdu += vb;

    int status = m_snmp->set(pdu, *m_target);
    
    if (status != SNMP_CLASS_SUCCESS)
    {
        SetError(m_snmp->error_msg(status), status);
        return -1;
    }

    if (pdu.get_error_status() != SNMP_CLASS_SUCCESS)
    {
        ostringstream oss;
        oss << "SNMP SET失败，错误码: " << pdu.get_error_status();
        SetError(oss.str(), pdu.get_error_status());
        return -1;
    }

    return 0;
}

int CSnmpClient::SetInt(const string& oid, int value)
{
    if (!m_initialized && Init() != 0)
    {
        return -1;
    }

    Oid oidObj(oid.c_str());
    if (!oidObj.valid())
    {
        SetError("无效的OID: " + oid);
        return -1;
    }

    Pdu pdu;
    Vb vb(oidObj);
    vb.set_value(value);
    pdu += vb;

    int status = m_snmp->set(pdu, *m_target);
    
    if (status != SNMP_CLASS_SUCCESS)
    {
        SetError(m_snmp->error_msg(status), status);
        return -1;
    }

    if (pdu.get_error_status() != SNMP_CLASS_SUCCESS)
    {
        ostringstream oss;
        oss << "SNMP SET失败，错误码: " << pdu.get_error_status();
        SetError(oss.str(), pdu.get_error_status());
        return -1;
    }

    return 0;
}

string CSnmpClient::GetLastError() const
{
    return m_lastError;
}

int CSnmpClient::GetLastErrorCode() const
{
    return m_lastErrorCode;
}

void CSnmpClient::SetTimeout(int timeout)
{
    m_timeout = timeout;
    if (m_target)
    {
        m_target->set_timeout(timeout);
    }
}

void CSnmpClient::SetRetries(int retries)
{
    m_retries = retries;
    if (m_target)
    {
        m_target->set_retry(retries);
    }
}

bool CSnmpClient::IsReachable()
{
    // 尝试获取sysDescr来检查设备是否可达
    string value;
    return Get("1.3.6.1.2.1.1.1.0", value) == 0;
}

void CSnmpClient::SetError(const string& error, int code)
{
    m_lastError = error;
    m_lastErrorCode = code;
}

