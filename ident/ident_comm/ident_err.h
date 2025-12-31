#ifndef IDENT_ERR_H
#define IDENT_ERR_H

#define ERR_SERVER_NO                 "3300"  //错误服务号
#define ERR_USER_HAS_REGISTER         "3301"  //该用户已被注册
#define ERR_WRITE_SESSION             "3302"  //保存session失败
#define ERR_USER_PASSWORD             "3303"  //密码错误
#define ERR_UPDATE_USER_LOGIN         "3304"  //更新用户登录信息失败
#define ERR_MKDIR_UPLOAD              "3305"  //创建文件目录失败
#define ERR_UPFILE_COUNT              "3306"  //上传文件数量不对
#define ERR_OVER_MAXUPLOADESIZE       "3307"  //上传文件大小异常
#define ERR_UPLOAD_FILENAME           "3308"  //上传文件后缀名异常
#define ERR_USER_NOT_REGISTER         "3309"  //不存在的用户
#define ERR_GET_DEPT                  "3310"  //未配置虚拟操作部门
#define ERR_GET_TELLER                "3311"  //未配置虚拟操作员
#define ERR_KEY_CONFIG_EMPTY          "3312"  //关键配置为空
#define ERR_NOT_REAL_USER             "3313"  //用户未实名认证
#define ERR_OPT                       "3314"  //错误操作
#define ERR_SAME_MOBILE               "3315"  //绑定手机号与原有一致
#define ERR_DISACCORD_PWD             "3316"  //新密码与确认密码不一致
#define ERR_UNKNOWN_USER              "3317"  //不存在的用户
#define ERR_USER_HAS_ID_NO            "3318"  //该身份证已绑定
#define ERR_USER_ID_NO_VALUE          "3319"  //用户身份证号码错误


#define ERR_VERIFY_CODE               "3320"  //验证码错误
#define ERR_VERIFYCODE_TIMEOUT        "3321"  //验证码过期
#define ERR_ENCODE_BASE               "3322"  //base64加密失败
#define ERR_DECODE_BASE               "3323"  //base64解密失败
#define ERR_UPDATE_PWD                "3322"  //更新密码失败
#define ERR_ACCOUNT_ACTIVE            "3323"  //账户激活失败
#define ERR_USER_NOT_LOGIN            "3324"  //用户未登录
#define ERR_MOBILE_INVALID            "3325"  //手机号码不合法  

#define ERR_JAVA_RETURN               "3326"  //JAVA前置返回失败
#define ERR_NO_LOWER_LEVER            "3327"  //没有下级了
#define ERR_JIFEN_USER_NO_AUTH        "3328"  //没有权限
#define ERR_SIGNATURE_INCORRECT       "3329"  //签名不正确
#define ERR_DATA_FORMAT               "3330"  //时间格式不正确   
#define ERR_QRCODE_MAKE_FAIL          "3331"  //生成二维码失败   
#define ERR_REQ_NOT_NULL              "3332"  //请求为空
#define ERR_FILE_NOT_EXIST            "3333"  //文件不存在
#define ERR_CALL_JAVA_FAIL            "3334"  //调用JAVA失败
#define ERR_CONN_JAVA_FAIL            "3335"  //与JAVA通讯失败
#define ERR_SYSCODE_INCORRECT         "3336"  //系统码错误
#define ERR_BATCH_NUM_NOT_MATCH       "3337"  //笔数不对
#define ERR_COMMUNICATION_FAIL        "3338"  //笔数不对
#define ERR_TRANS_TIME                "3339"  //transtime不对
#define ERR_ACK_NOTIFY_FAIL           "9999"  //确认失败


#endif
