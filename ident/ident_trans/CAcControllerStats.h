#ifndef _CACCONTROLLERSTATS_H_
#define _CACCONTROLLERSTATS_H_
#include "CIdentComm.h"
#include <vector>
#include <string>

using namespace std;

// 前向声明
class CSnmpClient;

// 路由器信息结构（从AC控制器获取）
struct RouterInfo
{
    string routerId;      // 路由器ID/索引
    string name;          // 路由器名称
    string ip;            // 路由器IP地址
    string status;        // 状态：online/offline/fault
    int cpu;              // CPU使用率
    int memory;           // 内存使用率
    uint64_t upload;      // 上行流量（KB）
    uint64_t download;    // 下行流量（KB）
    
    RouterInfo() : cpu(0), memory(0), upload(0), download(0) {}
};

class CAcControllerStats: public CIdentComm
{
public:
    CAcControllerStats(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CAcControllerStats()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("ac_controller_stats");
	};

private:
	// 从AC控制器获取路由器列表
	vector<RouterInfo> GetRouterListFromAC(CSnmpClient& acClient);
	
	// 从AC控制器获取指定路由器的详细信息
	int GetRouterInfoFromAC(CSnmpClient& acClient, RouterInfo& router);
};

CTrans* CCgi::MakeTransObj()
{
	CAcControllerStats* pTrans = new CAcControllerStats(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
