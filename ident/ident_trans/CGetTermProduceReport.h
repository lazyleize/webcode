#ifndef CGETTERMPRODUCEREPORT
#define CGETTERMPRODUCEREPORT
#include "CIdentComm.h"
#include "CProduceRecordData.h"

#include <sstream>
#include <vector>
#include <algorithm>

// 取消宏定义，避免冲突
#undef max
#undef min
#include "rapidjson/document.h"

using namespace std;
class CGetTermProduceReport: public CIdentComm
{
public:
    CGetTermProduceReport(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CGetTermProduceReport()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("get_term_produce_report");
	};

	virtual void CheckParameter(CStr2Map& inMap);
	
private:
	/**
	 * @brief 查询同类机型（根据Fsmt_node、Fac_node、Fpack_node相同）
	 * @param targetPosName 目标机型名称
	 * @param similarPosNames 返回的同类机型列表
	 * @return 是否成功
	 */
	bool QuerySimilarPosNames(const string& targetPosName, vector<string>& similarPosNames);
	
	/**
	 * @brief 构建ES查询JSON（根据时间+机型查询）
	 * @param posNames 机型名称列表
	 * @param timeBeg 开始时间
	 * @param timeEnd 结束时间
	 * @return ES查询JSON字符串
	 */
	string CreateESQueryJson(const vector<string>& posNames, const string& timeBeg, const string& timeEnd);
	
	/**
	 * @brief 发送ES查询请求
	 * @param indexName ES索引名称
	 * @param queryJson 查询JSON
	 * @param resultJson 返回的JSON结果
	 * @return 是否成功
	 */
	bool QueryElasticsearch(const string& indexName, const string& queryJson, string& resultJson);
	
	/**
	 * @brief 解析ES聚合结果
	 * @param esResultJson ES返回的JSON字符串
	 * @param esData 解析后的ES数据（输出）
	 * @return 是否成功
	 */
	bool ParseESAggregationResult(const string& esResultJson, rapidjson::Document& esData);
	
	/**
	 * @brief 生成最终报告JSON（整合生产记录数据 + ES数据）
	 * @param produceData 生产记录数据
	 * @param esData ES聚合数据
	 * @param targetPosName 目标机型名称
	 * @param similarPosNames 同类机型列表
	 * @param timeBeg 开始时间
	 * @param timeEnd 结束时间
	 * @return 报告JSON字符串
	 */
    	string GenerateFinalReportJson(CProduceRecordData& produceData,
    	                                const rapidjson::Document& esData,
    	                                const string& targetPosName,
    	                                const vector<string>& similarPosNames,
    	                                const string& timeBeg,
    	                                const string& timeEnd,
    	                                int totalCount = 0,
    	                                int successCount = 0,
    	                                int failCount = 0,
    	                                double successRate = 0.0);

};

CTrans* CCgi::MakeTransObj()
{
	CGetTermProduceReport* pTrans = new CGetTermProduceReport(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
