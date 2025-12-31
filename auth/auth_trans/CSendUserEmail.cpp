#include "CSendUserEmail.h"
#include "CAuthRelayApi.h"
#include "CSmtp.h"

int CSendUserEmail::AuthCommit(CReqData* pReqData,CResData* pResData)
{
	CStr2Map inMap, outMap;
	//取得所有的请求参数
        pReqData->GetStrMap(inMap);

	//检查用户是否是合法用户
	/*
        inMap["uid"] = CAuthRelayApi::QueryUidByName(inMap["name"]);
        if(inMap["uid"].empty())
        {
			ErrorLog("[%s %d]查询用户信息失败: name=[%s]",
                                __FILE__, __LINE__,
                                inMap["name"].c_str());
        	throw CTrsExp(ERR_USER_NOT_REGISTER,"此账户不存在！");
        }
	*/
	
	this->SendEmail();	
	
	return 0;
}
void CSendUserEmail::SendEmail()
{
	try
	{
		CSmtp mail;
		mail.SetSMTPServer("smtp.exmail.qq.com",465);
		mail.SetSecurityType(USE_SSL);
		mail.SetLogin("service@transt.cn");
		mail.SetPassword("cdh123456");
		mail.SetCharSet("gb2312");
  		mail.SetSenderName("财大户");//寄件人姓名
  		mail.SetSenderMail("service@transt.cn");//寄件人邮箱
  		mail.SetReplyTo("service@transt.cn");//回复
  		mail.SetSubject("财大户推广计划");//标题
  		mail.AddRecipient("hlh@transt.cn");//接受者
  		mail.SetXPriority(XPRIORITY_NORMAL);
  		//mail.SetXMailer("The Bat! (v3.02) Professional");//信件是从哪个客户端发送出来
  		mail.AddMsgLine("收到文件请回复，谢谢！");//添加正文
		//mail.AddMsgLine("");
		//mail.AddMsgLine("...");
		//mail.AddMsgLine("");
		//mail.AddMsgLine("");
		//mail.ModMsgLine(5,"regards");
		//mail.DelMsgLine(2);
		mail.AddAttachment("/home/httpd/web/web_pay5/emailfile/text.txt");
		//mail.AddAttachment("/home/httpd/web/web_pay5/emailfile/logo.jpg");
		//mail.AddAttachment("/home/httpd/web/web_pay5/emailfile/推广需求.zip");
		mail.Send();//发送邮件
	}
	catch(ECSmtp e)
	{
		ErrorLog("[%s %d]邮件发送失败: [%s]",
                                __FILE__, __LINE__,e.GetErrorText().c_str());
        	throw CTrsExp(ERR_USER_NOT_REGISTER,"邮件发送失败");
	}

	DebugLog("[%s %d]邮件发送成功!",__FILE__, __LINE__);
}
