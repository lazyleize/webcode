/************************************************************
 Desc:     用SVN去获取固件包
 Auth:     leize
 Modify:
 data:     2025-01-20
 ***********************************************************/
#include "adstcp.h"
#include "CRatingOnTerm.h"
#include "CIdentRelayApi.h"
#include "CTools.h"
#include "CSmtp.h"

#include <base/all.hpp>

using namespace aps;

extern CIdentAppComm* pIdentAppComm;
CRatingOnTerm::CRatingOnTerm()
{
}
CRatingOnTerm::~CRatingOnTerm()
{
}


int CRatingOnTerm::Commit(CStr2Map & inMap, CStr2Map &outMap)
{
    CStr2Map qryInMap;
    CStr2Map InsertInMap,InsertOutMap;
    CStr2Map upInMap,upOutMap;//更新固件规则表用
    CStr2Map preSVNInMap,preSVNOutMap;//更新预拉取固件表用

    //去机型表获取记录
    vector<CStr2Map> vectmapArray;
    qryInMap["offset"] = "0";
    qryInMap["limit"]  = "30"; //一次查询10笔

    int ret_num = 0;
    int count_cpu = 0;
    int deal_num = 0; //处理笔数
	int seccess_num = 0;
	int fail_num = 0;
	int conerror_num = 0;//连续失败次数

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
			//单笔处理
			deal_num++;
			InfoLog("第[%d]个机型 term_name=[%s]",deal_num,returnMap["term_name"].c_str());

            //计算时间
            InsertInMap["create_beg"] = Date::now().format("Y-M-D");
            InsertInMap["create_end"] = (Date::now().addDays(-10)).format("Y-M-D");
            InsertInMap["term_name"] = returnMap["term_name"];
            InsertInMap["year"] = to_string( Date::now().getYear());
            InsertInMap["month"] = Date::now().format("Y-M-D").substr(5,2);

            //在这个时间范围内去统计
            CIdentRelayApi::InsertPosName(InsertInMap,InsertOutMap,false);
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
        qryInMap["offset"] = outMap["ret_num"];
        qryInMap["limit"]  = "10"; //一次查询10笔

    }
    
    return 0;
}
