#ifndef AUTH_ERR_H
#define AUTH_ERR_H

#define ERR_SERVER_NO                 "20000"           //错误服务号
#define ERR_USER_HAS_REGISTER         "20001"          //该用户已被注册
#define ERR_USER_NOT_REGISTER         "20002"          //不存在的用户
#define ERR_WRITE_SESSION             "20003"          //保存session失败
#define ERR_USER_PASSWORD             "20004"          //密码错误
#define ERR_UPDATE_USER_LOGIN         "20005"          //更新用户登录信息失败
#define ERR_MKDIR_UPLOAD              "20006"           //创建文件目录失败
#define ERR_UPFILE_COUNT              "20007"           //上传文件数量不对
#define ERR_OVER_MAXUPLOADESIZE       "20008"           //上传文件大小异常
#define ERR_UPLOAD_FILENAME           "20009"           //上传文件后缀名异常
#define ERR_GET_DEPT                  "20010"           //未配置虚拟操作部门
#define ERR_GET_TELLER                "20011"           //未配置虚拟操作员
#define ERR_KEY_CONFIG_EMPTY          "20012"           //关键配置为空
#define ERR_NOT_REAL_USER             "20013"           //用户未实名认证
#define ERR_OPT                       "20014"           //错误操作
#define ERR_SAME_MOBILE               "20015"          //绑定手机号与原有一致
#define ERR_DISACCORD_PWD             "20016"           //新密码与确认密码不一致
#define ERR_UNKNOWN_USER              "20017"          //不存在的用户
#define ERR_VERIFY_CODE               "20018"          //验证码错误
#define ERR_VERIFYCODE_TIMEOUT        "20019"          //验证码过期
#define ERR_ENCODE_BASE               "20020"          //base64加密失败
#define ERR_DECODE_BASE               "20021"          //base64解密失败
#define ERR_UPDATE_PWD                "20022"          //更新密码失败
#define ERR_ACCOUNT_ACTIVE            "20023"          //账户激活失败
#define ERR_USER_NOT_LOGIN            "20024"          //用户未登录
#define ERR_UPDATE_USER_LOGOUT        "20025"          //更新用户退出信息失败
#define ERR_USER_MOBILE               "20026"          //用户手机号不正确
#define ERR_USER_REAL_NAME            "20027"          //用户未实名
#define ERR_USER_UNBIND_MOBILE        "20028"          //用户未绑定手机号
#define ERR_SYS_ERROR                 "20029"          //系统繁忙
#define ERR_RSA_DECRYPT_ERROR         "20030"          //RSA解密失败
#define ERR_DATA_FORMAT               "20031"          //参数格式不对
#define ERR_USER_HAS_ID_NO            "20032"          //该身份证已绑定
#define ERR_USER_ID_NO_NUM            "20033"          //用户实名次数已用完
#define ERR_USER_ID_NO_VALUE          "20034"          //用户身份证号码错误
#define ERR_USER_LIMIT_IP             "20035"          //用户登录IP受限
#define ERR_VMDECRYPT_NOT_VALID       "20036"          //解密后的字段不合法
#define ERR_VMDECRYPT_COMBINATIONA_NOT "20037"          //解密后的字段与组合内容不相等
#define ERR_SIGNATURE_INCORRECT        "20038"          //签名不正确   
#define ERR_TRANS_TIME                 "20039"          //交易时间错误
#define ERR_VERSION_ERROR              "20040"          //app版本号不对
#define ERR_DLLFILE_ERROR              "20041"          //dll文件校验不过
#define ERR_MD5FILE_ERROR              "20042"          //文件MD5校验不过
#define ERR_NO_LOWER_LEVER             "20043"          //没用权限

#endif
