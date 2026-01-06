#include "CIdentRelayApi.h"
#include "tools/CTools.h"
#include "tools/RuntimeGather.h"
#include "relaycomm/ParseXmlData.h"

bool CIdentRelayApi::QueryYearSummary(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp)
{
    CStr2Map inMap;

    inMap["transcode"] = TRANSCODE_COMM_QUERY_NOTOTAL;
    inMap["reqid"] = "1863";
    inMap["offset"] = paramap["offset"].empty() ? "0" : paramap["offset"];
    inMap["limit"] = paramap["limit"].empty() ? "0" : paramap["limit"];

    string str = "nono:123456";
	if(!paramap["year"].empty())
	{
		str += "|year:" + paramap["year"];
	}
	inMap["fields"] = str;

    CRelayCall* m_pRelayCall = CIdentRelayApi::GetRelayObj(CIdentRelayApi::IDENT_COMM_QUERY);
            
    bool ret = m_pRelayCall->Call(inMap);

    InfoLog("send : %s", m_pRelayCall->getSendStr());
    InfoLog("recv : %s", m_pRelayCall->getResultStr());

    if (!CIdentRelayApi::ParseResult(ret, m_pRelayCall, resultmap, throwexp))
    {
        return false;
    }

    const Para_Struct paras[] = {
                                {"Fyear"},
							    {"Fonepass"},
								{"Frework"},
								{"Favgefficiency"},
                                {"FlastOvertime"},
                                {"Finspection"},
                                {"Ferror_comm"},
                                {"Ferror_work"},
                                {"Ferror_api"},
                                {"Ferror_data"},
                                {"Ferror_custom"},
                                {"Ferror_down"},
                                {"Ferror_offline"},
                                {"Ferror_normal"},
                                {"FfirstQuarter"},
                                {"FfirstpassRate"},
                                {"Ffirstefficiency"},
                                {"FsecondQuarter"},
                                {"FsecondpassRate"},
                                {"Fsecondefficiency"},
                                {"FthirdQuarter"},
                                {"FthirdpassRate"},
                                {"Fthirdefficiency"},
                                {"FfourthQuarter"},
                                {"FfourthpassRate"},
                                {"Ffourthefficiency"},
        {NULL}
    };
    resultmap["ret_num"]="0";
    vector<CStr2Map> vectmapArray;
    //解析xml格式获取数据
    GetRecFromXml(vectmapArray, resultmap, m_pRelayCall->getResultStr(),&paras[0]);
    if(vectmapArray.size() > 0)
    {
        resultmap.insert(vectmapArray[0].begin(),vectmapArray[0].end());
    }
    return true;
}

