//
// Created by sunjun on 16-6-14.
//

#ifndef ZXFEED_L1FEED_H
#define ZXFEED_L1FEED_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <iostream>
#include <string.h>
#include <float.h>
#include <iomanip>

#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include "consts.h"
#include <queue>
#include <pthread.h>

#define GET_LAST_SOCK_ERROR() errno
typedef unsigned int DWORD;
typedef unsigned short WORD;
typedef unsigned int UNINT32;
typedef unsigned char BYTE;
#define ioctlsocket ioctl

using namespace std;

//在传送sockaddr时，使用的表示长度的类型不同 gyd 20020605 将 UNIX变为LINUX

#define SOCKADDRLEN    socklen_t
//#define SOCKADDRLEN	unsigned

struct L1RawData {
    char* data;
    int nCount;
};
queue<L1RawData*> queueDataL1;
pthread_mutex_t mutexL1;
pthread_t tidParseL1;

void* ParseMarketDataL1(void* arg);


void* runl1(void* arg) {
    ThreadARG* info = (ThreadARG*) arg;
    int port  = info->port;
    std::fstream* fs = info->fs;
    struct sockaddr_in servaddr; //IPv4套接口地址定义
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0); //建立UDP套接字
    /* set reuse and non block for this socket */
    int son = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &son, sizeof(son));

    memset(&servaddr, 0, sizeof(servaddr)); //地址结构清零
    servaddr.sin_family = AF_INET; //IPv4协议
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//内核指定地址
    servaddr.sin_port = htons(port); //端口

    //分配协议地址,绑定端口
    if (bind(sockfd, (sockaddr *) &servaddr, sizeof(servaddr)) != 0) {
        printf("bind port fail\n");
        exit(1);
    }

    for (; ;) {

        int on = 1;
        if (ioctlsocket(sockfd, FIONBIO, (char *) &on) < 0) {


            if (GET_LAST_SOCK_ERROR() == 4)
                continue;
            printf("Can not set FIONBIO\n");
        }
        else
            break;
    }

    int rcvbufsize = 1 * 1024 * 1024;
    SOCKADDRLEN paramlen = sizeof(rcvbufsize);
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (const char *) &rcvbufsize, paramlen);
    if (ret != 0) {
        printf("Can not setsockopt revbuf\n");
    }


    int on = 1;

    ret = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
    if (ret != 0) {
        printf("Can not setsockopt\n");
    }

    int number = 4000;
    char buffer[4096];

    pthread_mutex_init(&mutexL1, NULL);
    queueDataL1 = queue<L1RawData *>();
    int rett = pthread_create(&tidParseL1, NULL, ParseMarketDataL1, fs); //参数：创建的线程id，线程参数，线程运行函数的起始地址，运行函数的参数
    if (rett != 0) //创建线程成功返回0
        printf("pthread_create(ParseMarketData) error:error_code=%d\n", rett);

    while (1) {
        sockaddr_in fromAddr;
        int nFromLen = sizeof(sockaddr_in);
        int nCount = recvfrom(sockfd, buffer, 4000, 0, (sockaddr *) &fromAddr, (socklen_t *) &nFromLen);
        if (nCount == 0)
            continue;
        if (nCount == -1) {
            int nErrno = GET_LAST_SOCK_ERROR();
            if (nErrno == 0 || nErrno == 251 || nErrno == EWOULDBLOCK)    /*251 for PARisk */    //20060224 IA64 add 0
                continue;
            else {
                printf("Error in recvFrom,ErrNo[%d]\n", nErrno);
//                return -1;
            }
        }
        L1RawData* data = new L1RawData;
        data->data = buffer;
        data->nCount = nCount;
        pthread_mutex_lock(&mutexL1);
        queueDataL1.push(data);
        pthread_mutex_unlock(&mutexL1);

        //ParseMarketData(fs,buffer, nCount);
    }

//    return 0;
}




struct TFTDCHeader {
    BYTE Version;
    /**< 版本号	1	二进制无符号整数。目前版本为1*/
    BYTE Chain;
    /**< 报文链	1	ASCII码字符。*/
    WORD SequenceSeries;
    /**< 序列类别号	2	二进制无符号短整数。*/
    DWORD TransactionId;
    /**<（TID）	FTD信息正文类型	4	二进制无符号整数。*/
    DWORD SequenceNumber;
    /**<（SeqNo）	序列号	4	二进制无符号整数。*/
    WORD FieldCount;
    /**< 数据域数量	2	二进制无符号短整数。*/
    WORD FTDCContentLength;
    /**< FTDC信息正文长度	2	二进制无符号短整数。以字节为单位。*/
    DWORD RequestId;            /**< 请求编号(由发送请求者维护，应答中会带回)  4 二进制无符号整数。*/
};

struct TFieldHeader {
    WORD FieldID;
    WORD Size;
};

#pragma pack(push)
#pragma pack(1)

///行情更新时间属性
class CMarketDataUpdateTimeField {
public:
    ///合约代码
    char InstrumentID[31];
    ///最后修改时间
    char UpdateTime[9];
    ///最后修改毫秒
    int UpdateMillisec;
    ///业务日期
    char ActionDay[9];
};

///行情最优价属性
class CMarketDataBestPriceField {
public:
    ///申买价一
    double BidPrice1;
    ///申买量一
    int BidVolume1;
    ///申卖价一
    double AskPrice1;
    ///申卖量一
    int AskVolume1;
};

class CMarketDataStaticField {
public:
    ///今开盘
    double OpenPrice;
    ///最高价
    double HighestPrice;
    ///最低价
    double LowestPrice;
    ///今收盘
    double ClosePrice;
    ///涨停板价
    double UpperLimitPrice;
    ///跌停板价
    double LowerLimitPrice;
    ///今结算
    double SettlementPrice;
    ///今虚实度
    double CurrDelta;
};

class CMarketDataLastMatchField {
public:
    ///最新价
    double LastPrice;
    ///数量
    int Volume;
    ///成交金额
    double Turnover;
    ///持仓量
    double OpenInterest;
};

///行情申买二、三属性
class CMarketDataBid23Field {
public:
    ///申买价二
    double BidPrice2;
    ///申买量二
    int BidVolume2;
    ///申买价三
    double BidPrice3;
    ///申买量三
    int BidVolume3;
};


///行情申卖二、三属性
class CMarketDataAsk23Field {
public:
    ///申卖价二
    double AskPrice2;
    ///申卖量二
    int AskVolume2;
    ///申卖价三
    double AskPrice3;
    ///申卖量三
    int AskVolume3;
};

///行情申买四、五属性
class CMarketDataBid45Field {
public:
    ///申买价四
    double BidPrice4;
    ///申买量四
    int BidVolume4;
    ///申买价五
    double BidPrice5;
    ///申买量五
    int BidVolume5;
};

///行情申卖四、五属性
class CMarketDataAsk45Field {
public:
    ///申卖价四
    double AskPrice4;
    ///申卖量四
    int AskVolume4;
    ///申卖价五
    double AskPrice5;
    ///申卖量五
    int AskVolume5;
};

class CFTDMarketDataBaseField {
public:
    //交易日
    char TradingDay[9];
    //上次结算价
    double PreSettlementPrice;
    //昨收盘
    double PreClosePrice;
    //昨持仓量
    double PreOpenInterest;
    //昨虚实度
    double PreDelta;
};


#pragma pack(pop)

std::string double2str(double v) {
    if (v == DBL_MAX)
        return "NaN";
    else {
        char result[50] = {0};
        snprintf(result, 50, "%.6f", v);
        return result;
    }
}

void* ParseMarketDataL1(void* arg) {
    std::fstream* fs = (std::fstream*) arg;
    char* buffer;
    int nCount;
    L1RawData* rawData;
    while (true)
    {
        pthread_mutex_lock(&mutexL1);
        if (queueDataL1.size() == 0)
        {
            pthread_mutex_unlock(&mutexL1);
            continue;
        }
        rawData = queueDataL1.front();
        queueDataL1.pop();
        pthread_mutex_unlock(&mutexL1);

        buffer = rawData->data;
        nCount = rawData->nCount;

        if (nCount < sizeof(TFTDCHeader)) {
            printf("Invalid Package!\n");
            continue;
        }
        TFTDCHeader header;
        memcpy(&header, buffer, sizeof(header));

        ///校验一下包是否正确
        if (header.Chain != 'L' && header.Chain != 'C') {
            printf("Invalid Package!,Invalid Header[%c]\n", header.Chain);
            continue;
        }

        if (header.FTDCContentLength != (nCount - sizeof(TFTDCHeader))) {
            printf("Invalid Package Length!\n");
            printf("Content Len[%d],Expected Len[%d]\n", nCount - sizeof(TFTDCHeader));
            continue;
        }

        if (header.TransactionId != 0x0000F103) {
            printf("Not a Market Package!\n");
            continue;
        }

        char buf[4096];

        CMarketDataUpdateTimeField updateTimeField;
        memset(&updateTimeField,0,sizeof(CMarketDataUpdateTimeField));
        CMarketDataBestPriceField bestPriceField;
        memset(&bestPriceField,0,sizeof(CMarketDataBestPriceField));
        CMarketDataStaticField staticField;
        memset(&staticField,0,sizeof(CMarketDataStaticField));
        CMarketDataLastMatchField lastMatchField;
        memset(&lastMatchField,0,sizeof(CMarketDataLastMatchField));
        CFTDMarketDataBaseField databaseField;
        memset(&databaseField,0,sizeof(CFTDMarketDataBaseField));
        CMarketDataAsk23Field ask23Field;
        memset(&ask23Field,0,sizeof(CMarketDataAsk23Field));
        CMarketDataAsk45Field ask45Field;
        memset(&ask45Field,0,sizeof(CMarketDataAsk45Field));
        CMarketDataBid23Field bid23Field;
        memset(&bid23Field,0,sizeof(CMarketDataBid23Field));
        CMarketDataBid45Field bid45Field;
        memset(&bid45Field,0,sizeof(CMarketDataBid45Field));


        TFieldHeader fieldHeader;

        char local_timestamp[128] = {0};

        struct timeval t;
        gettimeofday(&t, NULL);
        struct tm *p = localtime(&t.tv_sec);
        snprintf(local_timestamp, sizeof(local_timestamp), "%04d%02d%02d,%02d%02d%02d.%06d", 1900 + p->tm_year,
                 p->tm_mon + 1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, (int32_t) (t.tv_usec));
        int local_date = (1900 + p->tm_year) * 10000 + (p->tm_mon + 1) * 100 + p->tm_mday;

        ///遍历Field
        char *pz = buffer + sizeof(TFTDCHeader);
        for (int i = 0; i < header.FieldCount; i++) {
            memcpy(&fieldHeader, pz, sizeof(TFieldHeader));
            pz += sizeof(TFieldHeader);
            switch (fieldHeader.FieldID) {
                case 0x2439: {
                    memcpy(&updateTimeField, pz, fieldHeader.Size);
                    pz += fieldHeader.Size;
                    // printf("Receive Data\n");
                    // printf("	Instrument[%s]\n",updateTimeField.InstrumentID);
                    // printf("	Time[%s]\n",updateTimeField.UpdateTime);
                    // printf("	ms[%d]\n",updateTimeField.UpdateMillisec);
                }
                    break;
                case 0x2434: {
                    memcpy(&bestPriceField, pz, fieldHeader.Size);
                    pz += fieldHeader.Size;
                    // printf("best price\n");
                    //                         printf("         BidPrice1[%f]\n",bestPriceField.BidPrice1);
                    //                         printf("        BidVolume1[%d]\n",bestPriceField.BidVolume1);
                    //                         printf("        AskPrice1[%f]\n",bestPriceField.AskPrice1);
                    //                         printf("        AskVolume1[%d]\n",bestPriceField.AskVolume1);
                }
                    break;
                case 0x2432: {
                    memcpy(&staticField, pz, fieldHeader.Size);
                    pz += fieldHeader.Size;
                    // printf("staticfield\n");
                    //                         printf("         OpenPrice[%f]\n",staticField.OpenPrice);
                    //                         printf("         HighestPrice[%f]\n",staticField.HighestPrice);
                    //                         printf("         LowestPrice[%f]\n",staticField.LowestPrice);
                    //                         printf("         ClosePrice[%f]\n",staticField.ClosePrice);
                    //                         printf("         UpperLimitPrice[%f]\n",staticField.UpperLimitPrice);
                    //                         printf("         LowerLimitPrice[%f]\n",staticField.LowerLimitPrice);
                    //                         printf("         SettlementPrice[%f]\n",staticField.SettlementPrice);
                    //                         printf("         CurrDelta[%f]\n",staticField.CurrDelta);
                }
                    break;
                case 0x2433: {
                    memcpy(&lastMatchField, pz, fieldHeader.Size);
                    pz += fieldHeader.Size;
                    // printf("lastMatchField\n");
                    //                         printf("         LastPrice[%f]\n",lastMatchField.LastPrice);
                    //                         printf("         Volume[%d]\n",lastMatchField.Volume);
                    //                         printf("         Turnover[%f]\n",lastMatchField.Turnover);
                    //                         printf("         OpenInterest[%f]\n",lastMatchField.OpenInterest);
                }
                    break;
                case 0x2435: {
                    memcpy(&bid23Field, pz, fieldHeader.Size);
                    pz += fieldHeader.Size;
                }
                    break;
                case 0x2436: {
                    memcpy(&ask23Field, pz, fieldHeader.Size);
                    pz += fieldHeader.Size;
                }
                    break;
                case 0x2437: {
                    memcpy(&bid45Field, pz, fieldHeader.Size);
                    pz += fieldHeader.Size;
                }
                    break;
                case 0x2438: {
                    memcpy(&ask45Field, pz, fieldHeader.Size);
                    pz += fieldHeader.Size;
                }
                    break;
                case 0x2431: {

                    memcpy(&databaseField, pz, fieldHeader.Size);
                    pz += fieldHeader.Size;
                    //printf("Receive Data\n");
                    //printf("        Tradingday[%s]\n",databaseField.TradingDay);
                }
                    break;
            }
        }

        (*fs) << updateTimeField.ActionDay << ","
        << databaseField.TradingDay << ","
        << updateTimeField.UpdateTime << ","
        << updateTimeField.UpdateMillisec << ","
        << local_timestamp << ","
        << "exchange" << ","
        << updateTimeField.InstrumentID << ","
        << updateTimeField.InstrumentID << ","
        << std::setiosflags(std::ios::fixed)
        << double2str(lastMatchField.LastPrice) << ","
        << lastMatchField.Volume << ","
        << double2str(lastMatchField.Turnover) << ","
        << double2str(lastMatchField.OpenInterest) << ","
        << "todayavgprice" << ","
        << double2str(staticField.OpenPrice) << ","
        << double2str(staticField.HighestPrice) << ","
        << double2str(staticField.LowestPrice) << ","
        << double2str(staticField.ClosePrice) << ","
        << double2str(staticField.SettlementPrice) << ","
        << double2str(staticField.UpperLimitPrice) << ","
        << double2str(staticField.LowerLimitPrice) << ","
        << double2str(databaseField.PreClosePrice) << ","
        << double2str(databaseField.PreSettlementPrice) << ","
        << double2str(databaseField.PreOpenInterest) << ","
        << double2str(bestPriceField.AskPrice1) << ","
        << bestPriceField.AskVolume1 << ","
        << double2str(ask23Field.AskPrice2) << ","
        << ask23Field.AskVolume2 << ","
        << double2str(ask23Field.AskPrice3) << ","
        << ask23Field.AskVolume3 << ","
        << double2str(ask45Field.AskPrice4) << ","
        << ask45Field.AskVolume4 << ","
        << double2str(ask45Field.AskPrice5) << ","
        << ask45Field.AskVolume5 << ","
        << double2str(bestPriceField.BidPrice1) << ","
        << bestPriceField.BidVolume1 << ","
        << double2str(bid23Field.BidPrice2) << ","
        << bid23Field.BidVolume2 << ","
        << double2str(bid23Field.BidPrice3) << ","
        << bid23Field.BidVolume3 << ","
        << double2str(bid45Field.BidPrice4) << ","
        << bid45Field.BidVolume4 << ","
        << double2str(bid45Field.BidPrice5) << ","
        << bid45Field.BidVolume5 << std::endl;

        //delete[] rawData->data;
        delete rawData;
    }
}


#endif //ZXFEED_L1FEED_H
