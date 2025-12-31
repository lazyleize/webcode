/************************************************************
 Desc:     用SVN去获取固件包
 Auth:     leize
 Modify:
 data:     2025-01-20
 ***********************************************************/
#include "adstcp.h"
#include "CYearOnTermShip.h"
#include "CIdentRelayApi.h"
#include "CTools.h"
#include "CSmtp.h"

#include <base/all.hpp>

using namespace aps;

extern CIdentAppComm* pIdentAppComm;
CYearOnTermShip::CYearOnTermShip()
{
}
CYearOnTermShip::~CYearOnTermShip()
{
}


int CYearOnTermShip::Commit(CStr2Map & inMap, CStr2Map &outMap)
{
    CStr2Map qryInMap;
    CStr2Map InsertInMap,InsertOutMap;
    CStr2Map upInMap,upOutMap;//更新固件规则表用
    CStr2Map preSVNInMap,preSVNOutMap;//更新预拉取固件表用

    //去机型表获取记录
    vector<CStr2Map> vectmapArray;
    qryInMap["offset"] = "0";
    qryInMap["limit"]  = "10"; //一次查询10笔

    int ret_num = 0;
    int count_cpu = 0;
    int deal_num = 0; //处理笔数
	int seccess_num = 0;
	int fail_num = 0;
	int conerror_num = 0;//连续失败次数
	int current_offset = 0; //当前偏移量

    while(1)
    {
        count_cpu++;

		//将容器清空
		vectmapArray.clear(); 
        CIdentRelayApi::QueryTermList(qryInMap,outMap,vectmapArray,false);
        InfoLog("机型个数 vect-size=[%d]",count_cpu,vectmapArray.size());
        ret_num = atoi(outMap["ret_num"].c_str());
        if(ret_num < 1)
		{
			InfoLog("机型个数是0,不需要处理-退出");
			break;
		}
        

        //开始处理每个机型的统计
        for(size_t i = 0;i < vectmapArray.size();i++) 
        {
            CStr2Map returnMap;
			CIdentPub::DelMapF(vectmapArray[i],returnMap);
            Date m_date;
            m_date.setDate(inMap["days"]);
			//单笔处理
			deal_num++;
			InfoLog("第[%d]个机型 term_name=[%s]",deal_num,returnMap["term_name"].c_str());

            //计算时间
            InsertInMap["create_beg"] = (m_date.addDays(-1)).format("Y-M-D");
            InsertInMap["create_end"] = inMap["days"];
            InsertInMap["term_name"] = returnMap["term_name"];
            InsertInMap["year"] = to_string(m_date.getYear());
            InsertInMap["month"] = m_date.format("Y-M-D").substr(5,2);

            //在这个时间范围内去统计
            if(!CIdentRelayApi::InsertPosName(InsertInMap,InsertOutMap,false))
            {
                ErrorLog("机型[%s] ,统计失败",InsertInMap["term_name"].c_str());
            }
        }
        //判断是否处理完成
        if(atoi(outMap["total"].c_str()) == deal_num)
		{
			InfoLog("已处理完,总共[%d]个机型-退出",deal_num);
			break;
		}
        //累加当前查询到的记录数到偏移量
        current_offset += ret_num;
        qryInMap["offset"] = to_string(current_offset);
        qryInMap["limit"]  = "10"; //一次查询10笔
        
    }
    
    return 0;
}
