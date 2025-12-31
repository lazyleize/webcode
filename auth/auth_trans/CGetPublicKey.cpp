/*****************************************************************
 Desc:     财大户 获取RSA公共密钥
 Auth:     Jerry
 Modify:
 date:     2015-06-19
 *****************************************************************/
#include "CGetPublicKey.h"
#include "CAuthRelayApi.h"
#define PUBSSLKEY "rsa_public_key.pem"
#define PRISSLKEY "rsa_private_key.pem"
int padding = RSA_PKCS1_PADDING;

bool CGetPublicKey::generate_key()
{
    int             ret = 0;
    RSA             *r = NULL;
    BIGNUM          *bne = NULL;
    BIO             *bp_public = NULL, *bp_private = NULL;

    int             bits = 2048;
    unsigned long   e = RSA_F4;

    // 1. generate rsa key
    bne = BN_new();
    ret = BN_set_word(bne,e);
    if(ret != 1){
        goto free_all;
    }

    r = RSA_new();
    ret = RSA_generate_key_ex(r, bits, bne, NULL);
    if(ret != 1){
        goto free_all;
    }

     // 2. save public key
    bp_public = BIO_new_file(PUBSSLKEY, "w+");
    ret = PEM_write_bio_RSAPublicKey(bp_public, r);
    if(ret != 1){
        goto free_all;
    }

    // 3. save private key
    bp_private = BIO_new_file(PRISSLKEY, "w+");
    ret = PEM_write_bio_RSAPrivateKey(bp_private, r, NULL, NULL, 0, NULL, NULL);

    // 4. free
free_all:

    BIO_free_all(bp_public);
    BIO_free_all(bp_private);
    RSA_free(r);
    BN_free(bne);

    return (ret == 1);
   	
}

RSA* CGetPublicKey::createRSA(unsigned char * key,int isPublic)
{
    RSA *rsa= NULL;
    BIO *keybio ;
    keybio = BIO_new_mem_buf(key, -1);
    if (keybio==NULL)
    {
        ErrorLog( "[%s:%d] Failed to create key BIO",__FILE__,__LINE__);
        return 0;
    }
    if(isPublic)
    {
        rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa,NULL, NULL);
    }
    else
    {
        rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa,NULL, NULL);
    }
    if(rsa == NULL)
    {
        ErrorLog( "[%s:%d] Failed to create RSA",__FILE__,__LINE__);
    }
 
    return rsa;
}

RSA* CGetPublicKey::createRSAWithFilename(char * filename,int isPublic)
{
    FILE * fp = fopen(filename,"rb");
 
    if(fp == NULL)
    {
        ErrorLog("[%s:%d] Unable to open file %s \n",__FILE__,__LINE__,filename);
        return NULL;    
    }
    RSA *rsa= RSA_new() ;
 
    if(isPublic)
    {
        rsa = PEM_read_RSA_PUBKEY(fp, &rsa,NULL, NULL);
    }
    else
    {
        rsa = PEM_read_RSAPrivateKey(fp, &rsa,NULL, NULL);
    }
 
    return rsa;
}

int CGetPublicKey::public_encrypt(unsigned char * data,int data_len,unsigned char * key, unsigned char *encrypted)
{
    RSA * rsa = createRSA(key,1);
    int result = RSA_public_encrypt(data_len,data,encrypted,rsa,padding);
    return result;
}

int CGetPublicKey::private_decrypt(unsigned char * enc_data,int data_len,unsigned char * key, unsigned char *decrypted)
{
    RSA * rsa = createRSA(key,0);
    int  result = RSA_private_decrypt(data_len,enc_data,decrypted,rsa,padding);
    return result;
}
 
 
int CGetPublicKey::private_encrypt(unsigned char * data,int data_len,unsigned char * key, unsigned char *encrypted)
{
    RSA * rsa = createRSA(key,0);
    int result = RSA_private_encrypt(data_len,data,encrypted,rsa,padding);
    return result;
}

int CGetPublicKey::public_decrypt(unsigned char * enc_data,int data_len,unsigned char * key, unsigned char *decrypted)
{
    RSA * rsa = createRSA(key,1);
    int  result = RSA_public_decrypt(data_len,enc_data,decrypted,rsa,padding);
    return result;
}

string CGetPublicKey::getPublicKey()
{
	/*
	ifstream inFile;
	stringstream buffer;

	inFile.open(filename);
	if (!inFile) {
		ErrorLog("[%s:%d] open public key file: %s error",__FILE__,__LINE__,filename);
		inFile.close();
	}
	// Lecture ligne a ligne
	  while (inFile&&!inFile.eof ())
	  {
		char ligne[32000];

		inFile.getline (ligne, sizeof (ligne));
		buffer << ligne;
		
	  }
	  inFile.close();
	*/

	 stringstream buffer;
        buffer << "-----BEGIN PUBLIC KEY-----\n"\
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAy8Dbv8prpJ/0kKhlGeJY\n"\
        "ozo2t60EG8L0561g13R29LvMR5hyvGZlGJpmn65+A4xHXInJYiPuKzrKUnApeLZ+\n"\
        "vw1HocOAZtWK0z3r26uA8kQYOKX9Qt/DbCdvsF9wF8gRK0ptx9M6R13NvBxvVQAp\n"\
        "fc9jB9nTzphOgM4JiEYvlV8FLhg9yZovMYd6Wwf3aoXK891VQxTr/kQYoq1Yp+68\n"\
        "i6T4nNq7NWC+UNVjQHxNQMQMzU6lWCX8zyg3yH88OAQkUXIXKfQ+NkvYQ1cxaMoV\n"\
        "PpY72+eVthKzpMeyHkBn7ciumk5qgLTEJAfWZpe4f4eFZj/Rc8Y8Jj2IS5kVPjUy\n"\
        "wQIDAQAB\n"\
        "-----END PUBLIC KEY-----\n";
	  return buffer.str();
}

string CGetPublicKey::getPrivateKey()
{
	/*
	ifstream inFile;
        stringstream buffer;

        inFile.open(filename);
        if (!inFile) {
                ErrorLog("[%s:%d] open private key file: %s error",__FILE__,__LINE__,filename);
                inFile.close();
        }       
        // Lecture ligne a ligne
          while (inFile&&!inFile.eof ())
          {             
                char ligne[32000];
                
                inFile.getline (ligne, sizeof (ligne));
                buffer << ligne;
                  
          }
          inFile.close();
	*/

	stringstream buffer;
        buffer << "-----BEGIN RSA PRIVATE KEY-----\n"\
        "MIIEowIBAAKCAQEAy8Dbv8prpJ/0kKhlGeJYozo2t60EG8L0561g13R29LvMR5hy\n"\
        "vGZlGJpmn65+A4xHXInJYiPuKzrKUnApeLZ+vw1HocOAZtWK0z3r26uA8kQYOKX9\n"\
        "Qt/DbCdvsF9wF8gRK0ptx9M6R13NvBxvVQApfc9jB9nTzphOgM4JiEYvlV8FLhg9\n"\
        "yZovMYd6Wwf3aoXK891VQxTr/kQYoq1Yp+68i6T4nNq7NWC+UNVjQHxNQMQMzU6l\n"\
        "WCX8zyg3yH88OAQkUXIXKfQ+NkvYQ1cxaMoVPpY72+eVthKzpMeyHkBn7ciumk5q\n"\
        "gLTEJAfWZpe4f4eFZj/Rc8Y8Jj2IS5kVPjUywQIDAQABAoIBADhg1u1Mv1hAAlX8\n"\
        "omz1Gn2f4AAW2aos2cM5UDCNw1SYmj+9SRIkaxjRsE/C4o9sw1oxrg1/z6kajV0e\n"\
        "N/t008FdlVKHXAIYWF93JMoVvIpMmT8jft6AN/y3NMpivgt2inmmEJZYNioFJKZG\n"\
        "X+/vKYvsVISZm2fw8NfnKvAQK55yu+GRWBZGOeS9K+LbYvOwcrjKhHz66m4bedKd\n"\
        "gVAix6NE5iwmjNXktSQlJMCjbtdNXg/xo1/G4kG2p/MO1HLcKfe1N5FgBiXj3Qjl\n"\
        "vgvjJZkh1as2KTgaPOBqZaP03738VnYg23ISyvfT/teArVGtxrmFP7939EvJFKpF\n"\
        "1wTxuDkCgYEA7t0DR37zt+dEJy+5vm7zSmN97VenwQJFWMiulkHGa0yU3lLasxxu\n"\
        "m0oUtndIjenIvSx6t3Y+agK2F3EPbb0AZ5wZ1p1IXs4vktgeQwSSBdqcM8LZFDvZ\n"\
        "uPboQnJoRdIkd62XnP5ekIEIBAfOp8v2wFpSfE7nNH2u4CpAXNSF9HsCgYEA2l8D\n"\
        "JrDE5m9Kkn+J4l+AdGfeBL1igPF3DnuPoV67BpgiaAgI4h25UJzXiDKKoa706S0D\n"\
        "4XB74zOLX11MaGPMIdhlG+SgeQfNoC5lE4ZWXNyESJH1SVgRGT9nBC2vtL6bxCVV\n"\
        "WBkTeC5D6c/QXcai6yw6OYyNNdp0uznKURe1xvMCgYBVYYcEjWqMuAvyferFGV+5\n"\
        "nWqr5gM+yJMFM2bEqupD/HHSLoeiMm2O8KIKvwSeRYzNohKTdZ7FwgZYxr8fGMoG\n"\
        "PxQ1VK9DxCvZL4tRpVaU5Rmknud9hg9DQG6xIbgIDR+f79sb8QjYWmcFGc1SyWOA\n"\
        "SkjlykZ2yt4xnqi3BfiD9QKBgGqLgRYXmXp1QoVIBRaWUi55nzHg1XbkWZqPXvz1\n"\
        "I3uMLv1jLjJlHk3euKqTPmC05HoApKwSHeA0/gOBmg404xyAYJTDcCidTg6hlF96\n"\
        "ZBja3xApZuxqM62F6dV4FQqzFX0WWhWp5n301N33r0qR6FumMKJzmVJ1TA8tmzEF\n"\
        "yINRAoGBAJqioYs8rK6eXzA8ywYLjqTLu/yQSLBn/4ta36K8DyCoLNlNxSuox+A5\n"\
        "w6z2vEfRVQDq4Hm4vBzjdi3QfYLNkTiTqLcvgWZ+eX44ogXtdTDO7c+GeMKWz4XX\n"\
        "uJSUVL5+CVjKLjZEJ6Qc2WZLl94xSwL71E41H4YciVnSCQxVc4Jw\n"\
        "-----END RSA PRIVATE KEY-----\n";
          return buffer.str();
}

int CGetPublicKey::AuthCommit(CReqData* pReqData,CResData* pResData)
{
		CStr2Map inMap;
		
		//取得所有的请求参数
        //pReqData->GetStrMap(inMap);

	/*
	//检查用户是否是合法用户
        inMap["uid"] = CAuthRelayApi::QueryUidByUin(inMap["name"]);
        if(inMap["uid"].empty())
        {
			ErrorLog("[%s %d]查询用户信息失败: uin=[%s]",
                                __FILE__, __LINE__,
                                inMap["name"].c_str());
        	throw CTrsExp(ERR_USER_NOT_REGISTER,"不存在的用户名！");
        }
	*/

	//生成公钥和私钥文件
	//this->generate_key();	
	string publicKey = this->getPublicKey();
	//string privateKey = this->getPrivateKey();

	pResData->SetPara("public_key", publicKey);
	//pResData->SetPara("private_key", privateKey);
	return 0;
}
