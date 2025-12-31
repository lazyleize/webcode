#ifndef CQUERYSVNFILESLIST_H_
#define CQUERYSVNFILESLIST_H_

#include "cgicomm/CCgi.h"
#include "cgicomm/CTrans.h"
#include "cgicomm/CReqData.h"
#include "cgicomm/CResData.h"
#include "CIdentComm.h"

struct SVNItem {
    std::string name;   // 文件或目录名称
    std::string size;   // 文件大小
    std::string date;   // 日期和时间
    std::string type;   // "file" 或 "directory"
};

class CQuerySvnFilesList: public CIdentComm
{
public:
	CQuerySvnFilesList(CReqData *pReqData, CResData *pResData) :
		CIdentComm(pReqData, pResData)
	{
	}
	virtual ~CQuerySvnFilesList()
	{
	}
	virtual int IdentCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		string sTid("query_svn_files_list");
		return sTid;
	};

    string exec(const char* cmd);
    vector<SVNItem> parseSVNOutput(const std::string& output);
    int listSVNDirectory(const std::string& repoUrl,vector<CStr2Map>& vectmapArray);
	vector<SVNItem> parseSVNOutput1(const std::string& output);
};

CTrans* CCgi::MakeTransObj()
{
	CQuerySvnFilesList* pTrans = new CQuerySvnFilesList(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("ident");
	return pTrans;
}

#endif
