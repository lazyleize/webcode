/************************************************************
 Desc:     根据t_firmware_download_rules表发获取固件包
 Auth:     leize
 Modify:
 data:     2024-12-26
 ***********************************************************/
#ifndef CGETFIRMPACK_H_
#define CGETFIRMPACK_H_

#include <base/file.hpp>
#include "CIdentAppComm.h"
#include "xml/unicode.h"

class CGetFirmPack
{
    public:
    CGetFirmPack();
    virtual ~CGetFirmPack();
    int Commit(CStr2Map & inMap,CStr2Map &outMap);
    string GetTid(){
        return string("app_GetSVN_Firmware");
    };
 
    void DelMapF(CStr2Map& dataMap,CStr2Map& outMap);
	bool dealSVNPackGet(const string& svn_path, string& target_path, const string& username, const string& password, string& strfilename, string& fileSizeStr);//从SVN拉取固件
    bool dealSVNPackGet(const string& svn_path, const string& localPath, string& target_path, vector<aps::FileInfo>& zipfiles);
    
private:
    string svnUser;//SVN用户名
    string svnPassWd;////SVN密码
};

#endif 
