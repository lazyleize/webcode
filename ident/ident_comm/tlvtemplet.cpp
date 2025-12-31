/*
 *Description:
 *  TLV 处理类
 *Author:
 *   leize
 *Copyright:
 *   Copyright(C) 2020
*/
#include "tlvtemplet.h"
#include "CIdentPub.h"

CTLVItem::CTLVItem(const char * cTag, unsigned iLen, unsigned char * cValue)
{
    memset(m_cTag, 0, sizeof(m_cTag));
    m_iLen = 0;
    m_cValue = NULL;

    int iTagLen = strlen(cTag);
    if(iTagLen > 0 && iTagLen <= TLV_STRTAG_LEN &&
       iLen >= 0 && iLen <= TLV_STRUCT_LEN)
    {
        if((m_cValue = (unsigned char *)malloc(iLen+1)) != NULL)
        {
			memcpy(m_cTag,cTag,iTagLen);
            m_iLen = iLen;
            memcpy(m_cValue, cValue, iLen);

        }
    }
}

CTLVItem::~CTLVItem()
{
    if(m_cValue)
    {
        free(m_cValue);
        m_cValue = NULL;
    }
}

const char * CTLVItem::GetTag()const
{
    return m_cTag;
}

int CTLVItem::GetLength()const
{
    return m_iLen;
}

int CTLVItem::GetValue(unsigned char *cValue, unsigned iMaxLen)const
{
    if(m_iLen > 0 && m_iLen <= iMaxLen && m_cValue)
    {
        memcpy(cValue, m_cValue, m_iLen);
        return m_iLen;
    }
    return 0;
}

/*********************************************************************************/
CTLVTemplet::CTLVTemplet()
{
    //ctor
    m_tlvItemList.clear();
}

CTLVTemplet::~CTLVTemplet()
{
    //dtor
    ClearItem();
}

CTLVTemplet::CTLVTemplet(const CTLVTemplet & tlvObj)
{
    unsigned char cValue[TLV_STRUCT_LEN];

    m_tlvItemList.clear();

    std::list<CTLVItem *>::const_iterator it;
    for(it = tlvObj.m_tlvItemList.begin(); it != tlvObj.m_tlvItemList.end(); it++)
    {
        if(0 < ((*it)->GetValue(cValue, sizeof(cValue))))
        {
            CTLVItem *pItem = new CTLVItem((*it)->GetTag(), (*it)->GetLength(), cValue);
            m_tlvItemList.push_back(pItem);
        }
    }
}

CTLVTemplet & CTLVTemplet::operator = (const CTLVTemplet & tlvObj)
{
    if(this != &tlvObj)
    {
        unsigned char cValue[TLV_STRUCT_LEN];

        ClearItem();

        std::list<CTLVItem *>::const_iterator it;
        for(it = tlvObj.m_tlvItemList.begin(); it != tlvObj.m_tlvItemList.end(); it++)
        {
            if(0 < ((*it)->GetValue(cValue, sizeof(cValue))))
            {
                CTLVItem *pItem = new CTLVItem((*it)->GetTag(), (*it)->GetLength(), cValue);
                m_tlvItemList.push_back(pItem);
            }
        }
    }
    return *this;
}

/*******************************************************************
函数名称: PackTLVData
函数功能: 打包TLV数据
参    数: cData[out]:打包后的数据
          iLen[in/out]:in cData的有效容量，out 打包后数据的长度
返 回 值: 1 成功， 0 失败
相关调用: 在调用AddTLVItemByStr或AddTLVItemByHex后调用
备    注:
********************************************************************/
int CTLVTemplet::PackTLVData(unsigned char * cData, unsigned & iLen)
{
    std::list<CTLVItem *>::const_iterator it;

    unsigned char tlvData[TLV_STRUCT_LEN+5], cPackData[TLV_MESSAGE_LEN];
    int iPos = 0, iItemLen = 0, iTmpLen = 0;

    for(it = m_tlvItemList.begin(); it != m_tlvItemList.end(); it++)
    {
        CTLVItem *pItem = *it;

        memset(tlvData, 0, sizeof(tlvData));

        iItemLen = 0;
        iTmpLen = strlen(pItem->GetTag());
        

		string TagStr = CIdentPub::HEX_TO_ASCII((char*)pItem->GetTag(),iTmpLen);
		memcpy(tlvData,TagStr.c_str(),iTmpLen/2);
        iItemLen += iTmpLen/2;

        iTmpLen = pItem->GetLength();

		tlvData[iItemLen++] = (unsigned char)(iTmpLen & 0x000000FF);
		tlvData[iItemLen++] = (unsigned char)((iTmpLen >> 8)&0x000000FF);
		tlvData[iItemLen++] = (unsigned char)((iTmpLen >> 16)&0x000000FF);
		tlvData[iItemLen++] = (unsigned char)((iTmpLen >> 24)&0x000000FF);
		
        if(iTmpLen == pItem->GetValue(tlvData+iItemLen, sizeof(tlvData)-iItemLen))
        {
            iItemLen += iTmpLen;
        }
        else
        {
            ErrorLog("TLV长度获取错误");
			return -1;
        }

        if(iPos + iItemLen < (int)sizeof(cPackData))
        {
            memcpy(cPackData+iPos, tlvData, iItemLen);	//拷贝一条TLV数据
            iPos += iItemLen;
        }
        else
        {
			ErrorLog("TLV组包失败");
            return -3;
        }
    }

    if(iPos <= iLen)
    {
        memcpy(cData, cPackData, iPos);
        iLen = iPos;
        return 1;
    }
    return 0;
}

/*******************************************************************
函数名称: UnPackTLVData
函数功能: 解析TLV数据
参    数: iLen[in]:TLV数据长度
          cData[in]:要解析的数据
          bClear[in]:解析前是否清除原有数据，默认清除
返 回 值: 1 成功， 0 失败
相关调用:
备    注:
********************************************************************/
int CTLVTemplet::UnPackTLVData(unsigned iLen, unsigned char * cData, bool bClear)
{
    if(iLen > TLV_MESSAGE_LEN)return 0;

    if(bClear)ClearItem();

    unsigned char cTag[TLV_STRTAG_LEN+1], cValue[TLV_STRUCT_LEN];
    unsigned int iPos = 0, iTagLen = 0, iLengthLen = 0, iValueLen = 0, iLength = 0;

    iLengthLen = 4;
	iTagLen = 2;

    while(iLen > iPos)
    {
        memset(cTag, 0, sizeof(cTag));
		CIdentPub::Hex2Str(&cData[iPos], (char *)cTag, iTagLen);


		//4字节长度
		iValueLen = cData[iPos+iTagLen] + ((unsigned int)cData[iPos+iTagLen+1] << 8)
				+ ((unsigned int)cData[iPos+iTagLen+1+1] << 16) + ((unsigned int)cData[iPos+iTagLen+1+1+1] << 24);
        
		if( iValueLen == 0 )
		{
			memcpy(cValue,"0", 1);
		}
        iLength = iTagLen + iLengthLen + iValueLen;

        if(iLength < TLV_STRUCT_LEN)
        {
            memcpy(cValue, &cData[iPos+iTagLen+iLengthLen], iValueLen);
        }
        else
        {
			ErrorLog("值超过最大长度 %d",iLength);
            return -1;
        }

		if(iValueLen == 0)
			iValueLen = 1;
        CTLVItem *pItem = new CTLVItem((const char *)cTag, iValueLen, cValue);
        if(iValueLen == (unsigned)pItem->GetLength())
        {
            m_tlvItemList.push_back(pItem);
            iPos += iLength;
        }
        else
        {
			ErrorLog("标签或者值长度域错误");
            return -1;
        }
    }
    return 1;
}

/*******************************************************************
函数名称: AddTLVItemByStr
函数功能: 以字符串格式设置数据
参    数: cTag[in]:数据标签
          iLen[in]:字符串cData的长度
          cValue[in]:字符串数据
返 回 值: 1 成功， 0 失败
相关调用:
备    注:
********************************************************************/
int CTLVTemplet::AddTLVItemByStr(const char * pTag, unsigned iLen, unsigned char * cValue)
{
    if(iLen%2 == 1)return 0;

    unsigned char cHex[iLen/2 + 1];

    memset(cHex, 0, sizeof(cHex));

    //Ccommon::Asc2Bcd((char *)cValue, cHex, iLen);

    return AddTLVItemByHex((unsigned char*)pTag, iLen/2, cHex);
}
/*******************************************************************
函数名称: AddTLVItemByHex
函数功能: 以十六进制格式设置数据
参    数: cTag[in]:数据标签
          iLen[in]:字符串cData的长度
          cValue[in]:字符串数据
返 回 值: 1 成功， 0 失败
相关调用:
备    注:
********************************************************************/
int CTLVTemplet::AddTLVItemByHex(const unsigned char *pTag, unsigned iLen, unsigned char * cValue)
{
    char cTag[TLV_STRTAG_LEN + 1];

	memset(cTag,0x00,sizeof(cTag));
	CIdentPub::Hex2Str(pTag,cTag,2);
   // memcpy(cTag,pTag,sizeof(cTag));
    //DelTLVItemByTag(cTag);

    int iTagLen = strlen(cTag);
    if(iTagLen > 0 && iTagLen <= TLV_STRTAG_LEN && iLen >= 0 && iLen <= TLV_STRUCT_LEN)
    {
        m_tlvItemList.push_back(new CTLVItem(cTag, iLen, cValue));
        return 1;
    }
    return 0;
}
/*******************************************************************
函数名称: AddTLVItemByStr
函数功能: 以枚举值设置数据
参    数: eTag[in]:数据标签
          iLen[in]:字符串cData的长度
          cValue[in]:字符串数据
返 回 值: 1 成功， 0 失败
相关调用:
备    注: 以重载的方式添加
********************************************************************/
int CTLVTemplet::AddTLVItemByStr(const CEATE_TLV_HEAD_TAG& eTag, unsigned iLen, unsigned char * cValue)
{
    if(iLen%2 == 1 ||(int)eTag > 65535)
        return 0;

    unsigned char cHex[iLen/2 + 1];
    memset(cHex, 0, sizeof(cHex));

    //Ccommon::Asc2Bcd((char *)cValue, cHex, iLen);

    char cTag[TLV_STRTAG_LEN+1] = {0};
    unsigned int nTag = (int)eTag;

    if(nTag > 0 && nTag <= 0xFF)
        cTag[0] = nTag;
    else if(nTag > 0xFF && nTag < 0x0FFF)
    {
        cTag[0] = nTag%256;
        cTag[1] = (nTag/256 << 4) | 0x0F;
    }

    return AddTLVItemByHex((unsigned char*)cTag, iLen/2, cHex);
}

/*******************************************************************
函数名称: AddTLVItemByEnum
函数功能: 以枚举值设置数据
参    数: eTag[in]:数据标签
          iLen[in]:数据串cValue的长度
          cValue[in]:数据串,为十六进制
返 回 值: 1 成功， 0 失败
相关调用:
备    注:
********************************************************************/
int CTLVTemplet::AddTLVItemByEnum(const CEATE_TLV_HEAD_TAG& eTag, unsigned iLen, unsigned char * cValue)
{
    int nTag = (int)eTag;
	unsigned char hTag[TLV_STRTAG_LEN+1]={0};

	IntToByte(hTag,nTag);
    return AddTLVItemByHex(hTag, iLen, cValue);
}
/*******************************************************************
函数名称: GetTLVItemByStr
函数功能: 以字符串格式取数据
参    数: cTag[in]:数据标签
          cValue[in/out]:获取的字符串数据
          iMaxLen[in]:cValue的最大容量
返 回 值: 返回字符串的长度
相关调用:
备    注:
********************************************************************/
int CTLVTemplet::GetTLVItemByStr(const unsigned char * pTag, unsigned char * cValue, unsigned iMaxLen)
{
    unsigned char cHexValue[iMaxLen];


    int iHexLen = GetTLVItemByHex(pTag, cHexValue, (iMaxLen-1)/2);

    if(iHexLen > 0)
    {
        //Ccommon::Hex2Str(cHexValue, (char *)cValue, iHexLen);
        cValue[iHexLen*2] = '\0';
    }
    return iHexLen*2;
}
/*******************************************************************
函数名称: GetTLVItemByHex
函数功能: 以十六进制格式取数据
参    数: cTag[in]:数据标签
          cValue[in/out]:获取的字符串数据
          iMaxLen[in]:cValue的最大容量
返 回 值: 返回十六进制数据字节数
相关调用:
备    注:
********************************************************************/
int CTLVTemplet::GetTLVItemByHex(const unsigned char * pTag, unsigned char * cValue, unsigned iMaxLen)
{
    std::list<CTLVItem *>::const_iterator it;
    for(it = m_tlvItemList.begin(); it != m_tlvItemList.end(); it++)
    {
        CTLVItem *pItem = *it;
        if(strcmp(pItem->GetTag(), (const char *)pTag) == 0)
        {
            return pItem->GetValue(cValue, iMaxLen);
        }
    }
    return 0;
}
/*******************************************************************
函数名称: GetTLVItemByHex
函数功能: 以枚举的值取数据
参    数: eTag[in]:数据标签
          cValue[in/out]:获取的字符串数据
          iMaxLen[in]:cValue的最大容量
返 回 值:  >0:返回十六进制数据字节数 0:失败
相关调用:
备    注:以重载的方式添加
********************************************************************/
int CTLVTemplet::GetTLVItemByHex(const TLV_HEAD_TAG& eTag, unsigned char * cValue, unsigned iMaxLen)
{
    if((int)eTag >  4095)
        return 0;

    unsigned char cTag[TLV_STRTAG_LEN+1] = {0};
    unsigned int nTag = (int)eTag;

	sprintf((char*)cTag,"0%d",nTag);


    int iHexLen = GetTLVItemByHex(cTag, cValue, iMaxLen);
    if(iHexLen > 0)
    {
        cValue[iHexLen] = '\0';
    }
    return iHexLen;
}

int CTLVTemplet::DelTLVItemByTag(const char * cTag)
{
    //unsigned char cTmpData[TLV_STRUCT_LEN] = {0};
    //unsigned iTmpLen = TLV_STRUCT_LEN;

    //CTLVItem *pItem = NULL;
    std::list<CTLVItem *>::const_iterator it;
    for(it = m_tlvItemList.begin(); it != m_tlvItemList.end(); it++)
    {
        CTLVItem *pItem = *it;
        if(strcmp((*it)->GetTag(), cTag) == 0)
        {
            m_tlvItemList.remove(*it);
                delete pItem;
            break;
        }
    }
    return 0;
}

/*******************************************************************
函数名称: ClearItem
函数功能: 清除节点数据并释放内存
参    数:
返 回 值:
相关调用:
备    注:
********************************************************************/
void CTLVTemplet::ClearItem()
{
    while(!m_tlvItemList.empty())
    {
        CTLVItem * pItem = m_tlvItemList.front();
        if(pItem)
        {
            delete pItem;
            pItem = NULL;
        }
        m_tlvItemList.pop_front();
    }
}


int CTLVTemplet::IntToByte(unsigned char *data, int value)
{
	*data++ = (unsigned char)(value & 0xFF);
	*data++ = (unsigned char)(value >> 8);
	*data++ = (unsigned char)(value >> 16);
	*data++ = (unsigned char)(value >> 24);
	return 0;
}

