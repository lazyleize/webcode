/************************************************************
 Desc:     根据t_firmware_download_rules表发获取固件包
 Auth:     leize
 Modify:
 data:     2024-12-26
 ***********************************************************/
#ifndef CGETFIRMPACK_H_
#define CGETFIRMPACK_H_

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
	//bool dealSVNPackGet(const string& svn_path, string& target_path, const string& username, const string& password, string& strfilename, string& fileSizeStr);//从SVN拉取固件
    bool UpdateFirmRules(CStr2Map& inMap);//更新记录
    long getPackageSize(const std::string& svn_url,const string& username, const string& password);
    //long getFileSize(const std::string& path);
};

#endif 
