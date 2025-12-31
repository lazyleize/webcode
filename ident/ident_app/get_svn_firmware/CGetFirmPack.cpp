/************************************************************
 Desc:     用SVN去获取固件包
 Auth:     leize
 Modify:
 data:     2025-01-20
 ***********************************************************/
#include "adstcp.h"
#include "CGetFirmPack.h"
#include "CIdentRelayApi.h"
#include "CTools.h"
#include "CSmtp.h"

#include <base/all.hpp>

using namespace aps;

extern CIdentAppComm* pIdentAppComm;
CGetFirmPack::CGetFirmPack()
{
}
CGetFirmPack::~CGetFirmPack()
{
}


int CGetFirmPack::Commit(CStr2Map & inMap, CStr2Map &outMap)
{
    CStr2Map qryInMap,pvInMap,pvOutMap;
    CStr2Map InsertInMap,InsertOutMap;//插入固件信息表用
    CStr2Map upInMap,upOutMap;//更新固件规则表用
    CStr2Map preSVNInMap,preSVNOutMap;//更新预拉取固件表用

    //取出SVN密码
    char svn_name[256]={0};
    char svn_passwd[256]={0};
    memcpy(svn_name,g_mTransactions[GetTid()].m_mVars["svnname"].c_str(),g_mTransactions[GetTid()].m_mVars["svnname"].length());
    memcpy(svn_passwd,g_mTransactions[GetTid()].m_mVars["svnpasswd"].c_str(),g_mTransactions[GetTid()].m_mVars["svnpasswd"].length());
    
    svnUser = svn_name;
    svnPassWd = svn_passwd;

    InfoLog("1-PV-申请PV");
    pvInMap["sem_id"] = "1"; //拉取SVN包
    pvInMap["sem_op"] = "P";
    CIdentRelayApi::CommSemPV(pvInMap,pvOutMap,FALSE);
    if(pvOutMap["pv_result"]!="1")
    {
        ErrorLog("获取发起批量信号量失败 id=[1] state=[%s]",pvOutMap["sem_state"].c_str());
        return -1;
    }

    InfoLog("获取固件规则表数据pack_id=[%s]",inMap["pack_id"].c_str());
    vector<CStr2Map> vectmapArray;
    vectmapArray.clear();
    
    qryInMap["offset"] = "0";
    qryInMap["limit"]  = "10"; //一次查询10笔
    qryInMap["pack_id"] = inMap["pack_id"];  //包ID
	
	//从固件预拉取表查询数据并保存在map容器
	CIdentRelayApi::QueryFirmPreList(qryInMap,outMap,vectmapArray,false);
	InfoLog("获取固件预拉取表数据 vect-size=[%d]",vectmapArray.size());
	if(atoi(outMap["ret_num"].c_str()) ==0)
	{
		InfoLog("无需处理-或处理完-退出");
		InfoLog("1-PV-释放PV");
    	pvInMap["sem_id"] = "1";
    	pvInMap["sem_op"] = "V";
    	CIdentRelayApi::CommSemPV(pvInMap,pvOutMap,FALSE);
    	if(pvOutMap["pv_result"]!="1")
   	 	{
         	ErrorLog("释放发起批量信号量失败 id=[1] state=[%s]", pvOutMap["sem_state"].c_str());
   	 	}
        return 0;
	}

    //拉取固件包之前，先把固件信息表的这个pack_id所有记录删除掉---不然可以重复插入pack_id包
    CIdentRelayApi::DeleteFirmInfo(inMap,outMap,false);

    //从配置获取基本路径
    char localSavePath[256]={0};
    char urlPath[256]={0};
    char sharePathBase[256]={0};
    memcpy(localSavePath,g_mTransactions[GetTid()].m_mVars["localbasepath"].c_str(),g_mTransactions[GetTid()].m_mVars["localbasepath"].length());
    memcpy(urlPath,g_mTransactions[GetTid()].m_mVars["urlpath"].c_str(),g_mTransactions[GetTid()].m_mVars["urlpath"].length());
    memcpy(sharePathBase,g_mTransactions[GetTid()].m_mVars["sharepath"].c_str(),g_mTransactions[GetTid()].m_mVars["sharepath"].length());
    
    string strSavePath = localSavePath;
    strSavePath += "/";
    strSavePath += Datetime::now().format("YMD");//创建临时目录
    Directory::mkdir(strSavePath);
    
    upInMap["down_count"] ="0";
    int nTotal = 0;//下载包总数
    upInMap["state"]="1";
    upInMap["pack_id"]=inMap["pack_id"];
    long nPackAllSize = 0;//包总大小
    int deal_num = 0; //处理笔数
    int fail_num = 0;//失败个数
    for(size_t i = 0;i < vectmapArray.size();i++) 
    {
        deal_num++;
        preSVNInMap.clear();
	    CIdentPub::DelMapF(vectmapArray[i],preSVNInMap);
	    InfoLog("数据库获取到的SVN路径=[%s]",preSVNInMap["svn_path"].c_str());
		InfoLog("第[%d]笔 id=[%s]",deal_num,preSVNInMap["id"].c_str());
        
        string fileName,fileSizeStr,strUrlPath;

        //拼出解压到共享目录上的路径,按照 /机型/YYYYMMDD/
        string target_path = sharePathBase;
        char termname[64]={0};
        memcpy(termname,preSVNInMap["term_type"].c_str(),preSVNInMap["term_type"].length());
        string SpliDown = "/";
        SpliDown += ads_trimall(termname);
        SpliDown += "/";
        SpliDown += Datetime::now().format("YMD");

        target_path += SpliDown; 
        //创建目录
        Directory::mkdir(target_path);

        strUrlPath = urlPath ;
        strUrlPath += SpliDown;

        //SVN地址转码 ---手动执行不用转gbk
        char buff[1024] = {0} ;
        int  iDestLen = sizeof(buff)-1;
        //Tools::ConvertCharSet((char*)preSVNInMap["svn_path"].c_str(),buff,iDestLen,"utf-8","GBK");
        
		InsertInMap["pack_id"]=inMap["pack_id"];
        vector<FileInfo> fileListInfo;
        bool bSVNpullRet = dealSVNPackGet(preSVNInMap["svn_path"],strSavePath,target_path,fileListInfo);
        size_t nSigZipCount = fileListInfo.size();

        InfoLog("第[%d]个SVN地址拉取成功,解压出[%d]个zip包",deal_num,nSigZipCount);
        if(bSVNpullRet)
        {
            for (size_t i = 0; i < nSigZipCount; ++i)
            {
                InfoLog("第[%d]个SVN的第[%d]个zip包:",deal_num,i+1);
                double fileSizeMB = static_cast<double>(fileListInfo[i].size64()) / (1024 * 1024); // 转换为 MB
                InsertInMap["svn_path"] = preSVNInMap["svn_path"];
                InsertInMap["pack_name"]=fileListInfo[i].name();
		        InsertInMap["pack_size"]=to_string(fileSizeMB);
                InsertInMap["pack_path"]=strUrlPath+"/"+InsertInMap["pack_name"];

                InfoLog("包名称:%s",InsertInMap["pack_name"].c_str());
                InfoLog("包大小:%s MB",InsertInMap["pack_size"].c_str());
                InfoLog("包下载地址:%s ",InsertInMap["pack_path"].c_str());

                //去入库固件信息表
                if(!CIdentRelayApi::InsertFirmInfo(InsertInMap,InsertOutMap,FALSE))//会返回固件信息的ID号
                {
                    ErrorLog("插入固件信息表失败 pack_id[%s]",InsertInMap["pack_id"].c_str());

                    //去更新固件规则表
                    upInMap["state"]="2";//失败
                    upInMap["pack_id"]=inMap["pack_id"];
	                if(!CIdentRelayApi::UpdateFirmDownloadPath(upInMap,upOutMap,FALSE))
	                {
		                ErrorLog("更新固件规则表失败 [%s]",upInMap["pack_id"].c_str());
	                }
                    InfoLog("1-PV-释放PV");
                    pvInMap["sem_id"] = "1";
                    pvInMap["sem_op"] = "V";
                    CIdentRelayApi::CommSemPV(pvInMap,pvOutMap,FALSE);
                    Directory::rmdir(strSavePath);
                    return 0;
                }
                nPackAllSize += fileSizeMB;

                upInMap["all_id"] += InsertOutMap["id"];//记录ID，给固件规则表
                upInMap["all_id"] += ",";
            }
            //都成功才计数
            nTotal += nSigZipCount;
            
        }
        else//SVN拉取失败
        {
            fail_num++;
            upInMap["state"]="2";     //固件规则表填失效，继续拉取其它包
        }
        //需要去更新固件预拉取表记录
        if(nSigZipCount == 0)//固件预拉取表，表示SVN解压后没有zip包
            preSVNInMap["state"]="3"; 
        else
            preSVNInMap["state"]="1"; //1:已处理(拉取)

        //去掉不必要的字段，再去更新固件预先拉取表
        preSVNInMap["svn_path"].clear(); 
        CIdentRelayApi::UpdatePreFirmDownload(preSVNInMap,preSVNOutMap,FALSE);
    }

    InfoLog("本次总共处理[%d]个包，有异常的包[%d]个,最终状态[%s]",nTotal,fail_num,upInMap["state"]=="1"?"成功":"失败");//upInMap["state"]只有成功跟失败

    upInMap["pack_size"]=to_string(nPackAllSize);
    //去掉最后一个字符
    if (!upInMap["all_id"].empty()) 
    upInMap["all_id"].pop_back(); // 移除最后一个字符

    if(nTotal == 0)//没有.zip包
    {
        upInMap["state"]="2";     //固件规则表填失效
    }

    upInMap["down_count"] = to_string(nTotal);
        
	if(!CIdentRelayApi::UpdateFirmDownloadPath(upInMap,upOutMap,FALSE))
	{
		ErrorLog("更新固件规则表失败 [%s]",upInMap["pack_id"].c_str());
	}

    //去更新固件预拉取表

    InfoLog("1-PV-释放PV");
    pvInMap["sem_id"] = "1";
    pvInMap["sem_op"] = "V";
    CIdentRelayApi::CommSemPV(pvInMap,pvOutMap,FALSE);
    if(pvOutMap["pv_result"]!="1")
    {
         ErrorLog("释放发起批量信号量失败 id=[1] state=[%s]", pvOutMap["sem_state"].c_str());
    }
    //清空本地路径的预拉取的固件包的临时目录
    Directory::rmdir(strSavePath);
    return 0;
}

bool CGetFirmPack::dealSVNPackGet(const string& svn_path, string& target_path, const string& username, const string& password, string& strfilename, string& fileSizeStr)
{
    // 构建 SVN 命令
    std::string command = "svn export --force --username " + username + " --password " + password + " \"" + svn_path + "\" \"" + target_path + "\"";

   
    // 执行命令
    int result = std::system(command.c_str());

    // 检查命令执行结果
    if (result == 0) 
    {
        InfoLog("成功从 SVN 拉取固件=[%s]", svn_path.c_str());

        // 获取文件名
        std::size_t lastSlash = svn_path.find_last_of("/\\");
        if (lastSlash != std::string::npos) 
        {
            strfilename = svn_path.substr(lastSlash + 1);
        } 
        else 
        {
            strfilename = svn_path; // 如果没有路径分隔符，直接使用整个字符串
        }

        // 更新目标路径为完整文件路径
        target_path += "/" + strfilename;

        // 计算文件大小
        std::ifstream file(target_path, std::ios::binary);
        if (file) 
        {
            // 移动到文件末尾以获取文件大小
            file.seekg(0, std::ios::end);
            std::streamsize fileSize = file.tellg();
            file.close();

            double fileSizeMB = static_cast<double>(fileSize) / (1024 * 1024); // 转换为 MB
            fileSizeStr = std::to_string(fileSizeMB); // 转换为字符串
        } 
        else 
        {
            ErrorLog("文件不存在: [%s]", target_path.c_str());
            return false;
        }

        return true;
    } 
    else 
    {
        ErrorLog("从 SVN 拉取固件失败=[%s]", svn_path.c_str());
        return false;
    }
}

bool CGetFirmPack::dealSVNPackGet(const string& svn_path, const string& localPath, string& target_path, vector<FileInfo>& zipfiles)
{
    // 构建 SVN 命令
    std::string command = "svn export --force --username " + svnUser + " --password " + svnPassWd + " \"" + svn_path + "\" \"" + localPath + "\"";

    InfoLog("command=[%s]", command.c_str());
    // 执行命令
    int result = std::system(command.c_str());

    // 检查命令执行结果
    if (result == 0) 
    {
        InfoLog("成功从 SVN 拉取固件=[%s]", svn_path.c_str());

        // 获取文件名
        std::size_t lastSlash = svn_path.find_last_of("/\\");
        std::string strfilename = (lastSlash != std::string::npos) ? svn_path.substr(lastSlash + 1) : svn_path;

        // 更新目标路径为完整文件路径
        string strLocalFull = localPath +"/" + strfilename;

        // 检查文件扩展名
        if (strfilename.compare(strfilename.length() - 4, 4, ".zip") == 0) 
        {
            // 使用 unzip 命令解压 .zip 文件
            string unzipCommand = "unzip -o \"" + strLocalFull + "\" -d \"" + target_path + "\"";
            int unzipResult = std::system(unzipCommand.c_str());

            if (unzipResult == 0) 
            {
                InfoLog("成功解压缩 .zip 文件到目录: [%s]", target_path.c_str());

                vector<FileInfo> files;
                Directory::getAllSubFiles(target_path,files);
                InfoLog("[%s]目录文件数量[%d]", target_path.c_str(),files.size());

                // 遍历解压后的文件
                for (size_t i = 0; i < files.size(); ++i)
                {
                    if(files[i].suffix() == "zip")//只处理后缀为.zip的下载包
                    {
                        zipfiles.emplace_back(files[i]); // 直接添加 FileInfo 对象
                    }
                }
            } else 
            {
                InfoLog("解压缩 .zip 文件失败: [%s]", target_path.c_str());
                return false;
            }
        } else 
        {
            // 计算文件大小
            FileInfo mfileInfo(strfilename);
            if(mfileInfo.suffix() == ".zip")//只处理后缀为.zip的下载包
            {
                zipfiles.emplace_back(mfileInfo); // 直接添加 FileInfo 对象
            }
            else 
            {
                ErrorLog("文件不存在: [%s]", target_path.c_str());
                return false;
            }
        }

        return true;
    } else 
    {
        ErrorLog("从 SVN 拉取固件失败=[%s]", svn_path.c_str());
        return false;
    }
}