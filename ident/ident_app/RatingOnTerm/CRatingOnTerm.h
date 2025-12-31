/************************************************************
 Desc:     根据t_firmware_download_rules表发获取固件包
 Auth:     leize
 Modify:
 data:     2024-12-26
 ***********************************************************/
#ifndef CRATINGONTERM_H_
#define CRATINGONTERM_H_

#include <base/file.hpp>
#include "CIdentAppComm.h"
#include "xml/unicode.h"

class CRatingOnTerm
{
    public:
    CRatingOnTerm();
    virtual ~CRatingOnTerm();
    int Commit(CStr2Map & inMap,CStr2Map &outMap);
    string GetTid(){
        return string("app_Rating_On_Term");
    };
 
private:
    string svnUser;//SVN用户名
    string svnPassWd;////SVN密码
};

#endif 
