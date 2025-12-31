/************************************************************
 Desc:     财大户 获取RSA公共密钥
 Auth:     Jerry
 Modify:
 data:     2015-06-19
 ***********************************************************/
#ifndef _CGETPUBLICKEY_H_
#define _CGETPUBLICKEY_H_ 

#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <stdio.h>
#include "CAuthComm.h"

class CGetPublicKey : public CAuthComm
{
public:
	CGetPublicKey(CReqData* pReqData,CResData* pResData)
		: CAuthComm(pReqData,pResData)
	{
	}
	~CGetPublicKey(){};
	virtual int AuthCommit(CReqData* pReqData,CResData* pResData);
	string GetTid()
	{
		return string("get_public_key");
	}
	
	// 生成RSA公钥和私钥文件
	bool generate_key();
	// 获取RSA公钥和私钥
	string getPublicKey();
	string getPrivateKey();
	// 生成RSA对象
	RSA * createRSA(unsigned char * key,int isPublic);
	// 生成RSA对象
	RSA * createRSAWithFilename(char * filename,int isPublic);	
	// 使用公钥加密数据
	int public_encrypt(unsigned char * data,int data_len,unsigned char * key, unsigned char *encrypted);
 	// 使用私钥解密数据
	int private_decrypt(unsigned char * enc_data,int data_len,unsigned char * key, unsigned char *decrypted); 
	// 使用私钥加密数据
	int private_encrypt(unsigned char * data,int data_len,unsigned char * key, unsigned char *encrypted); 
 	// 使用公钥解密数据
	int public_decrypt(unsigned char * enc_data,int data_len,unsigned char * key, unsigned char *decrypted); 
	
};

CTrans* CCgi::MakeTransObj()
{
	CGetPublicKey* pTrans = new CGetPublicKey(&m_reqData,&m_resData);
	m_resData.SetOutPutType(OUTPUTJSON);
	m_resData.SetTid(pTrans->GetTid());
	m_resData.SetSubModule("auth");
	return pTrans;
}

#endif

