#include "CCheckRsaPwd.h"
#include "CAuthRelayApi.h"
#define PUBSSLKEY "rsa_public_key.pem"
#define PRISSLKEY "rsa_private_key.pem"
int padding = RSA_PKCS1_PADDING;

bool CCheckRsaPwd::generate_key()
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

RSA* CCheckRsaPwd::createRSA(unsigned char * key,int isPublic)
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

RSA* CCheckRsaPwd::createRSAWithFilename(char * filename,int isPublic)
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

int CCheckRsaPwd::public_encrypt(unsigned char * data,int data_len,unsigned char * key, unsigned char *encrypted)
{
    RSA * rsa = createRSA(key,1);
    int result = RSA_public_encrypt(data_len,data,encrypted,rsa,padding);
    return result;
}

int CCheckRsaPwd::private_decrypt(unsigned char * enc_data,int data_len,unsigned char * key, unsigned char *decrypted)
{
    RSA * rsa = createRSA(key,0);
    int  result = RSA_private_decrypt(data_len,enc_data,decrypted,rsa,padding);
    return result;
}


int CCheckRsaPwd::private_encrypt(unsigned char * data,int data_len,unsigned char * key, unsigned char *encrypted)
{
    RSA * rsa = createRSA(key,0);
    int result = RSA_private_encrypt(data_len,data,encrypted,rsa,padding);
    return result;
}

int CCheckRsaPwd::public_decrypt(unsigned char * enc_data,int data_len,unsigned char * key, unsigned char *decrypted)
{
    RSA * rsa = createRSA(key,1);
    int  result = RSA_public_decrypt(data_len,enc_data,decrypted,rsa,padding);
    return result;
}

string CCheckRsaPwd::getPublicKey()
{
	stringstream buffer;

	buffer << "-----BEGIN PUBLIC KEY-----\n"\
	"MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDVWdmADJzZyQGXoOLXMGIpKjSQ\n"\
	"a9Uh8JggEe6M6t9EGBAAPVUovZqnyUMsJu+NK/vwli9xu2YHD/tddmwSG93ntnaa\n"\
	"fkyxCPDc+LuFvGlcyVIvPhCsxJZIjFHltTQWjeP5tkv28Zk20Ajo84NlKqIj123t\n"\
	"PtGVSvzphiulGqi0QQIDAQAB\n"\
	"-----END PUBLIC KEY-----\n";
	
          return buffer.str();
}

string CCheckRsaPwd::getPrivateKey()
{
	stringstream buffer;

	buffer << "-----BEGIN RSA PRIVATE KEY-----\n"\
	"MIICXQIBAAKBgQDVWdmADJzZyQGXoOLXMGIpKjSQa9Uh8JggEe6M6t9EGBAAPVUo\n"\
	"vZqnyUMsJu+NK/vwli9xu2YHD/tddmwSG93ntnaafkyxCPDc+LuFvGlcyVIvPhCs\n"\
	"xJZIjFHltTQWjeP5tkv28Zk20Ajo84NlKqIj123tPtGVSvzphiulGqi0QQIDAQAB\n"\
	"AoGADNRjXOzlPb91pg2ZtoEFVh5gnNKvNWRHcZ5CbxzsoiakR69ogJOuSiqzLGOH\n"\
	"AkFD3DrbzYYMY55ValBlvU0JsfpBK3HDbmhIXpkai5yZZZ+2SNie6y5o2zd8Ts4p\n"\
	"nHehM6n4aHLCj69xTMrAvpjFUaieTHZefWmZqRyQ5199F3ECQQD4Ugnxj5ztx8bH\n"\
	"Z/0gARTEdIotoDzHkADDW0QNdJk/l9cX3yGCxh2Bkx+PBG5P0MVX2U51Up3wR9HD\n"\
	"GgFTVEy1AkEA2/L0yNU7XADSBVcf9DrtEg7paASerATwyKtkG/76RQemeQErwFiw\n"\
	"VXn+ZRxVykJNraN9jNQCt1U7u+wpsmIM3QJBAM3ZdFTDcJvM2IPFOJinTMfus/1O\n"\
	"sBPe1EMeDTP6TG/jN3OajPUTtrILfEXarneL4YwJoHixnCvr3X+WtzKU12ECQHSe\n"\
	"MON7C2oQlUp3k12vEJuOhHq+WFLkm7YKCZ4+ZvvKvp1R3ZyyaWworpV1nJcM6Jqn\n"\
	"IJFWp8oEyxMseD84dZECQQCsAflGI8HFj16/rb9al93JjVdcUt562xCJjufg+k92\n"\
	"elyh/8cNcIcDqUXZOlbz+FN10U6S7BLC1lO7iJtS+dml\n"\
	"-----END RSA PRIVATE KEY-----\n";

          return buffer.str();
}

int CCheckRsaPwd::AuthCommit(CReqData *pReqData, CResData *pResData)
{
	CStr2Map inMap,outMap;
	pReqData->GetStrMap(inMap);

	char buffer[2048] = {};
	unsigned char base64_open[2048] = {};
	unsigned long len;
	unsigned char decrypted[2048]={};
	unsigned char  encrypted[2048]={};
	char login[2048];
	memset(login,0x00,sizeof(login));
	memcpy(login,inMap["login"].c_str(),strlen(inMap["login"].c_str()));

	len = ads_base64_dec(const_cast<char *>(inMap["login"].c_str()),inMap["login"].length(),buffer,sizeof(buffer));

	memcpy(base64_open,buffer,strlen(buffer));
	ErrorLog("[%s:%d]  base64_open  = [%s]",__FILE__,__LINE__,base64_open);

	string privateKey = this->getPrivateKey();
	ErrorLog("[%s:%d] PrivateKey = [%s]",__FILE__,__LINE__,privateKey.c_str());
	memcpy(encrypted,privateKey.c_str(),strlen(privateKey.c_str()));

	int decrypted_length = this->private_decrypt(base64_open,len,encrypted,decrypted);
	if(decrypted_length == -1)
	{
    		ErrorLog("[%s:%d] Private Decrypt failed ",__FILE__,__LINE__);
		throw CTrsExp(ERR_USER_PASSWORD,"密码错误");
	}

    	DebugLog("[%s:%d] Private Decrypt success: %s ",__FILE__,__LINE__,decrypted);

	return 0;
}
