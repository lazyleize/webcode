#include "CTest.h"
#include "CAuthRelayApi.h"

int CTest::AuthCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,outMap;
	//取得所有的请求参数
	pReqData->GetStrMap(inMap);

	//InfoLog("inst_name = [%s]", inMap["inst_name"].c_str());
	//InfoLog("manager_name = [%s]", inMap["manager_name"].c_str());
	
	//用户状态默认为1 有效
	//inMap["state"]="1";

	//调用relay接口
	//CAuthRelayApi::CreateUserInfo(inMap,true);
	InfoLog("inst_name = [%s]", inMap["inst_name"].c_str());
	inMap["id_no"] = "440795198901303755 ";
	outMap["id_no"] = Tools::rtrim(inMap["id_no"]);
	DebugLog("[%s:%d] id_no=%s",__FILE__,__LINE__,outMap["id_no"].c_str());
	
	return 0;
}
