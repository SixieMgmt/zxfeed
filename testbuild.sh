#!/bin/bash
g++ -pthread main2.cpp consts.h l1feed.h l2feed2.h ntime.h -D_GNU_SOURCE
rm *.gch
