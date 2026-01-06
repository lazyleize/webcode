#include "CUserGetTotpSecret.h"
#include "CAuthRelayApi.h"

int CUserGetTotpSecret::AuthCommit(CReqData* pReqData,CResData* pResData)
{
        
    CStr2Map sessMap;
	CStr2Map inMap,outMap;

	//取得所有的请求参数
	string strPostdata=pReqData->GetPostData();
	CAuthPub::parsePubRespJson(strPostdata,inMap);

	CAuthRelayApi::getTotpSecret(inMap,outMap,true);

	pResData->SetPara("secret", outMap["secret"]);
	pResData->SetPara("qr_code", outMap["otpauth_uri"]);
	
	return 0;
}
