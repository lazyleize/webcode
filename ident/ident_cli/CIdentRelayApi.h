#ifndef CIDENTRELAYAPI_H_
#define CIDENTRELAYAPI_H_
#include "relaycomm/CRelayCall.h"
#include "tools/commdef.h"
#include "tools/CTrsExp.h"

//状态  0 正常  1 停用  2 待认证  3 认证中  4 解除协议
enum EContractState
{
    CONTRACT_STATE_NORMAL   =   0,
    CONTRACT_STATE_STOP     =   1,
    CONTRACT_STATE_WAIT     =   2,
    CONTRACT_STATE_CHECKING =   3,
    CONTRACT_STATE_DELETE   =   4,

};

#define CONN_JAVA_MD5_KEY          "flajglalflkjaogouf0u709417d"      //跟JAVA通讯使用的MD5秘钥
#define CONN_CGI_3DES_KEY          "E45988F49D61FCC6BEBDFC3F5237CE72" //本地CGI通讯的3DES秘钥
#define CONN_CGI_MD5_KEY           "lfajglajg187lajglajg"             //本地CGI通讯的MD5秘钥

#define TRANSCODE_COMM_QUERY                       "100001" //公共查询交易码
#define TRANSCODE_COMM_QUERY_NOTOTAL               "100002" //公共查询交易码 ---- 批量不返回total

namespace CIdentRelayApi
{
    /*
     * 声明SERVICE的RequestType。
     */
    const static int IDENT_COMM_QUERY                  = 1000;
    const static int IDENT_COMM_QUERY_NOTOTAL          = 2000;
    const static int IDENT_COMM_USER                   = 8801;//C端程序用
    const static int IDENT_COMM_TRANSMIT               = 4000;//转发服务
    const static int IDENT_COMM_MANAGER                = 8803;//管理员
	const static int IDENT_COMM_BATCH                  = 8804;//批量处理

    //	系统交易码
    //  100001
    #define TRANSCODE_ID_TYPE_DEAL               "100001"

	#define TRANSCODE_TR_UPDATTESTSTATE          "100028"  //测试更新

	#define TRANSCODE_ID_SEM_PV                  "100219"  //批量账务


	#define TRANSCODE_TR_TRAN_DEAL               "100200"  //转发处理
	#define TRANSCODE_TR_RESULT_UP               "100201"  //MES结果上送
	#define TRANSCODE_TR_SAVEDLLFile             "100202"  //保存dll文件
	#define TRANSCODE_TR_DOWNDLLFile             "100203"  //下载dll文件
	#define TRANSCODE_TR_SAVELOGFile             "100204"  //保存log文件
	#define TRANSCODE_TR_CREATEFACTORY           "100205"  //创建工厂
	#define TRANSCODE_TR_UPDATEFACTORY           "100206"  //更新工厂状态
	#define TRANSCODE_TR_MODIFYFACTORY           "100207"  //修改工厂名称
	#define TRANSCODE_TR_UPDATEDLLSTATE          "100208"  //更新dll状态

	#define TRANSCODE_TR_NORMUPLOADLOG           "100210"  //通用日志上传
	#define TRANSCODE_TR_DATAACQUISITION         "100211"  //数据采集入库
	#define TRANSCODE_TR_ASSPACK                 "100212"  //获取相关联的SN
	#define TRANSCODE_TR_TERMDATAUPLOAD          "100213"  //终端日志上传
	#define TRANSCODE_TR_FIRMDOWNLOADSET         "100214"  //固件下载规则设置
	#define TRANSCODE_TR_FIRMDOWNLOADUPDATE      "100215"  //更新固件下载本地路径
	#define TRANSCODE_TR_APPGETFIRMDONWLOADPATH  "100216"  //APP获取固件下载地址
	#define TRANSCODE_TR_APPUPDATEDONWLOADRCORD  "100217"  //APP校验固件下载Hash
	#define TRANSCODE_TR_APPUPDATENOTIFY         "100218"  //终端更新完成通知
	#define TRANSCODE_TR_FIRMSTATEUPDATE         "100220"  //固件下载规则更新
	#define TRANSCODE_TR_FIRMINFOUPDATE          "100221"  //固件下载信息表更新
	#define TRANSCODE_TR_ATELOGUPLOAD            "100222"  //射频日志上传
	#define TRANSCODE_TR_GETDOWNLOADTYPE         "100223"  //判断终端下载方式
	#define TRANSCODE_TR_CLOSETERMGO             "100224"  //关闭终端热点GO
	#define TRANSCODE_TR_INSERTFIRMINFO          "100225"  //记录单个固件信息
	#define TRANSCODE_TR_UPDATEPREFIRM           "100226"  //更新固件预先拉取表
	#define TRANSCODE_TR_DELETEFIRMINFO          "100227"  //删除固件信息表记录
	#define TRANSCODE_TR_PRODUCTMONTHREPORT      "100228"  //按月统计工厂生产流程情况
	#define TRANSCODE_TR_INSERTREPORTRECORD      "100229"  //插入报表生成记录到数据库
	#define TRANSCODE_TR_INSERTRFIRMREPORT       "100230"  //插入固件报表记录表
	#define TRANSCODE_TR_UPDATERFIRMREPORT       "100231"  //更新固件报表记录表
	#define TRANSCODE_TR_CREATEFIRMREPORT        "100232"  //生成固件下载报告
	#define TRANSCODE_TR_GETASSPIDMIN            "100233"  //获取组装段最小PID号
	#define TRANSCODE_TR_UPDATEFISTJOIN          "100234"  //更新用户首次进入状态
	#define TRANSCODE_TR_CHECKPOSNAME            "100235"  //判断机型是否存在
	#define TRANSCODE_TR_INSERTPOSNAME           "100236"  //入库机型出货表
	#define TRANSCODE_TR_ALARMSET                "100237"  //告警消息设置
	#define TRANSCODE_TR_BINDMESSAGEUSER         "100238"  //消息设置绑定人员
	#define TRANSCODE_TR_GETSERIALNUM            "100239"  //获取流水号
	#define TRANSCODE_TR_UPDATETERMRECORD        "100240"  //插入/更新工厂生产概况表
	#define TRANSCODE_TR_UPDATEMEAASGELIST       "100241"  //更新消息通知表
	#define TRANSCODE_TR_GETTERMPRODUCINFO       "100242"  //获取机型生产信息
	#define TRANSCODE_TR_TERMGOLOGUPLOAD         "100243"  //终端GO日志上传
	#define TRANSCODE_TR_CREATEORDERCHANGE       "100244"  //创建生产变化记录
	#define TRANSCODE_APP_ACSAVRAPINFO           "100245"  //保存AP历史信息
	#define TRANSCODE_APP_ACINFOSACE             "100246"  //保存AC信息--统计用
	#define TRANSCODE_APP_ACPEAKTRAFFIC          "100247"  //保存AC峰值流量数据
	#define TRANSCODE_APP_ACAPTODAYTRAFFIC       "100248"  //保存AP今日流量数据
	#define TRANSCODE_APP_ACTRAFFICTREND         "100249"  //保存AC流量趋势数据
    
   
    /*
     *分析调用返回结果
     *@param bCallRes 调ey返回结果

     *@param result 返回结果集
     *@param throwexp 是否抛出异常
     *@param xmlPrase 是否要解析返回的xml
     *说明 通用查询返回xml
     */
    bool ParseResult(bool bCallRes, CRelayCall* m_pRelayCall, CStr2Map& resultMap,
            bool throwexp = false, bool xmlPrase = false);

    /**
     *创建发包,收包对像解析数据的对像,返回对像指针
     *@param objtype 对像类型,在些表示
     1 表示发送明文对像,2  用于创建发送密文对像
     *@param request_type 接口的编号
     *说明: 此对象指针无需释放，所有relay访问共用一个对像
     */
    CRelayCall* GetRelayObj(int request_type, int objtype = 1);

    CRelayCall* GetRelayObj(const string & reqtype, int objtype = 1);

	/* 测试用*/
	bool UpdateTestState(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

    /*
     * pParams, 是一组字符串指针,字符串以NULL结尾
     * paramMap : 参数容器
     * 如果在paramMap不存在指定的参数,直接抛异常,表示
     * 接口调用基本条件不满足
     */
    void CheckParams(const char** pParams, CStr2Map& paramMap,const int& iRequestType, bool bStrongCheck = false);

	/* 外网转发*/
	bool IdentTrans(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 记录日志信息*/
	bool RecordLog(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 批量查询生产记录表 */
	bool QueryProduceRecordList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 批量查询生产记录表 */
	bool QueryProducePIDRecordList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 批量查询生产记录表 */
	bool QueryProducePIDRecordListNoToal(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 批量查询生产记录表 */
	bool QueryProduceRecordListNoToal(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 批量SN查询生产记录表 */
	bool QueryProduceSNRecordList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 查询生产记录总数 */
	bool QueryProduceRecordTotal(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 单笔查询生产记录详情 */
	bool QueryPrdRecord(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 批量查询动态库 */
	bool QueryDllList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 保存Dll文件*/
	bool SaveDllFile(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 获取文件下载路径 */
	bool GetDllPath(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 保存日志文件*/
	bool SaveLogFile(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 获取日志下载路径 */
	bool GetLogPath(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 获取标准日志下载路径 */
	bool GetStandLogPath(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 获取标准日志下载路径--批量用 */
	bool GetStandLogPathBath(const string paramap,string& result,string& strMinIOPath,bool throwexp);

	/* 获取日志下载路径--批量用 */
	bool GetLogPathBath(const string paramap,string& result,string& strMinIOPath,bool throwexp);

	/* 查询防切下载次数 */
	bool QueryDownEncTotal(CStr2Map& paramap,string log_type,CStr2Map& resultmap,bool throwexp);

	/* 创建工厂 */
	bool CreateFactory(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 批量查询工厂 */
	bool QueryFactoryList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 保存Dll文件*/
	bool SaveDllFile(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 查询用户所属工厂ID */
	bool QueryUserFac(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 更新工厂状态*/
	bool UpdateFactory(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 修改工厂名称*/
	bool ModifyFactory(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 批量查询web用户列表 */
	bool QueryWebUserList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 更新动态库状态*/
	bool UpdateDllState(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 查询错误对应解决 */
	bool QueryErrorSolve(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 测试查询错误对应解决 */
	bool QueryTestSolve(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 通用记录日志信息*/
	bool NormalRecordLog(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 记录采集数据*/
	bool RecordDataAcquisi(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 批量查询数据采集类型 */
	bool QueryCollectTypeList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 批量查询数据采集表 */
	bool QueryCollectList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 用PID批量模糊查询数据采集表 */
	bool QueryPidCollectList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 用PID批量查询数据采集表 */
	bool QueryConfPidCollectList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 单笔查询数据采集表 */
	bool QueryCollectPath(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 终端日志上传*/
	bool RecordTermLogUpload(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 射频日志上传*/
	bool ATELogUpload(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 固件下载规则录入*/
	bool FirmDownloadRuleSet(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 批量查询固件下载规则表 */
	bool QueryFirmRulesList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 批量查询固件下载记录表 */
	bool QueryFirmDownList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 单笔查询生产记录详情 */
	bool QueryFirmDownDetail(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 单笔查询固件下载规则 */
	bool QueryFirmDownRule(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 批量查询固件预拉取表--app用 */
	bool QueryFirmPreList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 批量查询固件信息表 */
	bool QueryFirmInfoList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 批量查询GO热点列表 */
	bool QueryGODownList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 单笔查询固件下载规则表 */
	bool QueryFirmRulesPath(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 更新固件下载的本地路径*/
	bool UpdateFirmDownloadPath(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 更新固件信息表本地路径*/
	bool UpdateFirmInfoPath(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 更新固件预拉取表*/
	bool UpdatePreFirmDownload(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 删除固件信息表*/
	bool DeleteFirmInfo(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* SVN拉取成功后插入固件信息*/
	bool InsertFirmInfo(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 终端获取固件下载地址*/
	bool GetFirmDownloadPath(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 更新固件下载记录 */
	bool CheckFirmDownLoadHash(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 终端固件更新完成通知 */
	bool TermFirmUpdateNofity(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 固件下载规则更新状态*/
	bool FirmDownloadRuleUpdate(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 用于批量处理加锁解锁 */
	bool CommSemPV(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 批量查询射频日志记录 */
	bool QueryAtmLogList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 获取射频日志下载路径 */
	bool GetAtmLogPath(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 判断是否GO下载*/
	bool GetDownloadMethod(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 终端关闭GO热点*/
	bool CloseTermGo(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 批量查询近7天生产数据 */
	bool QueryProductSevenList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 批量查询近7天采集数据 */
	bool QueryCollectSevenList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 批量生产流程统计记录表 */
	bool QueryReportRecordList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 批量时间范围内不同工厂 */
	bool QueryProductFactoryList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 批量查询时间范围内机型个数 */
	bool QueryProductModelList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 工厂生成月度数据获取--生成报表 */
	bool ProductMonthReport(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 插入报表生成记录到数据库 */
	bool InsertReportRecord(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 插入固件报告记录表 */
	bool InsertFirmReportRecord(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 更新固件报告记录表 */
	bool UpdateFirmReportRecord(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 生成固件下载报告 */
	bool FirmReportCreate(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 批量查询生产报表月份 */
	bool QueryReportMonthList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 获取单个下载报表路径*/
	bool GetReportDownloadPath(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 批量查询下载报表路径 */
	bool QueryReportDownList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 查询组装段的开始PID*/
	bool GetAssMinPid(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 查询ES索引名称*/
	bool QueryESIndex(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 单笔查询出货统计表 */
	bool QueryShipSum(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 跟新用户是否首次进入状态 */
	bool UpdateUserIsFirst(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 批量查询当前用户的模块顺序 */
	bool QueryUserModuleList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 查询机型是否存在 */
	bool CheckPosName(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 入库机型出货表 */
	bool InsertPosName(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 批量终端列表 */
	bool QueryTermList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 按照年份批量查询机型出货表 */
	bool QueryYearQuantityList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 按年份单笔查询出货总数 */
	bool QueryOfYearQuantity(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 批量查询机型评分表 */
	bool QueryTermScoreList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/*告警消息设置 */
	bool SetAlarm(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 批量查询消息通知人员列表 */
	bool QueryMessageUserList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 消息通知设置人员*/
	bool SetMessageBindUser(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 获取流水号*/
	bool GetSerialNum(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 批量查询消息列表 */
	bool QueryMessageList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 批量查询数据采集机型列表 */
	bool QueryTermNameList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 单笔查询机型站点情况 */
	bool QueryTermWorkNode(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 批量工厂生产概况 */
	bool QueryTermProduceList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 单笔查询机型站点情况 */
	bool QueryWorkNode(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 更新工厂生产概况表*/
	bool UpdateTermRecord(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 批量查询终端运行日志 */
	bool QueryTermRunList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 单笔查询终端运行日志 */
	bool QueryTermRun(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 单笔查询在线GO列表 */
	bool QueryTermOnline(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 单笔查询一键关机列表 */
	bool QueryTermAllShutDown(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 更新消息通知列表*/
	bool UpdateMessageList(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 单笔查询生产记录找机型 */
	bool QueryTermName(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 批量查询消息列表 */
	bool QueryExportTypeList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 批量查询采集记录-pid范围查询 */
	bool QueryPidRangCollectList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 批量查询终端运行日志-pid范围查询 */
	bool QueryPidRangTermRunList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 单笔查询生产记录找机型 */
	bool QueryMonthSum(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 获取机型生产统计 */
	bool GetTermProducInfo(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 终端GO日志上传*/
	bool TermGoLogUpload(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 创建生产订单变化记录*/
	bool CreateOrderChange(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 批量查询生产订单变化列表 */
	bool QueryOrderChangeList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 批量查询近7天测试详情列表 */
	bool QueryWeekCheckDetailList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 保存AP历史信息 */
	bool InsertAcApInfo(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 保存AC信息--统计用 */
	bool InsertAcControllerStats(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 保存AC峰值流量（入库时计算并存储） */
	// 数据库操作：INSERT INTO t_ac_controller_peak_traffic ... ON DUPLICATE KEY UPDATE ...
	bool InsertAcControllerPeakTraffic(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 单笔查询今天AC峰值流量 */
	// 数据库操作：SELECT Fpeak_traffic, Fpeak_time FROM t_ac_controller_peak_traffic WHERE Fac_ip = ? AND Fdate = ?
	bool QueryAcControllerPeakTraffic(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

	/* 批量查询AP信息列表 */
	bool QueryAcApInfoLatestList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray,bool throwexp);

	/* 查询AP今日流量 */
	bool QueryAcApInfoToday(CStr2Map& paramap, CStr2Map& resultmap, vector<CStr2Map>& vectmapArray, bool throwexp);

	/* 保存AP今日流量（入库时计算并存储） */
	// 数据库操作：INSERT INTO t_ac_ap_today_traffic ... ON DUPLICATE KEY UPDATE ...
	bool InsertAcApTodayTraffic(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 保存AC/AP流量趋势数据（每分钟一条记录） */
	bool InsertAcTrafficTrend(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp);

	/* 查询AC/AP流量趋势数据 */
	// 数据库操作：根据时间范围、AC IP、AP ID查询并聚合数据
	bool QueryAcTrafficTrendList(CStr2Map& paramap, CStr2Map& resultmap, vector<CStr2Map>& vectmapArray, bool throwexp);
}
#endif
