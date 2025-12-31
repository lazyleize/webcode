#ifndef _CACCONTROLLERROUTERSLIST_H_
#define _CACCONTROLLERROUTERSLIST_H_
#include "CIdentComm.h"
#include <vector>
#include <string>

using namespace std;

// 前向声明
class CSnmpClient;

// 路由器详细信息结构（用于列表接口）
struct RouterDetailInfo
{
    string routerId;      // 设备ID
    string name;          // 路由器名称
    string ip;            // IP地址
    string status;        // 状态：online/offline/fault
    int connections;      // 当前连接数
    int uploadSpeedCurrent;  // 当前上行速率（KB/s）
    uint64_t uploadToday;    // 今日上行总量（KB）
    int downloadSpeedCurrent; // 当前下行速率（KB/s）
    uint64_t downloadToday;   // 今日下行总量（KB）
    int cpu;              // CPU使用率（%）
    int memory;           // 内存使用率（%）
    int temperature;      // 温度（℃），可选
    uint32_t uptime;      // 运行时长（秒）
    string lastUpdate;    // 最后更新时间
    
    RouterDetailInfo() : connections(0), uploadSpeedCurrent(0), uploadToday(0),
                        downloadSpeedCurrent(0), downloadToday(0), cpu(0), memory(0),
                        temperature(0), uptime(0) {}
};

class CAcControllerRoutersList: public CIdentComm
{
public:
    CAcControllerRoutersList(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CAcControllerRoutersList()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("ac_controller_routers_list");
	};

private:
    // MAC地址转换函数：将十六进制MAC地址转换为OID索引格式
    static string ConvertMacToOidIndex(const string& hexMac);
    
    // 从AC控制器获取路由器列表
    vector<RouterDetailInfo> GetRouterListFromAC(CSnmpClient& acClient);
    
    // 从AC控制器获取指定路由器的详细信息
    int GetRouterDetailInfoFromAC(CSnmpClient& acClient, RouterDetailInfo& router);
};

CTrans* CCgi::MakeTransObj()
{
	CAcControllerRoutersList* pTrans = new CAcControllerRoutersList(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
