include ../../Makefile.conf

COMMINC= ../auth_comm
COMMLIB= $(COMMINC)
RELAYINC= ../auth_cli
RELAYLIB= $(RELAYINC)
CGILIBLINK= -lauthcomm -lauthrelayapi -ladsenc
SESSIONLINK = -lsessionapi -ladsmemcli -lmemcached -lcgihtml -liconv -laps.base

#CGIBINDIR=/home/httpd/web/web_jifen/cgi-bin
CGIBINDIR=/home/httpd/web/web_pay5/app

#get_public_key.cgi check_rsa_pwd.cgi 

all: check_user_exist.cgi \
    create_user.cgi \
    user_login.cgi \
    user_logout.cgi \
    query_login_info.cgi \
    modify_email.cgi \
    get_verify_code.cgi \
    check_verify_code.cgi \
    modify_pwd.cgi \
    set_new_pwd.cgi \
    query_user_exist.cgi \
    check_real_name.cgi \
    query_register_info.cgi \
    query_auth_recommend.cgi \
    query_auth_user_baseinfo.cgi \
    check_user_answer.cgi \
    send_phone_vericode.cgi \
    send_email.cgi \
    set_auth_email.cgi \
    set_auth_mobile.cgi \
    create_pwd_tran.cgi \
    query_pwd_question.cgi \
    query_auth_real_name.cgi \
    check_tran_pwd.cgi \
    query_auth_safe_stats.cgi \
    test.cgi \
    check_username_exist.cgi \
    test_update.cgi \
    test_query.cgi \
    query_ip_list.cgi \
    app_user_reg.cgi \
    get_datetime.cgi \
    app_login.cgi \
    create_app_user.cgi \
    query_app_user_list.cgi \
    disable_app.cgi \
    modify_pwd_app.cgi \
    reset_pwd_app.cgi \
	user_get_totp_secret.cgi 


check_user_exist.cgi: CCheckUserExist.o 
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

create_user.cgi: CCreateUserInfo.o 
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

user_login.cgi: CUserLogin.o 
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

user_logout.cgi: CUserLogout.o 
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

create_auth_real_name.cgi: CCreateAuthInfo.o 
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

update_mobile.cgi: CUpdateMobile.o 
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

query_login_info.cgi: CQueryLoginInfo.o 
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

modify_email.cgi: CModifyEmail.o 
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

set_pwd_question.cgi: CSetPwdQuestion.o 
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

get_active_code.cgi: CGetActiveCode.o 
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

account_active.cgi: CAccountActive.o 
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

get_verify_code.cgi: CGetVerifyCode.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

get_public_key.cgi: CGetPublicKey.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

check_rsa_pwd.cgi: CCheckRsaPwd.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

check_verify_code.cgi: CCheckVerifyCode.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

modify_pwd.cgi: CModifyPwd.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

set_new_pwd.cgi: CSetNewPwd.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

bind_mobile_email.cgi: CBindMobileEmail.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

query_user_exist.cgi: CQueryUserExist.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

check_real_name.cgi: CCheckRealName.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

query_register_info.cgi: CQueryRegisterInfo.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

check_user_answer.cgi: CCheckUserAnswer.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

send_phone_vericode.cgi: CSendPhoneVericode.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) 
	strip $@

send_email.cgi: CSendUserEmail.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK) 
	strip $@

bind_bank_card.cgi: CBindBankCard.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

set_auth_email.cgi: CSetAuthEmail.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

set_auth_mobile.cgi: CSetAuthMobile.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

check_phone_vericode.cgi: CCheckPhoneVericode.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

create_pwd_tran.cgi: CCreatePwdTran.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

query_pwd_question.cgi: CQueryAnswerInfo.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

query_auth_real_name.cgi: CQueryAuthRealName.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

query_auth_user_baseinfo.cgi: CQueryUserBaseInfo.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

query_auth_safe_stats.cgi: CQueryAuthSafeStatus.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

query_auth_recommend.cgi: CQueryUserRecommendList.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

check_tran_pwd.cgi: CCheckTranPwd.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

test.cgi: CTest.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

check_username_exist.cgi: CCheckUsernameExist.o 
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

test_update.cgi: CTestUpdate.o 
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@
test_query.cgi: CTestQuery.o 
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@
query_ip_list.cgi: CQueryIPList.o 
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

app_user_reg.cgi: CAppUserReg.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

get_datetime.cgi: CGetDateTime.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

app_login.cgi:CAppLogin.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

create_app_user.cgi:CCreateAppUser.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

query_app_user_list.cgi:CQueryAppUserList.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

disable_app.cgi:CDisableApp.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

modify_pwd_app.cgi:CModifyPwdApp.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

reset_pwd_app.cgi:CResetPwdApp.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

user_get_totp_secret.cgi:CUserGetTotpSecret.o
	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
	strip $@

#session.cgi: Csession.o 
#	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
#	strip $@

#session_test.cgi: CSessionTest.o 
#	$(CXX) $(CFLAGS) -o $@ $^ -L$(RELAYLIB) -L$(COMMLIB) -L$(BASICLIB) $(CGILIBLINK) $(BASICLIBLINK) $(SESSIONLINK)
#	strip $@
cp:
	cp *.cgi $(CGIBINDIR)

%.o: %.cpp
	$(CXX) $(COMOPT) $(BASICINC) -I$(RELAYINC) -I$(COMMINC) $<

clean:
	rm -rf *.o *.cgi
