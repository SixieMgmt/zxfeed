//
// Created by sunjun on 16-6-14.
//

#ifndef ZXFEED_CONSTS_H
#define ZXFEED_CONSTS_H

#include <fstream>

typedef struct threadArg {
    std::fstream* fs;
    int port;
    char filePath[256];
} ThreadARG;

#endif //ZXFEED_CONSTS_H
