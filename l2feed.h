#ifndef ZXFEED_L2FEED_H
#define ZXFEED_L2FEED_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
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
#include <pthread.h>
#include <time.h>
#include <queue>
#include "consts.h"
#include "float.h"
#include <sys/mman.h>
#include <numeric>
#include <fcntl.h>
#include <signal.h> 

using std::queue;
using namespace std;

#define GET_LAST_SOCK_ERROR() errno
typedef unsigned int DWORD;
typedef unsigned short WORD;
typedef unsigned int UNINT32;
typedef unsigned char BYTE;
#define ioctlsocket ioctl


//queue<char *> queueData;
queue<int> queueData;
int fd;
uint64_t pos = 0;
pthread_t tidParse;
uint64_t FILESIZE = 1024 * 1024 * 1024 * 50ul;
char *map;
void *ParseMarketData(void *args);

//在传送sockaddr时，使用的表示长度的类型不同 gyd 20020605 将 UNIX变为LINUX

#define SOCKADDRLEN    socklen_t

#pragma pack(push)
#pragma pack(1)
struct CMBLMarketDataField {
    ///合约代码
    char InstrumentID[31];
    ///买卖方向
    char Direction;
    ///价格
    double Price;
    ///数量
    int Volume;
};
#pragma pack(pop)

pthread_mutex_t mutex;


std::string double2str2(double v) {
    if (v == DBL_MAX)
        return "NaN";
    else {
        char result[50] = {0};
        snprintf(result, 50, "%.6f", v);
        return result;
    }
}

void *ParseMarketData(void* arg) {
	char* map = (char*) arg;
	char *buffer;
    // char comma = ',';
    char endline = '\n';
	//CMBLMarketDataField theMarketData;
    //uint64_t pos = 0;
    char volumeValue[8];
    // char* priceValue;
    int i;
	while (true)
	{
    	
    	// snprintf(local_timestamp, sizeof(local_timestamp), "%04d%02d%02d,%02d%02d%02d.%06d",
     //         1900 + p->tm_year, p->tm_mon+1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, (int32_t)(t.tv_usec));

		pthread_mutex_lock(&mutex);
		if (queueData.size() == 0)
		{
			pthread_mutex_unlock(&mutex);
			//usleep(1000);
			continue;
		}
		//buffer = queueData.front();
		i = queueData.front();
		queueData.pop();
		pthread_mutex_unlock(&mutex);

		// struct timeval t;
  //   	gettimeofday(&t, NULL);

  //       memcpy(&map[pos],&t,sizeof(t));
  //       pos += sizeof(t);

        //memcpy(&map[pos],buffer,sizeof(CMBLMarketDataField));
        //pos += sizeof(CMBLMarketDataField);
        sprintf(volumeValue, "%d", i);
        memcpy(&map[pos],volumeValue,strlen(volumeValue));
        pos += strlen(volumeValue);

        map[pos] = endline;
        pos += 1;

    	//memcpy(&theMarketData, buffer, sizeof(theMarketData));
    	
    	// (*fstream) << local_timestamp << ","
    	// << theMarketData.InstrumentID << ","
    	// << theMarketData.Direction << ","
    	// << double2str2(theMarketData.Price) << ","
    	// << theMarketData.Volume << std::endl;

        // memcpy(&map[pos],local_timestamp,strlen(local_timestamp));
        // pos += strlen(local_timestamp);

        // map[pos] = comma;
        // pos += 1;

        // memcpy(&map[pos],theMarketData.InstrumentID,strlen(theMarketData.InstrumentID));
        // pos += strlen(theMarketData.InstrumentID);

        // map[pos] = comma;
        // pos += 1;

        // map[pos] = theMarketData.Direction;
        // pos += 1;

        // map[pos] = comma;
        // pos += 1;

        // priceValue = const_cast<char*>(double2str2(theMarketData.Price).c_str());
        // memcpy(&map[pos],priceValue,strlen(priceValue));
        // pos += strlen(priceValue);

        // map[pos] = comma;
        // pos += 1;

        // //itoa(theMarketData.Volume,volumeValue,10);
        // sprintf(volumeValue, "%d", theMarketData.Volume);
        // memcpy(&map[pos],volumeValue,strlen(volumeValue));
        // pos += strlen(volumeValue);

        // map[pos] = endline;
        // pos += 1;
         
        
		delete[] buffer;
	}
    // CMBLMarketDataField theMarketData;
    // char local_timestamp[128] = {0};
    // struct timeval t;
    // gettimeofday(&t, NULL);
    // struct tm * p = localtime(&t.tv_sec);
    // snprintf(local_timestamp, sizeof(local_timestamp), "%04d%02d%02d,%02d%02d%02d.%06d",
    //          1900 + p->tm_year, p->tm_mon+1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, (int32_t)(t.tv_usec));
    // memcpy(&theMarketData, buffer, sizeof(theMarketData));
    // (*fstream) << local_timestamp << ","
    // << theMarketData.InstrumentID << ","
    // << theMarketData.Direction << ","
    // << double2str2(theMarketData.Price) << ","
    // << theMarketData.Volume << std::endl;
}

void stopL2(){
	munmap(map,FILESIZE);
	ftruncate(fd, pos);
	close(fd);
}

void* runl2(void* arg) {
    ThreadARG* info = (ThreadARG*) arg;
    int port  = info->port;
    std::fstream* fs = info->fs;
    char* filepath = info->filePath;

    //begin mmap
    fd = open(filepath, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);

    if (fd == -1)
    {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    // Stretch the file size to the size of the (mmapped) array of char


    if (lseek(fd, FILESIZE-1, SEEK_SET) == -1)
    {
        close(fd);
        perror("Error calling lseek() to 'stretch' the file");
        exit(EXIT_FAILURE);
    }

    /* Something needs to be written at the end of the file to
     * have the file actually have the new size.
     * Just writing an empty string at the current file position will do.
     *
     * Note:
     *  - The current position in the file is at the end of the stretched 
     *    file due to the call to lseek().
     *  - An empty string is actually a single '\0' character, so a zero-byte
     *    will be written at the last byte of the file.
     */

    if (write(fd, "", 1) == -1)
    {
        close(fd);
        perror("Error writing last byte of the file");
        exit(EXIT_FAILURE);
    }


    // Now the file is ready to be mmapped.
    map = (char*)mmap(0, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED)
    {
        close(fd);
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    }

    //end mmap


    struct sockaddr_in servaddr; //IPv4套接口地址定义
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0); //建立UDP套接字
    /* set reuse and non block for this socket */
    int son = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &son, sizeof(son));

    memset(&servaddr, 0, sizeof(servaddr)); //地址结构清零
    servaddr.sin_family = AF_INET; //IPv4协议
    //servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//内核指定地址
    servaddr.sin_addr.s_addr = inet_addr("172.203.250.255");
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

    int number = 1024;
    char *buffer;

    //pthread_mutex_init(&mutex, NULL);
    //queueData = queue<char *>();
    // queueData = queue<int>();
    // int rett = pthread_create(&tidParse, NULL, ParseMarketData, map); //参数：创建的线程id，线程参数，线程运行函数的起始地址，运行函数的参数
    // if (rett != 0) //创建线程成功返回0
    //     printf("pthread_create(ParseMarketData) error:error_code=%d\n", rett);

    int len = sizeof(CMBLMarketDataField);
    CMBLMarketDataField theMarketData;
    sockaddr_in fromAddr;
    int nFromLen, nCount;
    int i = 0;
    char volumeValue[8];
    while (1) {
    	buffer = new char[number];
        memset(buffer, 0, number);
        nFromLen = sizeof(sockaddr_in);
        nCount = recvfrom(sockfd, buffer, number, 0, (sockaddr *) &fromAddr, (socklen_t *) &nFromLen);
        //printf("buffer length[%d],strlen [%d] nCount [%d]\n", sizeof(buffer),strlen(buffer),nCount); 
        if (nCount == 0) {
        	delete[] buffer;
            continue;
        }
        if (nCount == -1) {
            int nErrno = GET_LAST_SOCK_ERROR();
            if (nErrno == 0 || nErrno == 251 || nErrno == EWOULDBLOCK)
            {
            	//printf("EWOULDBLOCK Error in recvFrom,ErrNo[%d],size[%d]\n", nErrno,sizeof(buffer));
            	//std::cout << "ErrNo:" << nErrno << "buffer:" << buffer << std::endl;
            	// memcpy(&theMarketData, buffer, sizeof(theMarketData));
            	// printf("instrumentID[%s]\n",theMarketData.InstrumentID);
            	delete[] buffer;
            	continue;
            }    /*251 for PARisk */    //20060224 IA64 add 0
                
            else {
                printf("Error in recvFrom,ErrNo[%d]\n", nErrno);
                delete[] buffer;
//                return -1;
            }
        }
        if (nCount < len) {
        	//printf("nCount [%d]\n",nCount);
            delete[] buffer;
            continue;
        }

        // pthread_mutex_lock(&mutex);
        // //queueData.push(buffer);
        // i++;
        // queueData.push(i);
        // pthread_mutex_unlock(&mutex);
        
        struct timeval t;
    	gettimeofday(&t, NULL);

        memcpy(&map[pos],&t,sizeof(t));
        pos += sizeof(t);
     
     	//sprintf(volumeValue, "%d", i);
     	// i++;
      //   memcpy(&map[pos],&i,sizeof(int));
      //   pos += sizeof(int);

        memcpy(&map[pos],buffer,sizeof(CMBLMarketDataField));
        pos += sizeof(CMBLMarketDataField);

//        ParseMarketData(fs,buffer);
		delete[] buffer;
    }

//    return 0;
}
#endif //ZXFEED_L2FEED_H





