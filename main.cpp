#include <iostream>
#include <fstream>

#include "ntime.h"
#include "l1feed.h"
#include "l2feed.h"
#include <signal.h> 

using namespace std;
pthread_t tl1;
pthread_t tl2;

void sigbus_handle(int signo)  
{  
    printf("signo: [%d]\n",signo);
    if (signo == SIGTERM)
    {
        printf("begin kill\n");
        stopL2();
        raise(SIGKILL);
        printf("end kill\n");
    }
   
}  

int main(int argc, char* argv[]) {
    std::fstream* fs = new std::fstream;
    char file_name[256] = {0};
    struct timeval t;
    gettimeofday(&t, NULL);
    struct tm * p = localtime(&t.tv_sec);
    int local_date = (1900+p->tm_year) * 10000 + (p->tm_mon + 1) * 100 + p->tm_mday;

    signal(SIGTERM,sigbus_handle);
    uint64_t now = (p->tm_hour * 3600 + p->tm_min * 60 + p->tm_sec) * 1000ul;
    std::cout << now << ":" << t0800 << ":" << t1500 << std::endl;
    string suffix =  "day";
    if (now >= t0800 && now <= t1500)
        suffix = "day";
    else
        suffix = "night";
    int n = snprintf(file_name, sizeof(file_name), "%s/l1_%d.%s", argv[1],local_date,suffix.c_str());
    if (access(file_name, F_OK) == 0) {
        fs->open(file_name, std::fstream::out|std::fstream::ate|std::fstream::app);
    } else {
        fs->open(file_name, std::fstream::out|std::fstream::app);

    }

    std::fstream* fsl2 = new std::fstream;
    char file_name_l2[256] = {0};
    int m = snprintf(file_name_l2, sizeof(file_name_l2), "%s/l2_%d.%s", argv[1],local_date,suffix.c_str());
    std::cout << file_name_l2 << endl;
    if (access(file_name_l2, F_OK) == 0) {
        fsl2->open(file_name_l2, std::fstream::out|std::fstream::ate|std::fstream::app);
    } else {
        fsl2->open(file_name_l2, std::fstream::out|std::fstream::app);

    }

    int l1port = atoi(argv[2]);
    int l2port = atoi(argv[3]);
    ThreadARG l1arg;
    ThreadARG l2arg;
    l1arg.fs = fs;
    l1arg.port = l1port;

    l2arg.fs = fsl2;
    l2arg.port = l2port;
    strcpy(l2arg.filePath,file_name_l2);
    int rett = pthread_create(&tl1, NULL, runl1, &l1arg); //参数：创建的线程id，线程参数，线程运行函数的起始地址，运行函数的参数
    if (rett != 0) //创建线程成功返回0
        printf("pthread_create(ParseMarketData) error:error_code=%d\n", rett);

    rett = pthread_create(&tl2, NULL, runl2, &l2arg); //参数：创建的线程id，线程参数，线程运行函数的起始地址，运行函数的参数
    if (rett != 0) //创建线程成功返回0
        printf("pthread_create(ParseMarketData) error:error_code=%d\n", rett);

    while(1)
    {
        sleep(100);
    }

}
