#ifndef CAUTHRELAYAPI_H_
#define CAUTHRELAYAPI_H_
#include "relaycomm/CRelayCall.h"
#include "tools/commdef.h"
#include "tools/CTrsExp.h"

#define TIMEOUT_RSA 10 //RSA加密超时时间单位秒

#define TRANSCODE_QUERY_UID_BY_UIN        "100001" //通过uin查询uid
#define TRANSCODE_SEND_PHONE_VERICODE     "100002" //发送手机校验码
#define TRANSCODE_CREATE_USER_INFO        "100003" //用户注册
#define TRANSCODE_USER_LOGIN              "100004" //用户登录验证
#define TRANSCODE_UPDATE_USER_LOGIN       "100005" //用户登录成功通知
#define TRANSCODE_UPDATE_USER_LOGOUT      "100006" //用户退出成功通知
#define TRANSCODE_CREATE_PWD_TRAN         "100007" //设置交易密码
#define TRANSCODE_MODIFY_PWD              "100008" //修改密码
#define TRANSCODE_SET_PWD_APP             "100009" //修改APP密码
#define TRANSCODE_SET_NEW_PWD             "100010" //密码找回
#define TRANSCODE_CREATE_AUTH_INFO        "100011" //重置APP用户密码
#define TRANSCODE_BIND_BANK_CARD          "100012" //绑定银行卡
#define TRANSCODE_SET_AUTH_EMAIL          "100013" //设置邮箱
#define TRANSCODE_SET_AUTH_MOBILE         "100014" //设置手机号-绑定解绑
#define TRANSCODE_QUERY_LOGIN_INFO        "100015" //查询用户登录信息
#define TRANSCODE_MODIFY_EMAIL            "100016" //修改邮箱
#define TRANSCODE_CHECK_PHONE_VERICODE    "100019" //手机验证码验证
#define TRANSCODE_UNBIND_BANK_CARD        "100022" //解绑银行卡
#define TRANSCODE_CHECK_TRAN_PWD          "100024" //检查交易密码
#define TRANSCODE_UPDATE_REAL_NAME        "100026" //更新用户实名信息


#define TRANSCODE_CHECK_USER_IDENTITY     "100001" //用户身份认证，发送前置机


#define TRANSCODE_QUERY_USER_INFO         "100005" //查询用户实名认证信息交易码
#define TRANSCODE_UPDATE_MOBILE           "100007" //修改用户手机号码交易码
#define TRANSCODE_UPDATE_ACTIVE_CODE      "100012" //更新激活码
#define TRANSCODE_USER_ACTIVE             "100013" //账户激活
#define TRANSCODE_CHECK_USER_ANSWER       "100015" //验证密保问题
#define TRANSCODE_GETOTAB_CODE            "100018" //获取TOTP密钥
#define TRANSCODE_CHECK_USERNAME_EXIST    "100027" //检查用户名是否注册
#define TRANSCODE_TEST_UPDATE             "100028" //更新用户信息
#define TRANSCODE_TEST_QUERY              "100029" //查询用户信息

#define TRANSCODE_GY_USER_LOGIN           "100035"
#define TRANSCODE_GY_USER_LOGOUT          "100036"
#define TRANSCODE_GY_MODIFY_PASSWD        "100037"
#define TRANSCODE_GY_UPDATE_CMER          "100039"
#define TRANSCODE_GY_ADD_USER             "100040"
#define TRANSCODE_GY_ADD_CMER_PAY         "100041"
#define TRANSCODE_GY_UPDATE_CMER_PAY      "100042"
#define TRANSCODE_GY_BIND_CARD            "100043"
#define TRANSCODE_GY_APP_USER_REG         "100044" //app用户注册
#define TRANSCODE_GY_QR_ADD_CMER          "100045" //二维码商户注册

#define TRANSCODE_COMM_QUERY              "100001" //公共查询交易码


#define TRANSCODE_APP_USER_LOGIN    "100050"   //APP用户登录
#define TRANSCODE_APP_USER_CREATE    "100051"   //APP用户登录
#define TRANSCODE_APP_USER_UP_STATE    "100052"   //APP用户修改状态



namespace CAuthRelayApi
{
    /*
     * 声明项目中某个SERVICE的RequestType。
     */
    const static int CDH_COMM_QUERY = 1000;

    const static int CDH_ADS = 1001;
    const static int RQ_QUERY_USER = 1001;
    const static int RQ_SEND_PHONE_VERICODE = 1002;
    const static int RQ_CREATE_USER_INFO = 1003;
    const static int RQ_USER_LOGIN = 1004;
    const static int RQ_UPDATE_USER_LOGIN = 1005;
    const static int RQ_UPDATE_USER_LOGOUT = 1006;
    const static int RQ_CREATE_PWD_TRAN = 1007;
    const static int AU_MODIFY_PWD = 1008;
    const static int AU_MODIFY_PWD_APP = 1009;
    const static int AU_SET_NEW_PWD= 1010;
    const static int AU_CREATE_AUTH_INFO = 1011;
    const static int AU_BAND_BANK_CARD = 1012;
    const static int AU_SET_AUTH_EMAIL = 1013;
    const static int AU_SET_AUTH_MOBILE = 1014;
    const static int AU_QUERY_LOGIN_INFO = 1015;
    const static int AU_MODIFY_EMAIL = 1016;
    const static int AU_CHECK_PHONE_VERICODE = 1019;
    const static int AU_UNBIND_BANK_CARD = 1022;
    const static int AU_CHECK_TRAN_PWD = 1024;
    const static int AU_UPDATE_REAL_NAME = 1026;
    const static int AU_CHECK_USERNAME_EXIST = 1027;
    const static int AU_TEST_UPDATE = 1028;
    const static int AU_TEST_QUERY = 1029;

    const static int AU_GY_USER_LOGIN = 1035;
    const static int AU_GY_USER_LOGOUT = 1036;
    const static int AU_GY_MODIFY_PASSWD = 1037;
    const static int AU_GY_ADD_CMER = 1038;
    const static int AU_GY_UPDATE_CMER = 1039;
    const static int AU_GY_ADD_USER = 1040;
    const static int AU_GY_ADD_CMER_PAY = 1041;
    const static int AU_GY_UPDATE_CMER_PAY = 1042;
    const static int AU_GY_BIND_CARD = 1043;
    const static int AU_GY_APP_USER_REG = 1044;
    const static int AU_GY_QR_ADD_CMER = 1045;
    const static int RQ_THREE_ELEMENTS = 1046;

    const static int AU_CHECK_USER_IDENTITY = 4001 ;//发送前置机，用户身份认证

    const static int AU_UPDATE_MOBILE = 1007;
    const static int AU_QUERY_USER_INFO = 1008;
    const static int AU_UPDATE_ACTIVE_CODE = 1012;
    const static int AU_USER_ACTIVE = 1013;

	/* 以下是PASS系统用*/
	const static int AU_APP_USER_LOGIN = 1050;
	const static int AU_APP_CREATE_USER = 1051;
	const static int AU_APP_UPDATE_STATE = 1052;

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

    /*
     * pParams, 是一组字符串指针,字符串以NULL结尾
     * paramMap : 参数容器
     * 如果在paramMap不存在指定的参数,直接抛异常,表示
     * 接口调用基本条件不满足
     */
    void CheckParams(const char** pParams, CStr2Map& paramMap,
            const int& iRequestType, bool bStrongCheck = false);

    /*  查询用户登录信息 */
    bool QueryLoginInfo(const string& uin,CStr2Map& resultmap,bool throwexp = true);
	/*  通过UIN查询用户UID */
    string QueryUidByUin(const string& uin);
	/*  通过用户名查询用户UID */
    string QueryUidByName(const string& name,bool throwexp = true);

	/*  通过用户名查询用户地址 */
	string QueryAddressBYID(const string& id,bool throwexp = true);
	
	/*  通过id_no查询UID */
    string QueryUidByIDNumber(const string& id_no,bool throwexp = true);
	/*  通过用户UID查询密保问题 */
    string QueryAnswerByUid(const string& uid,bool throwexp = true);
	/*  通过用户UID查询用户真实姓名 */
    string QueryAuthRealNameByUid(const string& uid,bool throwexp = true);
	/*  创建用户注册信息 */
    bool CreateUserInfo(const CStr2Map& paramap,CStr2Map& resultmap,bool throwexp = true);
	/*  用户登陆验证 */
    bool UserLogin(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp = true);

	/*  APP用户登陆验证 */
	bool AppUserLogin(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp = true);

	/*  创建APP用户 */
    bool CreateAppUser(const CStr2Map& paramap,CStr2Map& resultmap,bool throwexp = true);

	/*  修改APP用户状态 */
    bool DisableAppState(const CStr2Map& paramap,CStr2Map& resultmap,bool throwexp = true);

	/*  用户登陆成功通知 */
    bool UpdateUserLogin(const CStr2Map& paramap);
	/*  用户退出成功通知 */
    bool UpdateUserLogout(const CStr2Map& paramap);
	/*  创建用户交易密码 */
    bool CreatePwdTran(const CStr2Map& paramap,CStr2Map& resultmap,bool throwexp = true);
	/*  查询用户基本信息 */
    bool QueryUserBaseInfo(const string& uid,CStr2Map& resultmap,bool throwexp = true);
	/*  重置APP用户密码 */
    bool ResetAppPwd(const CStr2Map& paramap,CStr2Map& resultmap,bool throwexp = true);
	/*  更新用户实名认证信息 */
    bool UpdateAuthInfo(const CStr2Map& paramap,CStr2Map& resultmap,bool throwexp = true);
    bool UpdateMobile(const CStr2Map& paramap,CStr2Map& resultmap,bool throwexp = true);
    //bool QueryLoginInfo(const string& uid,CStr2Map& resultmap,bool throwexp = true);
    // 查询用户实名信息
    bool QueryRealNameInfo(const string& uid,CStr2Map& resultmap,bool throwexp = true);
	/*  设置手机号-绑定或解绑 */
    bool SetAuthMobile(const CStr2Map& paramap,CStr2Map& resultmap,bool throwexp = true);
	/*  绑定银行卡 */
    bool BindBankCard(const CStr2Map& paramap,CStr2Map& resultmap,bool throwexp = true);
	/*  设置邮箱 */
    bool SetAuthEmail(const CStr2Map& paramap,CStr2Map& resultmap,bool throwexp = true);
	/*  修改邮箱 */
    bool ModifyEmail(const CStr2Map& paramap,CStr2Map& resultmap,bool throwexp = true);
	/*  修改APP用户密码 */
    bool ModifyAppPwd(const CStr2Map& paramap,CStr2Map& resultmap,bool throwexp = true);
    bool UpdateActiveCode(const CStr2Map& paramap,bool throwexp = true);
    bool UserActive(const CStr2Map& paramap,bool throwexp = true);
	/*  修改密码 */
    bool ModifyPassword(const CStr2Map& paramap,CStr2Map& resultmap,bool throwexp = true);
	/*  密码找回 */
    bool SetNewPassword(const CStr2Map& paramap,CStr2Map& resultmap,bool throwexp = true);
    string QueryActiveCode(const string& uid,bool throwexp=true);
    bool CheckUserAnswer(CStr2Map& paramap,bool throwexp = true);
	/*  发送手机校验码 */
	bool SendPhoneVericode(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp = true);
	/*  手机验证码验证 */
	bool CheckPhoneVericode(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp = true);
	/*  检查交易密码 */
	bool CheckTranPwd(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp = true);
	/*  检查用户身份证 */
	bool CheckUserIdentity(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp = true);
	/*  解绑银行卡 */
	bool UnbindBankCard(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp = true);
	/*  查询用户安全状态 */
	bool QueryAuthSafeStatus(const string& uid,CStr2Map& resultmap,bool throwexp = true);
	/*  查询用户IP限制表 */
	bool QueryAuthIPInfo(const string& ip,CStr2Map& resultmap,bool throwexp = true);
	/*  查询用户推荐表 */
	string QueryAuthRecommend(const string& uid,bool throwexp = true);
	/*  用户推荐好友批量查询 */
	bool QueryUserRecommendList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray, bool throwexp = true);

	/*  检查用户是否注册 */
	bool CheckUsernameExist(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp = true);

	/*  测试更新接口 */
	bool TestUpdate(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp = true);
	/*  测试查询接口 */
	bool TestQuery(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp = true);

	bool QueryIPList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray, bool throwexp = true);

	bool QueryGypayHmjrOrder(CStr2Map& paramap,CStr2Map& resultmap,bool throwexp);

    bool QueryGypayHmjrOrderList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray, bool throwexp = true);

    bool GYUserLogin(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp = true);
    bool GYUserLogout(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp = true);
    bool GYModifyPasswd(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp = true);
    bool GYAddCmer(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp = true);
    bool GYUpdateCmer(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp = true);
    bool GYAddUser(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp = true);
    bool GYAddCmerPay(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp = true);
    bool GYUpdateCmerPay(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp = true);
    bool GYBindCard(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp = true);
    bool AppUserReg(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp = true); 	
    bool QrAddCmer(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp = true);    
	
	
	bool QueryThreeElements(const CStr2Map& paramap,CStr2Map& resultmap,bool throwexp = true);

	bool QueryAppUserList(CStr2Map& paramap,CStr2Map& resultmap,vector<CStr2Map>& vectmapArray, bool throwexp = true);

	/*  检查最新可用版本号 */
	bool QueryVerison(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp = true);

    /*  获取TOTP密钥 */
	bool getTotpSecret(const CStr2Map& paramap, CStr2Map& resultmap, bool throwexp = true);

    

	
}
#endif /* CAUTHRELAYAPI_H_ */
