/************************************************************
 Desc:     根据t_firmware_download_rules表发获取固件包
 Auth:     leize
 Modify:
 data:     2024-12-26
 ***********************************************************/
#ifndef CYEARONTERMSHIP_H_
#define CYEARONTERMSHIP_H_

#include <base/file.hpp>
#include "CIdentAppComm.h"
#include "xml/unicode.h"

class CYearOnTermShip
{
    public:
    CYearOnTermShip();
    virtual ~CYearOnTermShip();
    int Commit(CStr2Map & inMap,CStr2Map &outMap);
    string GetTid(){
        return string("app_Year_Term_ship");
    };
 
private:
    string svnUser;//SVN用户名
    string svnPassWd;////SVN密码
};

#endif 
