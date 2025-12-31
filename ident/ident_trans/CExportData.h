#ifndef _CEXPORTDATA_H_
#define _CEXPORTDATA_H_
#include "CIdentComm.h"
#include <map>
#include <string>
#include <vector>

class CExportData: public CIdentComm
{
public:
	CExportData(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CExportData()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("export_data");
	};
	virtual void CheckParameter(CStr2Map& inMap);

private:
	// 收集文件路径
	void CollectFilePaths(CStr2Map& quryInMap, CStr2Map& quryOutMap, 
		vector<string>& filePathList, map<string, string>& filePossnMap, bool isCollectData);
	// 压缩文件
	string CompressFiles(const vector<string>& filePathList, const char* szSavePath, CResData* pResData);
	// 生成Fpossn映射JSON文件
	string GeneratePossnJsonFile(const map<string, string>& filePossnMap, const char* szSavePath);
};

CTrans* CCgi::MakeTransObj()
{
	CExportData* pTrans = new CExportData(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
