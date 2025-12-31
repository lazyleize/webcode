#include "CQueryUserExist.h"
#include "CAuthRelayApi.h"

int CQueryUserExist::AuthCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap;
	//取得所有的请求参数
	pReqData->GetStrMap(inMap);
	string uid = CAuthRelayApi::QueryUidByUin(pReqData->GetPara("uin"));
	if(uid.empty())
	{
		ErrorLog("[%s],[%d]行 该用户不存在 uin:[%s]",__FILE__,__LINE__,inMap["uin"].c_str());
		throw CTrsExp(ERR_UNKNOWN_USER,"该用户不存在");
	}

	return 0;
}
