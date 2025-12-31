#include "CGetSNMP.h"
#include "CIdentRelayApi.h"
#include "curl/curl.h"
#include <iostream>
#include <snmp_pp/snmp_pp.h>

using namespace Snmp_pp;
int CGetSnmp::IdentCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map sessMap;
	CStr2Map inMap, outMap;

    Snmp::socket_startup();
    UdpAddress address("192.168.7.252");
    address.set_port(161);

    int status;
    Snmp snmp(status);

    // 设置 SNMP 版本和团体名
    CTarget ctarget(address);
    ctarget.set_version(version2c);  // 使用 SNMPv2c
    ctarget.set_readcommunity("xgd@#2025");

    // 设置要获取的 OID ---查看交换机信息
    //Oid oid("1.3.6.1.2.1.1.1.0");  // 这是 sysDescr OID

    // 设置要获取的 OID ---网口数量
    Oid oid("1.3.6.1.2.1.2.1.0");  // ifNumber OID

     // 设置要获取的 OID ---接口数量
    //Oid oid("1.3.6.1.2.1.2.1");  // ifNumber OID
    

    //TCP 连接数

     // 创建 PDU 对象
    Pdu pdu;
    pdu += oid;

    // 发起请求
    status = snmp.get(pdu, ctarget);

    // 处理响应
    if (status == SNMP_CLASS_SUCCESS) {
        for (int i = 0; i < pdu.get_vb_count(); i++) {
            Vb vb;
            pdu.get_vb(vb, i);
             DebugLog("接口数量:  %s  " , vb.get_printable_value()) ;
        }
    } else {
         DebugLog("SNMP++ Error: " , snmp.error_msg(status) );
    }

    // 清理 SNMP++ 库
    Snmp::socket_cleanup();
    
   
   

   //计算带宽占用率


   // 创建 OID 对象


/*
    int status;
    Snmp snmp(status);

    // 定义目标设备的 IP 和社区字符串
    UdpAddress address("192.168.8.87");
    address.set_port(161);
    const char* community = "Xgd@2024#88";

    // 创建 OID 对象
    Oid in_oid("1.3.6.1.2.1.2.2.1.10.1");  // 输入字节数 OID
    Oid out_oid("1.3.6.1.2.1.2.2.1.16.1"); // 输出字节数 OID
    Oid speed_oid("1.3.6.1.2.1.2.2.1.5.1"); // 接口带宽 OID

    Vb in_vb(in_oid);
    Vb out_vb(out_oid);
    Vb speed_vb(speed_oid);

    Pdu pdu;
    pdu += in_vb;
    pdu += out_vb;
    pdu += speed_vb;

    CTarget ctarget(address);
    ctarget.set_version(version2c);  // 使用 SNMPv2c
    ctarget.set_readcommunity(community);

    // 第一次获取数据
    snmp.get(pdu, ctarget);
    if (pdu.get_error_status() != SNMP_CLASS_SUCCESS) {
        std::cerr << "Error in first SNMP get: " << pdu.get_error_status() << std::endl;
        return 1;
    }

    uint64_t in_bytes_first = in_vb.get_value(uint64_t());
    uint64_t out_bytes_first = out_vb.get_value(uint64_t());
    uint64_t bandwidth_bps = speed_vb.get_value(uint64_t()); // 获取带宽

    // 打印获取到的值
    DebugLog("First Input Bytes: %llu\n", in_bytes_first);
    DebugLog("First Output Bytes: %llu\n", out_bytes_first);
    DebugLog("Bandwidth (bps): %llu\n", bandwidth_bps);

    // 等待一段时间（例如 5 秒）
    sleep(5);

    // 第二次获取数据
    pdu.clear();
    pdu += in_vb;
    pdu += out_vb;

    snmp.get(pdu, ctarget);
    if (pdu.get_error_status() != SNMP_CLASS_SUCCESS) {
        std::cerr << "Error in second SNMP get: " << pdu.get_error_status() << std::endl;
        return 1;
    }

    uint64_t in_bytes_second = in_vb.get_value(uint64_t());
    uint64_t out_bytes_second = out_vb.get_value(uint64_t());

    // 计算流量变化
    uint64_t in_bytes_diff = in_bytes_second - in_bytes_first;
    uint64_t out_bytes_diff = out_bytes_second - out_bytes_first;

    // 时间间隔
    double time_interval = 5; // 时间间隔为 5 秒

    // 计算带宽占用率
    double in_usage = (in_bytes_diff * 8.0) / (bandwidth_bps * time_interval) * 100; // 输入占用率
    double out_usage = (out_bytes_diff * 8.0) / (bandwidth_bps * time_interval) * 100; // 输出占用率

    // 使用 DebugLog 输出结果
    DebugLog("Input Bandwidth Usage: %.2f%%\n", in_usage);
    DebugLog("Output Bandwidth Usage: %.2f%%\n", out_usage);
*/

   /* Snmp::socket_startup();
    UdpAddress address("192.168.8.87");
    address.set_port(161);

    int status;
    Snmp snmp(status);

    // 设置 SNMP 版本和团体名
    CTarget ctarget(address);
    ctarget.set_version(version2c);  // 使用 SNMPv2c
    ctarget.set_readcommunity("Xgd@2024#88");

    // SNMP 会话参数
    const char* community = "Xgd@2024#88";
    const char* ip_address = "192.168.8.87";
    const char* ifInOctets = "1.3.6.1.2.1.2.2.1.10"; // ifInOctets OID
    const char* ifOutOctets = "1.3.6.1.2.1.2.2.1.16"; // ifOutOctets OID

    // 获取接口数量
    Vb vbx_ifCount("1.3.6.1.2.1.2.1"); // ifNumber OID
    Pdu pdu_ifCount;
    pdu_ifCount += vbx_ifCount;

    snmp.get(pdu_ifCount, ctarget);
    // 处理响应
    if (status == SNMP_CLASS_SUCCESS) {
        for (int i = 0; i < pdu_ifCount.get_vb_count(); i++) {
            pdu_ifCount.get_vb(vbx_ifCount, i);
            DebugLog("OID:%s" , vbx_ifCount.get_printable_value()) ;
        }
    }

    /*Vb vb;
    pdu.get_vb(vb, i);
    DebugLog("OID:  %s  " , vb.get_printable_value()) ;
    return 0;
*/
    // 存储接收和发送字节数

   


    Snmp::socket_cleanup();

	return 0;
}

