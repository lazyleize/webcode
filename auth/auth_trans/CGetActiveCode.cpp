#include "CGetActiveCode.h"
#include "CAuthRelayApi.h"

int CGetActiveCode::AuthCommit(CReqData* pReqData,CResData* pResData)
{
		CStr2Map inMap;
		
		//取得所有的请求参数
        pReqData->GetStrMap(inMap);

		//检查用户是否是合法用户
        inMap["uid"] = CAuthRelayApi::QueryUidByUin(inMap["name"]);
        if(inMap["uid"].empty())
        {
			ErrorLog("[%s %d]查询用户信息失败: uin=[%s]",
                                __FILE__, __LINE__,
                                inMap["name"].c_str());
        	throw CTrsExp(ERR_USER_NOT_REGISTER,"不存在的用户名！");
        }
		
		//生成激活码
		//int code = this->GetVerifyCode();
		inMap["active_code"] = this->GetVerifyCode();
		
		InfoLog("激活码code=[%s]",inMap["active_code"].c_str());
	
		//将激活码保存的数据库中
			
		/*if(CHECK_TYPE_MOBILE == pReqData->GetPara("c_type"))
		{	
			if(pReqData->GetPara("mobile").empty())
			{
				//查询用户手机号
			}
			//发送手机短信
		}
		else if(CHECK_TYPE_EMAIL == pReqData->GetPara("c_type"))
		{
			if(pReqData->GetPara("email").empty())
        	{
            	//查询用户手机号
        	}
			//发送电子邮件
		}
		*/
		pResData->SetPara("active_code", inMap["active_code"]);
	
		return 0;
}
