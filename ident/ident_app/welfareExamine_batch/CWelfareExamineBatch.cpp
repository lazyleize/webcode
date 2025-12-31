/************************************************************
 Desc:     根据t_jifen_batch_temp明细表发起卡激活交易
 Auth:     leize
 Modify:
 data:     2018-09-20
 ***********************************************************/
#include "adstcp.h"
#include "CWelfareExamineBatch.h"
#include "CIdentRelayApi.h"
#include "CTools.h"
#include "CSmtp.h"

extern CIdentAppComm* pIdentAppComm;

CWelfareExamineBatch::CWelfareExamineBatch()
{
}
CWelfareExamineBatch::~CWelfareExamineBatch()
{
}

int CWelfareExamineBatch::Commit(CStr2Map & inMap, CStr2Map &outMap)
{
    CStr2Map qryInMap,pvInMap,pvOutMap,upInMap,upOutMap;
    int ifor=0;
	int nRet = 0;

     InfoLog("1-PV-申请PV");
    pvInMap["sem_id"] = "1"; //批量发福利
    pvInMap["sem_op"] = "P";
    CIdentRelayApi::CommSemPV(pvInMap,pvOutMap,FALSE);
    if(pvOutMap["pv_result"]!="1")
    {
        ErrorLog("获取发起批量信号量失败 id=[1] state=[%s]",pvOutMap["sem_state"].c_str());
        return -1;
    }

    InfoLog("查询已审核通过的福利流水明细-并逐笔处理");
    vector<CStr2Map> vectmapArray;
    qryInMap["offset"] = "0";
    qryInMap["limit"]  = "10"; //一次查询10笔
	qryInMap["result"] = "7";  //审核通过

    int ret_num = 0;
    int count_cpu = 0;
    int deal_num = 0; //处理笔数
	int seccess_num = 0;
	int fail_num = 0;
	int conerror_num = 0;//连续失败次数
	//long long ltotalAmt = 0;
	
    while(1)
    {
        count_cpu++;

		//将容器清空
		vectmapArray.clear(); 

		//查询数据并保存在map容器

		//CIdentRelayApi::QueryJifenBatchNewList(qryInMap,outMap,vectmapArray,false);
		InfoLog("第[%d]个循环 vect-size=[%d]",count_cpu,vectmapArray.size());
		ret_num = atoi(outMap["ret_num"].c_str());
		if(ret_num < 1)
		{
			InfoLog("第[%d]个循环-查询笔数=[%d]-已处理完-退出",count_cpu,ret_num);
			break;
		}

		//遍历代付款流水
		for(size_t i = 0;i < vectmapArray.size();i++) 
		{
			CStr2Map returnMap;
			CIdentPub::DelMapF(vectmapArray[i],returnMap);
			//单笔处理
			deal_num++;
			InfoLog("第[%d]笔 batch_id=[%s]",deal_num,returnMap["batch_id"].c_str());
			nRet = CWelfareExamineBatch::dealOneBatchChargeInfo(returnMap);
			if(nRet == -1)
			{
				fail_num++;
				ErrorLog("第[%d]笔 batch_id=[%s] 处理异常-跳出FOR循环",deal_num,returnMap["batch_id"].c_str());
				conerror_num++;
				if(conerror_num==3)
				{
					ifor = 1;
					ErrorLog("第[%d]笔失败 处理异常-程序终止",deal_num);
					break;
				}

			}
			else
			{
				seccess_num++;
				conerror_num=0;
			}
			//调发送短信API通知用户
		}

		if(1 == ifor)
		{
			ErrorLog("第[%d]个循环-处理异常-退出",count_cpu);
			break;
		}

		if(outMap["ret_num"] != qryInMap["limit"])
		{
			InfoLog("第[%d]个循环-查询笔数=[%d]-limit=[%s]-已处理完-退出",count_cpu,ret_num,qryInMap["limit"].c_str());
			break;
		}

		//避免CPU过高，定期休眠
		if(count_cpu % 20 == 0)
		{
			InfoLog("第[%d]个循环-避免CPU过高-SLEEP-1秒",count_cpu);
			sleep(1);
		}

		//一次只处理200笔
		if(deal_num == 200)
		{
			InfoLog("第[%d]个循环-总笔数=[%d]-超过200退出",count_cpu,deal_num);
			break;
		}
  }

    InfoLog("批量处理代付总笔数=[%d]",deal_num);

    InfoLog("1-PV-释放PV");
    pvInMap["sem_id"] = "1";
    pvInMap["sem_op"] = "V";
    CIdentRelayApi::CommSemPV(pvInMap,pvOutMap,FALSE);
    if(pvOutMap["pv_result"]!="1")
    {
         ErrorLog("释放发起批量信号量失败 id=[1] state=[%s]", pvOutMap["sem_state"].c_str());
    }

    return 0;
}

bool CWelfareExamineBatch::dealOneBatchChargeInfo(CStr2Map& inMap)
{
	CStr2Map upInMap,upOutMap,tmpMap;

//随机生成兑换码，兑换密码
	string strRedeem_code("");
 string strExchange_cipher("");
 getRedeemcodeAndCipher(strRedeem_code, strExchange_cipher);

	upInMap["redeem_code"] = strRedeem_code;
	upInMap["exchange_cipher"] = strExchange_cipher;
	upInMap["batch_id"] = inMap["batch_id"];
 upInMap["trans_id"]  = inMap["trans_id"];
	upInMap["userid"]    = "8000050120";
	upInMap["totalamt"]   = inMap["amount"];

	if (inMap["mobile"].empty())//判断有无手机号
	{
		InfoLog("手机号为空");

		upInMap["result"]     = "11"; //未兑换
		upInMap["op_type"]  = "4"; 

		//CJIdentRelayApi::DealJifenChargeBatch(upInMap,upOutMap,true);

		upInMap["op_type"]  = "6";
  //CJifenRelayApi::DealJifenChargeBatch(upInMap,upOutMap,true);

	}else{

		//有手机号，校验用户是否注册
		if (false /*== CIdentRelayApi::CheckUserLogin(inMap,upOutMap,false)*/)
		{
			InfoLog("未注册用户[%s]",inMap["batch_id"].c_str());
			//帮用户自动完成注册
   CStr2Map userInMap;
   userInMap["pwd"] = "123456JF"; //登录初始密码
   userInMap["mobile"] = inMap["mobile"]; 
   userInMap["name"]  = "jifen_" + inMap["mobile"];
   userInMap["user_type"]  = "1"; //一般用户

			userInMap["op_type"]  = "9";
			//CJifenRelayApi::DealJifenChargeBatch(userInMap,upOutMap,true);

			upInMap["result"]     = "11"; //未兑换
			upInMap["op_type"]  = "4"; 

			//CJifenRelayApi::DealJifenChargeBatch(upInMap,upOutMap,true);

			upInMap["op_type"]  = "6";
			//CJifenRelayApi::DealJifenChargeBatch(upInMap,upOutMap,true);

			SendPhoneNotifyVericode(strRedeem_code, strExchange_cipher,  userInMap["pwd"],
				                                       upInMap["totalamt"], inMap["mobile"], 2);

 }else{

						upInMap["result"]     = "1"; //兑换成功
						upInMap["op_type"]  = "4"; 

						//CJifenRelayApi::DealJifenChargeBatch(upInMap,upOutMap,true);

						upInMap["op_type"]  = "6";
						//CJifenRelayApi::DealJifenChargeBatch(upInMap,upOutMap,true);
					
	     inMap["op_type"] = "2";//增加用户余额
						if(false /*== CJifenRelayApi::DealBatchWelfare(inMap,tmpMap,false*/)  //增加账户余额
						{
 						InfoLog("增加用户余额异常batch_id = [%s]",inMap["batch_id"].c_str());
						}

						SendPhoneNotifyVericode("", "", "",	upInMap["totalamt"], inMap["mobile"], 1);
				}
  }

	CStr2Map dealInMap;
	dealInMap["batch_id"] = inMap["batch_id"];
	dealInMap["result"] = upInMap["result"];
 dealInMap["amount"] = inMap["amount"];

 dealInMap["op_type"] = "10";
 //CJifenRelayApi::DealJifenChargeBatch(dealInMap,upOutMap,true);

	 return true;
}

bool CWelfareExamineBatch::getRedeemcodeAndCipher(string& strRedeem_code, string& strExchange_cipher)
{
	//随机生成兑换码，兑换密码
	const  static unsigned int UISEED[] = {523, 762, 863, 297, 475, 391, 659};
	static int iLocal = 0;
	unsigned int uiRedeem_code = 0;
	char szCode[64] = {"\0"};

	unsigned int uiExchange_cipher= 0;
	char szCipher[64] = {"\0"};

	struct timeval stuTimeVal;	
	memset(&stuTimeVal,0,sizeof(struct timeval)); 	
	gettimeofday(&stuTimeVal,NULL);	

	uiRedeem_code = stuTimeVal.tv_sec + stuTimeVal.tv_usec + UISEED[iLocal%7];
	sprintf(szCode, "%032d", uiRedeem_code);
	strRedeem_code = string(szCode).substr(22,10);

	uiExchange_cipher = stuTimeVal.tv_usec + (unsigned int )((stuTimeVal.tv_sec)/512) 
		+ (unsigned int )(UISEED[iLocal%7]/32);
	sprintf(szCipher, "%032d", uiExchange_cipher);
	strExchange_cipher = string(szCipher).substr(26,6);
	usleep(1);
	iLocal++;

 return true;
}

bool CWelfareExamineBatch::SendPhoneNotifyVericode(string strRedeem_code, string strExchange_cipher, string strInitPwd,
                                                                                    string strAmount, string strMobile, int iRegFlag)
{
 const static string STRMERCITYWEB = "s.jundax.com";
 const static string STRJFAPPWEB = TOOL_HTTP_DOMAIN"/h5/index.html";
 char szSMSContext[1024] = {"\0"};
	string strNowTime("");
	//string strSms = "验证码429918，您的操作需要进行验证，请勿向任何人提供您收到的短信验证码"
	strNowTime = CIdentPub::GetCurrentDateTime();

 string strInAmount = Tools::ConvertFenToYan(strAmount);

	CStr2Map chpInMap,chpOutMap;
	string strUrl = "http://www.ztsms.cn/sendSms.do";

	char utf_buf[2048]={"\0"};
	memset(utf_buf,0x00,sizeof(utf_buf));

if( 1 == iRegFlag ) //已经注册用户
{
     sprintf(szSMSContext, "尊敬的用户，您的账号 (本手机号) 内账户余额新增%s元，点击%s登录，"
                                       "进入首页>商城即可进行消费。【成交手】", strInAmount.c_str(), STRMERCITYWEB.c_str());
}else{

 string strNowDate = CIdentPub::GetDateNow();
 string strValidDate = CIdentPub::GetXyyDay(strNowDate.c_str(), 301);

	sprintf(szSMSContext, "尊敬的用户，您有1个价值%s元的兑换券：兑换券号：%s；兑换密码：%s。"
		                                  "请于%s年%s月%s日前手机端登录%s兑换：登录账号为本手机号码，初始密码为%s。【成交手】",
                                    strInAmount.c_str(), strRedeem_code.c_str(),  strExchange_cipher.c_str(),  
                                    strValidDate.substr(0,4).c_str(), strValidDate.substr(5,2).c_str(), strValidDate.substr(8,2).c_str(),
                                    STRJFAPPWEB.c_str(), strInitPwd.c_str());
}

 chpInMap["username"]     = "cxlm168";
 chpInMap["password"]     = "199cbfe9c0dfdc227401e0d49271288f";
 chpInMap["productid"]      = "676767";
 chpInMap["tkey"]             = strNowTime;
 chpInMap["xh"]                 = "";
 chpInMap["mobile"]           = strMobile;
 chpInMap["content"]         = string(szSMSContext);

	string postData;
	Tools::MapToStrNoEncode(chpInMap, postData);

	InfoLog("待发送短信POST url[%s?%s]", strUrl.c_str(),postData.c_str());

	ads_gb2312toutf8((char *)postData.c_str(),postData.length(),utf_buf);

	postData= utf_buf;
	InfoLog("待发送短信POST url UTF8[%s?%s]", strUrl.c_str(),postData.c_str());

	//真实发送的
	std::string strResponse;
	if(-1 == CIdentPub::HttpPost(strUrl,postData,strResponse))
	{
		ErrorLog("通讯失败 http post fail [%s]", strUrl.c_str());
		throw CTrsExp(ERR_JAVA_RETURN,"通讯失败");
	}

	int iLenSrc = strResponse.length();

	if(iLenSrc > 20480)
	{
		ErrorLog("返回异常 http response too long[%d]", iLenSrc);
		throw CTrsExp(ERR_JAVA_RETURN,"返回异常");
	}
	InfoLog("短信POST返回-UTF8[%d][%s]", iLenSrc,strResponse.c_str());

 return true;
}