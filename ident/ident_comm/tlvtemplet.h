/*
 *Description:
 *  TLV 处理类
 *Author:
 *   leize
 *Copyright:
 *   Copyright(C) 2020
*/

#ifndef CTLVTEMPLET_H
#define CTLVTEMPLET_H

#include <list>
#include "CIdentPub.h"

#define TLV_STRTAG_LEN   4     //TAG标签最大长度
#define TLV_STRUCT_LEN   2048*8  //TLV单域最大长度
#define TLV_MESSAGE_LEN  2048*10  //TLV报文最大长度
/**TLV结构类**/
class CTLVItem
{
public:
	CTLVItem(const char * cTag, unsigned iLen, unsigned char * cValue);
	virtual ~CTLVItem();

	const char* GetTag()const;
	int GetLength()const;
	int GetValue(unsigned char *cValue, unsigned iMaxLen)const;

private:
	CTLVItem(){};
    CTLVItem(const CTLVItem &){};
    CTLVItem & operator = (const CTLVItem &){return *this;}

	char m_cTag[TLV_STRTAG_LEN+1];
    unsigned m_iLen;
    unsigned char * m_cValue;
};


//TLV标签宏定义
typedef enum{
    TAG_SCC          = 100,  //0x0F01 服务点条件码
	TAG_DataSrcFlag	 = 200,         //数据来源标签的值
    TAG_SHAREKEY     = 300,            //加密随机数信息
    TAG_BASEINFOCIPHER  = 400,		//接头基础信息
	TAG_SIGNREQ			= 500,			//签名信息
	TAG_DEVICECERT	    = 600			//设备证书信息
} TLV_HEAD_TAG;

//组包TLV标签宏定义
typedef enum{
     DEVICE_SRC_TAG     = 1,   //数据源为设备
	 SERVER_SRC_TAG     = 2,   //数据源为服务器
	 SESSION_KEY_TAG    = 3,
	 CIPHER_TAG         = 4,
	 SIGN_TAG           = 5,
	 DEVICE_CERT_TAG    = 6,
	 OTHER_INFO_TAG     = 7,
     ENC_DLP_TAG        = 8,
} CEATE_TLV_HEAD_TAG;

typedef struct __stdOBDBaseInfoCiper
{

	unsigned char	   OBD_SN[16];				// 接头SN
	unsigned char      SE_SN[16];	            // SE SN
	unsigned char	   vin[24];			        // 车辆VIN码
    unsigned char      dlVersion[8];            // Download程序版本信息
    unsigned char      seVersion[8];            // 安全芯片程序版本信息
    int                licenseNum;              // license请求次数
	unsigned char	   otherInfo[32];		    // 其它附带信息
}stdOBDBaseInfoCiper;

typedef struct __stdBaseReq
{
	stdOBDBaseInfoCiper         baseInfo;
	unsigned char  				plus[16];  //设备端扰动因子
}stdBaseReq;

typedef struct __stdLicenseRequestPackage
{
	unsigned short 			TdataSrcFlag;		  //数据来源标签标识			 
	int 					dataSrcFlagLen; 	  //数据来源标签长度
	unsigned char* 			vDataSrcFlag;         //数据来源标签的值 
	unsigned short          TencShareKey;         //设备密钥标识           
	int 					encShareKeyLen;		  //设备密钥长度
	unsigned char* 			pencShareKey; 		  //加密随机数信息,SM2:0x04+C1(64bytes)+C3(32bytes)+C2(Nbytes)
	unsigned short			TbaseInfoCipher;	  //接头基础信息标识	  
	int          	        baseInfoCipherLen;    //接头基础信息长度	
	stdBaseReq* 			baseInfoCipher;	      //接头基础信息
	unsigned short			TsignReq;	          //签名信息标识	  
	int 	   				signReqLen;			  //签名信息长度
	unsigned char*		   	psignReq;			  //签名信息
	unsigned short			TdeviceCert;	      //设备证书信息标识
	int		   				deviceCertLen;		  //设备证书长度	
	unsigned char*		   	pdeviceCert;		  //设备证书信息,不超过2K
}stdLicenseRequestPackage;

class CTLVTemplet
{
public:
	CTLVTemplet();
	CTLVTemplet(const CTLVTemplet&);
	virtual ~CTLVTemplet();

	CTLVTemplet& operator = (const CTLVTemplet &);//赋值操作符重载

	

	int PackTLVData(unsigned char * cData, unsigned & iLen);//打包TLV格式数据
    int UnPackTLVData(unsigned iLen, unsigned char * cData, bool bClear = true);//解析TLV格式数据

    int AddTLVItemByStr(const char *pTag, unsigned iLen, unsigned char * cValue);//以字符串格式设置数据
    int AddTLVItemByHex(const unsigned char *pTag, unsigned iLen, unsigned char * cValue);//以十六进制格式设置数据
    int AddTLVItemByStr(const CEATE_TLV_HEAD_TAG& eTag, unsigned iLen, unsigned char * cValue); //以枚举的值设置数据
    int AddTLVItemByEnum(const CEATE_TLV_HEAD_TAG& eTag, unsigned iLen, unsigned char * cValue); //以枚举的值设置数据(数据格式为十六进制)

    int GetTLVItemByStr(const unsigned char *pTag, unsigned char * cValue, unsigned iMaxLen);//以字符串格式取数据
    int GetTLVItemByHex(const unsigned char *pTag, unsigned char * cValue, unsigned iMaxLen);//以十六进制格式取数据
    int GetTLVItemByHex(const TLV_HEAD_TAG& eTag, unsigned char * cValue, unsigned iMaxLen);//以枚举的值取数据

    int DelTLVItemByTag(const char * cTag);

	int IntToByte(unsigned char *data, int value);

private:
	void ClearItem();//清除链表中的节点并释放内存
	std::list<CTLVItem *> m_tlvItemList;
};
#endif // CTLVTEMPLET_H
