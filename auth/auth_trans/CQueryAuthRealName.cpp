#include "CQueryAuthRealName.h"
#include "CAuthRelayApi.h"

int CQueryAuthRealName::AuthCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap, outMap;
        /*---add---by---zhangwuyu---20151104---begin---*/
        CStr2Map sessMap;
        /*---add---by---zhangwuyu---20151104---end---*/
	//取得所有的请求参数
	pReqData->GetStrMap(inMap);
	
	CheckLogin(sessMap);
	//用户状态默认为1 有效
	//inMap["state"]="1";
	//调用relay接口
        inMap["uid"] = sessMap["uid"];
	outMap["real_name"] = CAuthRelayApi::QueryAuthRealNameByUid(inMap["uid"],true);
	if(outMap["real_name"].empty())
	{
			ErrorLog("[%s %d]查询用户实名信息失败: name=[%s]", __FILE__, __LINE__,
							inMap["name"].c_str());
			throw CTrsExp(ERR_USER_REAL_NAME,"查询用户实名信息失败！");
	}
	pResData->SetPara("real_name",outMap["real_name"]);

	return 0;
}
