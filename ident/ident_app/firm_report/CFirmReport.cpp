/************************************************************
 Desc:     每月1号凌晨1点生成《工厂生产流程报表(月)》
 Auth:     leize
 Modify:
 data:     2025-07-8
 ***********************************************************/
#include "adstcp.h"
#include "CFirmReport.h"
#include "CIdentRelayApi.h"
#include "CTools.h"
#include "CSmtp.h"
#include "jwsmtp/jwsmtp.h"

#include <base/all.hpp>

using namespace aps;

extern CIdentAppComm* pIdentAppComm;

CFirmReport::CFirmReport()
{
}
CFirmReport::~CFirmReport()
{
}

int CFirmReport::Commit(CStr2Map &inMap, CStr2Map &outMap)
{
    // 获取本地存储路径和 Python 脚本路径
    CStr2Map qryOutMap, InsertOutMap,updateInMap;
    char basePath[256] = {0};

    //从配置文件获取存放路径，创建规则，pack_id/,去统计记录，生成报告，写入到文件，更新数据库，结束
    // 从全局变量中获取路径
    memcpy(basePath, g_mTransactions[GetTid()].m_mVars["localbasepath"].c_str(), g_mTransactions[GetTid()].m_mVars["localbasepath"].length());

    if(!CIdentRelayApi::InsertFirmReportRecord(inMap,InsertOutMap,FALSE))
    {
        ErrorLog("插入固件报告记录表失败 [%s]",inMap["pack_id"].c_str());
        return 0;
    }

    // 去统计数据
    CIdentRelayApi::FirmReportCreate(inMap,qryOutMap,false);

    //去发邮件并写文件html
    string strEmailDate = CreateRport(qryOutMap,basePath);
    try
    {
        CSmtp mail;
        mail.m_bHTML = true;
        mail.SetSMTPServer("smtphz.qiye.163.com",465);      //设置发送邮件的服务器，smtp为发送，端口号是465或587，pop3为接收,
        mail.SetSecurityType(USE_SSL);                      //遵守SSL(安全套接层)协议的网络传输
        mail.SetLogin("leize@xgd.com");                     //登录名
        mail.SetPassword("Launch*05121");                   //登录密码
        mail.SetSenderName("雷泽");                         //寄件人姓名
        mail.SetSenderMail("leize@xgd.com");                //寄件人邮箱
        mail.SetReplyTo("leize@xgd.com");                   //回复的邮箱地址
        mail.SetSubject("自动化固件下载报告");               //邮件标题
        mail.AddRecipient("tangshun@xgd.com","唐顺");                 //邮件接受者
        mail.AddRecipient("cuilonglong@xgd.com","崔龙龙");                 //邮件接受者
        mail.AddRecipient("geyu@xgd.com","葛宇");                 //邮件接受者
        mail.AddRecipient("leize@xgd.com","雷泽");                 //邮件接受者
        //mail.AddCCRecipient("zhangchengli@xgd.com","利总");     //抄送人
        mail.SetCharSet("UTF-8");                           //设置字符集为 UTF-8
        mail.AddMsgLine(strEmailDate.c_str());
        mail.Send();//发送邮件
    }
    catch(ECSmtp e)
    {
        ErrorLog("[%s %d]邮件发送失败: [%s]", __FILE__, __LINE__,e.GetErrorText().c_str());
        throw CTrsExp(ERR_USER_NOT_REGISTER,"邮件发送失败");
    }
    
    
    InfoLog("Email sent successfully!");

    string strOutFile = basePath;
    strOutFile += "/";
    strOutFile += inMap["pack_id"];
    Directory::mkdir(strOutFile);
    strOutFile += "/FrimReport.html"; 
    File m_file(strOutFile,"w");
    if(!m_file.isOpened())
    {
        ErrorLog("File open error.");
        updateInMap["state"] = "FAIL";
    }
    else
        updateInMap["state"] = "DONE";

    m_file.write(strEmailDate.c_str(),strEmailDate.length());
    m_file.close();

    updateInMap["pack_id"] = inMap["pack_id"];
    
    if(!CIdentRelayApi::UpdateFirmReportRecord(updateInMap,qryOutMap,FALSE))
    {
        ErrorLog("更新固件报告记录表失败 [%s]",inMap["pack_id"].c_str());
        return 0;
    }
    
    return 0;
}

string CFirmReport::CreateRport(CStr2Map & inMap,char* pbasepath)
{
    //处理时间
    string endTime,down_rate,fail_rate,total_size;

    InfoLog(" start_time [%s][%s]", inMap["start_time"].c_str(),inMap["end_time"].c_str());


    string date_part = inMap["start_time"].substr(0,10); // "2025-01-22"
    string time_part = inMap["start_time"].substr(11); 
    int start_year = std::stoi(date_part.substr(0, 4));
    int start_month = std::stoi(date_part.substr(5, 2));
    int start_day = std::stoi(date_part.substr(8, 2));
    int start_hour = std::stoi(time_part.substr(0, 2));
    int start_minute = std::stoi(time_part.substr(3, 2));
    int start_second = std::stoi(time_part.substr(6, 2));
    string startTime = to_string(start_year) +"年" + to_string(start_month) + "月"+to_string(start_day)+"日"+to_string(start_hour)+" 时"+to_string(start_minute)+
    "分"+to_string(start_second)+"秒";

    string end_date_part = inMap["end_time"].substr(0,10); // "2025-01-22"
    string end_time_part = inMap["end_time"].substr(11); 
    int end_year = std::stoi(end_date_part.substr(0, 4));
    int end_month = std::stoi(end_date_part.substr(5, 2));
    int end_day = std::stoi(end_date_part.substr(8, 2));
    int end_hour = std::stoi(end_time_part.substr(0, 2));
    int end_minute = std::stoi(end_time_part.substr(3, 2));
    int end_second = std::stoi(end_time_part.substr(6, 2));
    endTime = to_string(end_year)+"年" + to_string(end_month) + "月"+to_string(end_day)+"日"+to_string(end_hour)+" 时"+to_string(end_minute)+
    "分"+to_string(end_second)+"秒";



    InfoLog("时间 [%s][%s]",startTime.c_str(),endTime.c_str());

    int down_success = stoi(inMap["down_success"].c_str());
    double pack_size = std::stod(inMap["pack_size"].c_str()); // 单位为 M
    double total_size_gb = (pack_size * down_success) / 1024.0; // 1G = 1024M
    std::ostringstream oss; // 使用ostringstream来格式化字符串
    oss << std::fixed << std::setprecision(4) << total_size_gb; // 设置为固定小数点格式并保留4位小数
    total_size = oss.str() + " G";

    int plan_down = std::stoi(inMap["plan_down"].c_str());

    if (plan_down > 0) 
    {
        double rate = (static_cast<double>(down_success) / plan_down) * 100;
        down_rate = std::to_string(static_cast<int>(rate)) + "%"; // 转换为整数并添加百分号
    } else 
    {
        down_rate = "0%"; // 处理计划下载数量为零的情况
    }

    string strCurrent ;
    strCurrent = to_string(Datetime::now().getYear()) + " 年 " + to_string(Datetime::now().getMonth()) + " 月 " + to_string(Datetime::now().getDay()) + " 日 ";


    std::string htmlContent = "<html>"
                           "<head>"
                           "<style>"
                           "body { font-family: Arial, sans-serif; margin: 20px; font-size: 16px; }" // 设置默认字体大小
                           ".container { border: 2px solid #000; padding: 20px; border-radius: 5px; font-size: 16px; }" // 设置容器字体大小
                           ".center { text-align: center; }" // 居中样式
                           ".right { text-align: right; }" // 右对齐样式
                           "</style>"
                           "</head>"
                           "<body>"
                           "<div class='container'>"
                           "<h2 class='center' style='font-size:24px;'><strong>自动化固件下载报告</strong></h2>"
                           "<p>订单号 <strong>" + inMap["order_id"] + "</strong>，于 " + startTime + " 到 " + endTime + " ,"
                           "固件下载情况: </p>"
                           "<ul>"
                           "<li>计划下载 <strong>" + inMap["plan_down"] + "</strong> 台</li>"
                           "<li>实际下载次数 <strong>" + inMap["actual_down"] + "</strong> 次</li>"
                           "<li>最终下载成功 <strong>" + inMap["down_success"] + "</strong> 台</li>"
                           "<li>下载完成率 <strong>" + down_rate + "</strong></li>"
                           "<li>总计下载量 <strong>" + total_size + "</strong></li>"
                           "</ul>"
                           "<p>详细数据请登陆: <a href='https://ppm.xgd.com'>https://ppm.xgd.com</a> 查看</p>"
                           "<p class='right'>" + strCurrent + "</p>" // 右对齐
                           "<p class='right'>PPM 系统</p>" // 右对齐
                           "</div>"
                           "</body>"
                           "</html>";
    return htmlContent;

}