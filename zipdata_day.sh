#!/bin/bash
Base_Path='/home/juns/work/zxfeed/data/'
today=`date +"%Y%m%d"`
l1_file=$Base_Path"l1_"$today".day"
l2_file=$Base_Path"l2_"$today".day"
echo $l2_file
gzip -cf $l1_file > $l1_file".gz"
gzip -cf $l2_file > $l2_file".gz"

