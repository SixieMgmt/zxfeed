#!/bin/bash
g++ -pthread main.cpp consts.h l1feed.h l2feed.h ntime.h
mv a.out zxfeed
rm *.gch
