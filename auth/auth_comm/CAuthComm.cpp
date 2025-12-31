#include "CAuthComm.h"
#include "uuid/lnxuuid.h"
#include "tools/base64.h"
#include "tlib/tlib_all.h"

/*
 * CGI入口
 */
int CAuthComm::Commit()
{
	//提交业务逻辑
	int iRet = AuthCommit(m_pReqData, m_pResData);
	return iRet;
}

void CAuthComm::saveUploadFile(const cgicc::FormFile& formFile,const string& fileName)
{
	ofstream fout;
	/*
	 * 创建多级目录，并以二进制方式打开写文件
	 */
	if(CAuthPub::OpenFile(fileName ,fout, ios::binary))
	{
		ErrorLog((string("创建文件目录失败:")+fileName+string(strerror(errno))).c_str());
		throw(CTrsExp(ERR_MKDIR_UPLOAD ,"创建文件目录失败，请通知管理员"));
	}
	ErrorLog("[%s:%d],创建文件目录:%s",__FILE__,__LINE__,fileName.c_str());
	formFile.writeToStream(fout);
	fout.close();
}

string CAuthComm::saveUploadFile(const cgicc::FormFile& formFile)
{
	/* 全目录路径 */
	char fileFullName[1024]={0};
	//文件名
	char szFileName[1024]={0};

	/* 取文件名后缀 */
	string::size_type iPos =
		formFile.getFilename().find_last_of('.');

	/*
	 *给每个文件一个唯一标示号,以便存入数据库中
	 *并能从前端根据此标示号，下载文件
	 */
	CUuid uid(1);
	//获取文件名
	snprintf(szFileName, sizeof(szFileName), "%s%s",
			uid.GetString(0).c_str(),
			formFile.getFilename().substr(iPos).c_str());

	//获取公共路径
	string commPath =CAuthPub::GetTransConfigNotEmpty(
			this->GetTid(),"upload_path");

	//得到上传文件的全路径名
	snprintf(fileFullName, sizeof(fileFullName), "%s/%s",
			commPath.c_str(),szFileName);
	//保存文件到本地
	this->saveUploadFile(formFile,fileFullName);

	return  szFileName;
}

int CAuthComm::UploadFile(CReqData *pReqData, CStr2Map &fileName,const unsigned int fileCount)
{
	int uploadCount(0);

	//得到上传文件的列表 
	vector<cgicc::FormFile> vcFilelist;
	m_pReqData->GetUploadFileList(vcFilelist);

	//判断文件数量 
	if (fileCount != vcFilelist.size())
	{
		ErrorLog("上传文件数量不对 上传文件数量[%d] 要求文件数量[%d]", vcFilelist.size(), fileCount);
		throw CTrsExp(ERR_UPFILE_COUNT, "上传文件数量不对");
	}
	//遍历 
	vector<cgicc::FormFile>::const_iterator it;
	for (it = vcFilelist.begin(); it != vcFilelist.end(); ++it)
	{
		//检查文件 
		this->CheckFile((*it));

		//存放文件到本地并保存文件名，便于后续取文件 
		fileName[it->getName()] =this->saveUploadFile(*it);

		uploadCount++;
	}

	return uploadCount;
}

void CAuthComm::CheckFile(const cgicc::FormFile formFile)
{
	/*
	 *          * 检查文件大小
	 *                   */
	ErrorLog("tid[%s]", this->GetTid().c_str());
	string sUploadMaxFileSize(CAuthPub::GetTransConfigNotEmpty(this->GetTid(),"max_file_length"));
	unsigned int maxUploadFileSize = atoi(sUploadMaxFileSize.c_str());

	if (formFile.getDataLength() <= 0 || formFile.getDataLength()
			> maxUploadFileSize)
	{
		ErrorLog("|上传的文件大小存在异常! ClientIp: %s, file size:%d, limit:%d",
				this->GetClientIp().c_str(),formFile.getDataLength(),maxUploadFileSize);
		throw(CTrsExp(ERR_OVER_MAXUPLOADESIZE, "上传的文件大小存在异常!"));
	}
	/*
	 * 检查文件名后缀
	 */
	size_t iPos = formFile.getFilename().find_last_of('.');
	if (string::npos == iPos)
	{
		ErrorLog("上传的文件后缀名异常! ClientIp: %s, file size:%d, limit:%d",
				this->GetClientIp().c_str(),formFile.getDataLength(),maxUploadFileSize);
		throw(CTrsExp(ERR_UPLOAD_FILENAME, "上传的文件后缀名异常!"));
	}
	//得到文件后缀名
	string sFileFmtName(formFile.getFilename().substr(iPos + 1));
	/*
	 * 将文件后缀名变为小写，再进行匹配
	 */
	sFileFmtName = Tools::lower(sFileFmtName);

	//从配置文件读取文件后缀名的配置
	string file_type =CAuthPub::GetTransConfigNotEmpty(
			this->GetTid(), "file_type");

	//做正则表达式匹配
	string strErrmsg;
	if (0 != Tools::regex_match(sFileFmtName, file_type,strErrmsg))
	{
		ErrorLog("上传的文件后缀名异常! ClientIp: %s, file size:%d, limit:%d",
				this->GetClientIp().c_str(),formFile.getDataLength(),maxUploadFileSize);
		throw(CTrsExp(ERR_UPLOAD_FILENAME, "上传的文件后缀名异常!"));
	}
}

string CAuthComm::GetVerifyCode()
{
	char tmpbuf[8]={0};
	srand((unsigned)time( NULL ));
    sprintf(tmpbuf,"%06d",rand()%1000000);
	return tmpbuf;
    
}

void CAuthComm::CheckVerifyCode(const string & verify_code)
{
		//获取cookie中的验证码
        string cookie_code = this->m_pReqData->GetCookie(VERIFYSESSION);
		DebugLog("cookie_code:[%s]=====================================================",cookie_code.c_str());
        if(cookie_code.empty())
        {
            ErrorLog("[%s],[%d]行 验证码过期 uin:[%s]",__FILE__,__LINE__, verify_code.c_str());
            throw CTrsExp(ERR_VERIFYCODE_TIMEOUT,"验证码过期，请从新获取验证码");
        }

        //验证码解密
        char verify_decode[32];
        memset(verify_decode, 0x00, sizeof(verify_decode));
        int length;
        //base64解密
        int result = Base64_Decode(cookie_code.c_str(), cookie_code.length(),
                    (unsigned char*)verify_decode,sizeof(verify_decode),&length);
        if(result != 0)
        {
            ErrorLog("[%s],[%d]行 验证码解密失败 cookie_code:[%s]",__FILE__,__LINE__,cookie_code.c_str());
            throw CTrsExp(ERR_DECODE_BASE,"系统繁忙,请稍后再试");
        }

        //删除cookie记录
        //set_cookie(VERIFYSESSION, "", "Thu, 01 Jan 1970 00:00:00 GMT", "/", "icandai.com",  0);

        //校验验证码是否正确
        if(string(verify_decode) != verify_code)
        {
            ErrorLog("[%s],[%d]行 验证码错误 uin:[%s]",__FILE__,__LINE__,verify_code.c_str());
            throw CTrsExp(ERR_VERIFY_CODE,"验证码错误");
        }
}

void CAuthComm::DelMapF(CStr2Map& dataMap,CStr2Map& outMap)
{
        CStr2Map::const_iterator it = dataMap.begin();
        while(it != dataMap.end())
        {
                string::const_iterator s = it->first.begin();
                if(*s == 'F')
                {
                        outMap[it->first.substr(1,it->first.length() - 1)] = it->second;
                        ++it;
                        continue;
                }
                outMap[it->first] = it->second;
                ++it;
        }
}



