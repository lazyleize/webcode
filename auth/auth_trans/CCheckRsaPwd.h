#ifndef _CCHECKRSAPWD_H_
#define _CCHECKRSAPWD_H_
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <stdio.h>
#include "CAuthComm.h"

class CCheckRsaPwd: public CAuthComm
{
public:
	CCheckRsaPwd(CReqData *pReqData, CResData *pResData) :
		CAuthComm(pReqData, pResData)
	{
	}
	virtual ~CCheckRsaPwd()
	{
	}
	virtual int AuthCommit(CReqData *pReqData, CResData *pResData);
	string GetTid(){
		return string("check_rsa_pwd");;
	};
	// 生成RSA公钥和私钥文件
        bool generate_key();
	// 生成RSA对象
        RSA * createRSA(unsigned char * key,int isPublic);
        // 生成RSA对象
        RSA * createRSAWithFilename(char * filename,int isPublic);
	// 获取RSA公钥和私钥
        string getPublicKey();
        string getPrivateKey();	
        // 使用公钥加密数据
        int public_encrypt(unsigned char * data,int data_len,unsigned char * key, unsigned char *encrypted);
        // 使用私钥解密数据
        int private_decrypt(unsigned char * enc_data,int data_len,unsigned char * key, unsigned char *decrypted);
        // 使用私钥加密数据
        int private_encrypt(unsigned char * data,int data_len,unsigned char * key, unsigned char *encrypted);
        // 使用公钥解密数据
        int public_decrypt(unsigned char * enc_data,int data_len,unsigned char * key, unsigned char *decrypted);
	//字节流转换为十六进制字符串  
	void ByteToHexStr(const unsigned char* source, char* dest, int sourceLen);
	//十六进制字符串转换为字节流  
	void HexStrToByte(const char* source, unsigned char* dest, int sourceLen);  
};

CTrans* CCgi::MakeTransObj()
{
	CCheckRsaPwd* pTrans = new CCheckRsaPwd(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif
