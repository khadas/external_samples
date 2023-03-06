#!/bin/sh

#set test loop
test_loop=20000

#set frame count for every loop
frame_count=10

#wrap function for 1103
ifOpen_wrap=1
vi_buff_cnt=1

vi_framerate_switch_loop=5000
ordinary_stream_test_framecount=450000


test_case()
{
    #PN mode switch
    echo -e "--------------------------------------- <sample_isp_stresstest> PN mode switch test start -------------------------------------------\n"
    echo -e "<sample_isp_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --mode_test_type 1 --test_frame_count $frame_count --mode_test_loop $test_loop >\n"
    sample_isp_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --mode_test_type 1 --test_frame_count $frame_count --mode_test_loop $test_loop
    if [ $? -eq 0 ]; then
        echo "-------------------------1 <sample_isp_stresstest> isp p/n mode switch test success" > /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_isp_stresstest> PN mode switch test_result success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------1 <sample_isp_stresstest> isp p/n mode switch test failure" > /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_isp_stresstest> PN mode switch test_result failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1103_stresstest_result.log

    #LDCH mode test
    echo -e "--------------------------------------- <sample_isp_stresstest> LDCH mode test start -------------------------------------------\n"
    echo -e "<sample_isp_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --mode_test_type 4 --test_frame_count $frame_count --mode_test_loop $test_loop >\n"
    sample_isp_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --mode_test_type 4 --test_frame_count $frame_count --mode_test_loop $test_loop
    if [ $? -eq 0 ]; then
        echo "-------------------------2 <sample_isp_stresstest> isp LDCH mode switch test success" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_isp_stresstest> LDCH mode test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------2 <sample_isp_stresstest> isp LDCH mode switch test failure" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_isp_stresstest> LDCH mode test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1103_stresstest_result.log
    
    #frameRate switch test
    echo -e "--------------------------------------- <sample_isp_stresstest> frameRate_switch_test start -------------------------------------------\n"
    echo -e "<sample_isp_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --mode_test_type 3 --test_frame_count $frame_count --mode_test_loop $vi_framerate_switch_loop >\n"
    sample_isp_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --mode_test_type 3 --test_frame_count $frame_count --mode_test_loop $vi_framerate_switch_loop
    if [ $? -eq 0 ]; then
        echo "-------------------------3 <sample_isp_stresstest> isp framerate switch test success" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_isp_stresstest> frameRate_switch_test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------3 <sample_isp_stresstest> isp framerate mode switch test failure" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_isp_stresstest> frameRate_switch_test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1103_stresstest_result.log

    #isp restart mode test
    echo -e "--------------------------------------- <sample_isp_stresstest> isp_restart mode test start -------------------------------------------\n"
    echo -e "<sample_isp_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --mode_test_type 6 --test_frame_count $frame_count --mode_test_loop $test_loop>\n"
    sample_isp_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --mode_test_type 6 --test_frame_count $frame_count --mode_test_loop $test_loop
    if [ $? -eq 0 ]; then
        echo "-------------------------6 <sample_isp_stresstest> isp_restart mode switch test success" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_isp_stresstest> isp_restart mode test success -------------------------------------------\n\n\n"
    else
        echo "-------------------------6 <sample_isp_stresstest> isp_restart mode switch test failure" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_isp_stresstest> isp_restart mode test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1103_stresstest_result.log

    #venc resolution switch test
    echo -e "--------------------------------------- <sample_venc_stresstest> venc resolution switch test start -------------------------------------------\n"
    echo -e "<sample_venc_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpen_wrap --mode_test_type 1 --mode_test_loop $test_loop --test_frame_count $frame_count>\n"
    sample_venc_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpen_wrap --mode_test_type 1 --mode_test_loop $test_loop --test_frame_count $frame_count
    if [ $? -eq 0 ]; then
        echo "-------------------------4 <sample_venc_stresstest> venc resolution switch test success" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_venc_stresstest> venc resolution switch test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------4 <sample_venc_stresstest> venc resolution switch test failure" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_venc_stresstest> venc resolution switch test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1103_stresstest_result.log

    # encode type switch test
    echo -e "--------------------------------------- <sample_venc_stresstest> encode type switch test start -------------------------------------------\n"
    echo -e "<sample_venc_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpen_wrap --mode_test_type 2 --mode_test_loop $test_loop --test_frame_count $frame_count>\n"
    sample_venc_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpen_wrap --mode_test_type 2 --mode_test_loop $test_loop --test_frame_count $frame_count
    if [ $? -eq 0 ]; then
        echo "-------------------------5 <sample_venc_stresstest> venc encode type switch test success" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_venc_stresstest> encode type switch test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------5 <sample_venc_stresstest> venc encode type switch test failure" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_venc_stresstest> encode type switch test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1103_stresstest_result.log

    #smartp mode switch test
    echo -e "--------------------------------------- <sample_venc_stresstest> smartp mode switch test start -------------------------------------------\n"
    echo -e "<sample_venc_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpen_wrap --mode_test_type 3 --mode_test_loop $test_loop --test_frame_count $frame_count>\n"
    sample_venc_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpen_wrap --mode_test_type 3 --mode_test_loop $test_loop --test_frame_count $frame_count
    if [ $? -eq 0 ]; then
        echo "-------------------------6 <sample_venc_stresstest> venc smartp mode switch test success" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_venc_stresstest> smartp mode switch test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------6 <sample_venc_stresstest> venc smartp mode switch test failure" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_venc_stresstest> smartp mode switch test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1103_stresstest_result.log

    #SVC mode switch test
    echo -e "--------------------------------------- <sample_venc_stresstest> SVC mode switch test start -------------------------------------------\n"
    echo -e "<sample_venc_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpen_wrap --mode_test_type 4 --mode_test_loop $test_loop --test_frame_count $frame_count>\n"
    sample_venc_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpen_wrap --mode_test_type 4 --mode_test_loop $test_loop --test_frame_count $frame_count
    if [ $? -eq 0 ]; then
        echo "-------------------------7 <sample_venc_stresstest> venc SVC mode switch test success" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_venc_stresstest> SVC mode switch test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------7 <sample_venc_stresstest> venc SVC mode switch test failure" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_venc_stresstest> SVC mode switch test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1103_stresstest_result.log

    #motion deblur switch test
    echo -e "--------------------------------------- <sample_venc_stresstest> motion deblur switch test start -------------------------------------------\n"
    echo -e "<sample_venc_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpen_wrap --mode_test_type 5 --mode_test_loop $test_loop --test_frame_count $frame_count>\n"
    sample_venc_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpen_wrap --mode_test_type 5 --mode_test_loop $test_loop --test_frame_count $frame_count
    if [ $? -eq 0 ]; then
        echo "-------------------------8 <sample_venc_stresstest> venc motion deblur switch test success" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_venc_stresstest> motion deblur switch test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------8 <sample_venc_stresstest> venc motion deblur switch test failure" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_venc_stresstest> motion deblur switch test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1103_stresstest_result.log

    #force IDR switch test
    echo -e "--------------------------------------- <sample_venc_stresstest> force IDR switch test start -------------------------------------------\n"
    echo -e "<sample_venc_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpen_wrap --mode_test_type 6 --mode_test_loop $test_loop --test_frame_count $frame_count>\n"
    sample_venc_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpen_wrap --mode_test_type 6 --mode_test_loop $test_loop --test_frame_count $frame_count
    if [ $? -eq 0 ]; then
        echo "-------------------------9 <sample_venc_stresstest> venc force IDR switch test success" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_venc_stresstest> force IDR switch test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------9 <sample_venc_stresstest> venc force IDR switch test failure" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_venc_stresstest> force IDR switch test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1103_stresstest_result.log

    #rgn stress 
    echo -e "--------------------------------------- <sample_rgn_stresstest> Rgn attach and detach test start -------------------------------------------\n"
    echo -e "<sample_rgn_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpen_wrap --inputBmp1Path /userdata/192x96.bmp --inputBmp2Path /userdata/160x96.bmp --mode_test_type 1 --mode_test_loop $test_loop --test_frame_count $frame_count>\n"
    sample_rgn_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpen_wrap --inputBmp1Path /userdata/192x96.bmp --inputBmp2Path /userdata/160x96.bmp --mode_test_type 1 --mode_test_loop $test_loop --test_frame_count $frame_count
    if [ $? -eq 0 ]; then
        echo "-------------------------10 <sample_venc_stresstest> Rgn attach and detach switch test success" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_rgn_stresstest> Rgn attach and detach test_result success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------10 <sample_venc_stresstest> Rgn attach and detach switch test failure" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_rgn_stresstest> Rgn attach and detach test_result failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1103_stresstest_result.log


    #isp p/n mode switch
    echo -e "--------------------------------------- <sample_demo_vi_venc> isp p/n mode switch test start -------------------------------------------\n"
    echo -e "<sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --wrap $ifOpen_wrap --vi_buff_cnt $vi_buff_cnt  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 1>\n"
    sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --wrap $ifOpen_wrap --vi_buff_cnt $vi_buff_cnt  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 1
    if [ $? -eq 0 ]; then
        echo -e "--------------------------------------- <sample_demo_vi_venc> isp p/n mode switch test success -------------------------------------------\n\n\n"
        echo "-------------------------11 <sample_demo_vi_venc> isp p/n mode switch test success" >> /tmp/rv1103_stresstest_result.log
    else 
        echo -e "--------------------------------------- <sample_demo_vi_venc> isp p/n mode switch test failure -------------------------------------------\n\n\n"
        echo "-------------------------11 <sample_demo_vi_venc> isp p/n mode switch test failure" >> /tmp/rv1103_stresstest_result.log
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1103_stresstest_result.log


    #isp LDCH switch
    echo -e "--------------------------------------- <sample_demo_vi_venc> isp LDCH switch test start -------------------------------------------\n"
    echo -e "<sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --wrap $ifOpen_wrap --vi_buff_cnt $vi_buff_cnt  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 4>\n"
    sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --wrap $ifOpen_wrap --vi_buff_cnt $vi_buff_cnt  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 4
    if [ $? -eq 0 ]; then
        echo "-------------------------12 <sample_demo_vi_venc> isp LDCH switch test success" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> isp LDCH switch test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------12 <sample_demo_vi_venc> isp LDCH switch test failure" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> isp LDCH switch test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1103_stresstest_result.log

    #venc resolution switch test
    echo -e "--------------------------------------- <sample_demo_vi_venc> venc resolution switch test start -------------------------------------------\n"
    echo -e "<sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --wrap $ifOpen_wrap --vi_buff_cnt $vi_buff_cnt  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 5>\n"
    sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --wrap $ifOpen_wrap --vi_buff_cnt $vi_buff_cnt  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 5
    if [ $? -eq 0 ]; then
        echo "-------------------------13 <sample_demo_vi_venc> venc resolution switch test success" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> venc resolution switch test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------13 <sample_demo_vi_venc> venc resolution switch test failure" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> venc resolution switch test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1103_stresstest_result.log

    # encode type switch test
    echo -e "--------------------------------------- <sample_demo_vi_venc> encode type switch test start -------------------------------------------\n"
    echo -e "<sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --wrap $ifOpen_wrap --vi_buff_cnt $vi_buff_cnt  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 6>\n"
    sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --wrap $ifOpen_wrap --vi_buff_cnt $vi_buff_cnt  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 6
    if [ $? -eq 0 ]; then
        echo "-------------------------14 <sample_demo_vi_venc> venc encode type switch test success" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> encode type switch test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------14 <sample_demo_vi_venc> venc encode type switch test failure" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> encode type switch test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1103_stresstest_result.log

    #SVC mode switch test
    echo -e "--------------------------------------- <sample_demo_vi_venc> SVC mode switch test start -------------------------------------------\n"
    echo -e "<sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --wrap $ifOpen_wrap --vi_buff_cnt $vi_buff_cnt  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 8>\n"
    sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --wrap $ifOpen_wrap --vi_buff_cnt $vi_buff_cnt  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 8
    if [ $? -eq 0 ]; then
        echo "-------------------------15 <sample_demo_vi_venc> SVC mode switch test success" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> SVC mode switch test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------15 <sample_demo_vi_venc> SVC mode switch test failure" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> SVC mode switch test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1103_stresstest_result.log

    #motion deblur switch test
    echo -e "--------------------------------------- <sample_demo_vi_venc> motion deblur switch test start -------------------------------------------\n"
    echo -e "<sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --wrap $ifOpen_wrap --vi_buff_cnt $vi_buff_cnt  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 9>\n"
    sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --wrap $ifOpen_wrap --vi_buff_cnt $vi_buff_cnt  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 9
    if [ $? -eq 0 ]; then
        echo "-------------------------16 <sample_demo_vi_venc> motion deblur switch test success" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> motion deblur switch test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------16 <sample_demo_vi_venc> motion deblur switch test failure" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> motion deblur switch test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1103_stresstest_result.log

    #force IDR switch test
    echo -e "--------------------------------------- <sample_demo_vi_venc> force IDR switch test start -------------------------------------------\n"
    echo -e "<sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --wrap $ifOpen_wrap --vi_buff_cnt $vi_buff_cnt  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 10>\n"
    sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --wrap $ifOpen_wrap --vi_buff_cnt $vi_buff_cnt  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 10
    if [ $? -eq 0 ]; then
        echo "-------------------------17 <sample_demo_vi_venc> force IDR switch test success" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> force IDR switch test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------17 <sample_demo_vi_venc> force IDR switch test failure" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> force IDR switch test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1103_stresstest_result.log

    #venc rgn attach/detach switch test
    echo -e "--------------------------------------- <sample_demo_vi_venc> rgn attach/detach test start -------------------------------------------\n"
    echo -e "<sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --wrap $ifOpen_wrap --vi_buff_cnt $vi_buff_cnt  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 12>\n"
    sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --wrap $ifOpen_wrap  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 12
    if [ $? -eq 0 ]; then
        echo "-------------------------18 <sample_demo_vi_venc> venc rgn attach/detach switch test success" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> rgn attach/detach test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------18 <sample_demo_vi_venc> venc rgn attach/detach switch test failure" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> rgn attach/detach test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1103_stresstest_result.log

    #ordinary stream
    echo -e "--------------------------------------- <sample_demo_vi_venc> ordinary stream test start -------------------------------------------\n"
    echo -e "<sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l $ordinary_stream_test_framecount --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --wrap $ifOpen_wrap  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 0>\n"
    sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l $ordinary_stream_test_framecount --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --wrap $ifOpen_wrap  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 0
    if [ $? -eq 0 ]; then
        echo "-------------------------19 <sample_demo_vi_venc> ordinary stream test success" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> ordinary stream test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------19 <sample_demo_vi_venc> ordinary stream test failure" >> /tmp/rv1103_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> ordinary stream test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1103_stresstest_result.log



    echo -e "--------------------------------------- ALL Test End -------------------------------------------\n\n"
    echo "-------------------------ALL Test End" >> /tmp/rv1103_stresstest_result.log
}


killall rkipc
while true
do
    ps|grep rkipc |grep -v grep
    if [ $? -ne 0 ]; then
        echo "rkipc exit"
        break
    else
        echo "rkipc active"
    fi
done

test_case

cat /tmp/rv1103_stresstest_result.log
cat /tmp/testLog.txt
