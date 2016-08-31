#!/bin/bash
Base_Path='/home/juns/work/zxfeed/data/'
lastday=`date  +"%Y%m%d" -d  "-1 days"`
l1_file=$Base_Path"l1_"$lastday".night"
#echo $l1_file
l2_file=$Base_Path"l2_"$lastday".night"
#echo $l2_file
gzip -cf $l1_file > $l1_file".gz"
gzip -cf $l2_file > $l2_file".gz"

