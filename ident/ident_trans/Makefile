include ../../Makefile.conf

COMMINC= ../ident_comm
COMMLIB= $(COMMINC)
RELAYINC= ../ident_cli
RELAYLIB= $(RELAYINC)
AUTHRELAYINC= ../../auth/auth_cli
AUTHRELAYLIB= ../../auth/auth_cli
CGILIBLINK= -lidentcomm -lidentrelayapi -lauthrelayapi
SESSIONLINK = -lsessionapi -ladsmemcli -lmemcached -lcgihtml
LIBICONV = -liconv -laps.base
CGIBINDIR=/home/httpd/web/web_pay5/app
POLLBINDIR=/home/httpd/web/web_pay6/app


all:  pass_send.cgi \
      result_notify.cgi \
      qry_produce_record_list.cgi \
      upload_dll.cgi \
      qry_prd_record.cgi \
      download_log.cgi \
      qry_dll_list.cgi \
      download_dll.cgi \
      upload_log.cgi \
      query_log.cgi \
      report_week_type.cgi \
      create_factor.cgi \
      qry_factory_list.cgi \
      update_factory.cgi \
      modify_factory.cgi \
      qry_web_user_list.cgi \
      update_dll_state.cgi \
      norm_upload_log.cgi \
      query_stand_log.cgi \
	  download_stand_log.cgi \
      data_acquisition.cgi \
	  query_data_collection_list.cgi \
	  query_collect_funclist.cgi \
      data_collection_file_deal.cgi \
	  term_log_upload.cgi \
	  qry_down_rules_list.cgi \
	  get_firm_version.cgi \
	  get_firm_down_path.cgi \
	  firm_download_set.cgi \
	  firm_check_hash.cgi \
	  term_firm_update_notice.cgi \
	  query_svn_files_list.cgi \
	  svn_retry_firm.cgi \
	  query_firm_down_list.cgi \
      Ate_log_upload.cgi \
	  qry_atm_log_list.cgi \
	  download_ate_log.cgi \
	  term_close_go.cgi \
      query_firm_down_details.cgi \
      qry_report_month_list.cgi \
      get_produce_report.cgi \
      qry_k3_aging_order_list.cgi \
      material_reverse_check_list.cgi \
      query_material_get_svn.cgi \
      query_collection_type_list.cgi \
      download_collection_files.cgi \
      query_collection_data.cgi \
      query_overall_list.cgi \
      data_collect_count_report.cgi \
      weekly_data_collect_count.cgi \
      query_module_pass_m_rate_report.cgi \
      query_annual_share_report.cgi \
      query_term_score_report.cgi \
      query_message_user_list.cgi \
      set_message_use.cgi \
      query_notice_list_report.cgi \
      get_document_describe_report.cgi \
      get_term_name_list.cgi \
      get_data_acquisition_report.cgi \
      alarm_set.cgi \
      query_is_first_report.cgi \
      query_work_node.cgi \
      query_work_node_data.cgi \
      query_term_run_file.cgi \
      download_point_log.cgi \
	  query_export_type_list.cgi \
	  export_data.cgi \
	  query_signle_term_report.cgi \
	  get_term_produce_report.cgi \
	  term_go_log_upload.cgi \
	  query_data_export_count.cgi \
	  get_memcache_pid_complete.cgi  \
	  query_active_data_list.cgi \
	  query_week_detail_list.cgi  \
	  query_collection_data_list.cgi \
	  ac_controller_stats.cgi \
	  ac_controller_routers_list.cgi \
	  ac_controller_realtime.cgi \
	  ac_controller_traffic_trend.cgi \
	  ac_controller_restart.cgi \
	  ac_controller_quality.cgi \
	  get_K3_pid_end.cgi \
	  ac_trap_message_list.cgi \
	  query_year_summary_report.cgi 



test.cgi: CTest.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

pass_send.cgi: CPassSend.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

result_notify.cgi: CResultNotify.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

qry_produce_record_list.cgi: CQryProduceRecordList.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

upload_dll.cgi: CUpLoadDll.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

qry_prd_record.cgi: CQueryPrdRecord.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

download_log.cgi: CDownLoadLog.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

qry_dll_list.cgi: CQueryDllList.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@


download_dll.cgi: CDownLoadDll.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

upload_log.cgi: CUpLoadLog.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

query_log.cgi: CQueryLog.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

report_week_type.cgi: CReportWeekType.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

create_factor.cgi: CCreateFactory.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

qry_factory_list.cgi: QryFactoryList.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

update_factory.cgi: CUpdateFactory.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

modify_factory.cgi: CModifyFactory.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

qry_web_user_list.cgi: CQueryWebUserList.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

update_dll_state.cgi: CUpdateDllState.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

norm_upload_log.cgi: CNormUpload.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

query_stand_log.cgi: CQueryStandLog.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

download_stand_log.cgi: CDownLoadStandLog.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

data_acquisition.cgi: CDataAcquisition.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

query_data_collection_list.cgi: CQueryDataCollectionList.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

query_collect_funclist.cgi: CQueryCollectFuncList.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

data_collection_file_deal.cgi: CDataCollectionFileDeal.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

getsnmp.cgi: CGetSNMP.o
	$(CXX) -g  $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
#	strip $@

term_log_upload.cgi: CTermLogUpload.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

qry_down_rules_list.cgi:CQueryDownRulesList.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

get_firm_version.cgi:CGetFirmVersion.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

get_firm_down_path.cgi:CGetFirmDownPath.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

firm_download_set.cgi:CFirmDownloadSet.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

firm_check_hash.cgi:CFirmCheckHash.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

term_firm_update_notice.cgi:CTermFirmUpdateNotice.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

query_svn_files_list.cgi:CQuerySvnFilesList.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

svn_retry_firm.cgi:CSvnRetryFirm.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

query_firm_down_list.cgi:CQueryFirmDownList.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

Ate_log_upload.cgi:CAteLogUpload.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

qry_atm_log_list.cgi:CQryAtmLogList.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

download_ate_log.cgi:CQueryAtmLog.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

term_close_go.cgi:CTermCloseGo.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

query_firm_down_details.cgi:CQueryFirmDownDetails.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

qry_report_month_list.cgi:CQryReportMonthList.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

get_produce_report.cgi:CGetProduceReport.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

qry_k3_aging_order_list.cgi:CQryK3AgingOrderList.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

material_reverse_check_list.cgi:CMaterialReverseCheckList.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

query_material_get_svn.cgi:CQueryMaterialGetSvn.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

query_collection_type_list.cgi:CQueryCollectionTypeList.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

download_collection_files.cgi:CDownloadCollectionFiles.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

query_collection_data.cgi:CQueryCollectionData.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

query_overall_list.cgi:CQueryOverallList.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

data_collect_count_report.cgi:CDataCollectCountReport.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

weekly_data_collect_count.cgi:CWeeklyDataCollectCount.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

query_module_pass_m_rate_report.cgi:CQueryModulePassMRateReport.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

query_annual_share_report.cgi:CQueryAnnualShareReport.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

query_term_score_report.cgi:CQueryTermScoreReport.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

query_message_user_list.cgi:CQueryMessageUserList.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

set_message_use.cgi:CSetMessageUse.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

query_notice_list_report.cgi:CQueryNoticeListReport.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

get_document_describe_report.cgi:CGetDocumentDescribeReport.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

get_term_name_list.cgi:CGetTermNameList.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

get_data_acquisition_report.cgi:CGetDataAcquisitionReport.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

alarm_set.cgi:CAlarmSet.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

query_is_first_report.cgi:CQueryIsFirstReport.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

query_work_node.cgi:CQueryWorkNode.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

query_work_node_data.cgi:CQueryWorkNodeData.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

query_term_run_file.cgi:CQueryTermRunFile.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

download_point_log.cgi:CDownloadPointLog.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

query_export_type_list.cgi:CQueryExportTypeList.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

export_data.cgi:CExportData.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

query_signle_term_report.cgi:CQuerySignleTermReport.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

get_term_produce_report.cgi:CGetTermProduceReport.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

term_go_log_upload.cgi:CTermGoLogUpload.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@
 
query_data_export_count.cgi:CQueryDataExportCount.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

get_memcache_pid_complete.cgi:CGetMemcachePidComplete.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

query_active_data_list.cgi:CQueryActiveDataList.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

query_week_detail_list.cgi:CQueryWeekDetailList.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

query_collection_data_list.cgi:CQueryCollectionDataList.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

ac_controller_stats.cgi:CAcControllerStats.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

ac_controller_routers_list.cgi:CAcControllerRoutersList.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

ac_controller_realtime.cgi:CAcControllerRealtime.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

ac_controller_traffic_trend.cgi:CAcControllerTrafficTrend.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

ac_controller_restart.cgi:CAcControllerRestart.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

ac_controller_quality.cgi:CAcControllerQuality.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

get_K3_pid_end.cgi:CGetK3PidEnd.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

ac_trap_message_list.cgi:CAcTrapMessageList.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

query_year_summary_report.cgi:CQueryYearSummaryReport.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(AUTHRELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) $(LIBICONV)
	strip $@

cp:
	cp *.cgi $(CGIBINDIR)

cp6:
	cp ac_* $(POLLBINDIR)

%.o: %.cpp
	$(CXX) $(COMOPT) $(BASICINC) -I$(RELAYINC) -I$(AUTHRELAYINC) -I$(COMMINC) $<

clean:
	rm -rf *.o *.cgi
