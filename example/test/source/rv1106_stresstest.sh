#!/bin/sh

#set test loop
test_loop=20000

#set frame count for every loop
frame_count=10
smartP=1
ifOpenWrap=0

vi_framerate_switch_loop=5000
ordinary_stream_test_framecount=450000

miniTest=$1

test_case()
{
    if [ ! -n "$miniTest" ]; then
        #PN mode switch
        echo -e "--------------------------------------- <sample_isp_stresstest> PN mode switch test start -------------------------------------------\n"
        echo -e "<sample_isp_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --mode_test_type 1 --test_frame_count $frame_count --mode_test_loop $test_loop>\n"
        sample_isp_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --mode_test_type 1 --test_frame_count $frame_count --mode_test_loop $test_loop
        if [ $? -eq 0 ]; then
            echo "-------------------------1 <sample_isp_stresstest> PN mode switch test success" > /tmp/rv1106_stresstest_result.log
            echo -e "--------------------------------------- <sample_isp_stresstest> PN mode switch test_result success -------------------------------------------\n\n\n"
        else 
            echo "-------------------------1 <sample_isp_stresstest> PN mode switch test failure" > /tmp/rv1106_stresstest_result.log
            echo -e "--------------------------------------- <sample_isp_stresstest> PN mode switch test_result failure -------------------------------------------\n\n\n"
            exit 0
        fi
        sleep 3
        echo 3 > /proc/sys/vm/drop_caches
        cat /proc/meminfo | grep MemAvailable >> /tmp/rv1106_stresstest_result.log

        # hdr mode switch test
        echo -e "--------------------------------------- <sample_isp_stresstest> hdr_mode_switch_test start -------------------------------------------\n"
        echo -e "<sample_isp_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --mode_test_type 2 --test_frame_count $frame_count --mode_test_loop $test_loop>\n"
        sample_isp_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --mode_test_type 2 --test_frame_count $frame_count --mode_test_loop $test_loop
        if [ $? -eq 0 ]; then
            echo "-------------------------2 <sample_isp_stresstest> HDR mode switch test success" >> /tmp/rv1106_stresstest_result.log
            echo -e "--------------------------------------- <sample_isp_stresstest> hdr_mode_switch_test success -------------------------------------------\n\n\n"
        else 
            echo "-------------------------2 <sample_isp_stresstest> HDR mode switch test failure" >> /tmp/rv1106_stresstest_result.log
            echo -e "--------------------------------------- <sample_isp_stresstest> hdr_mode_switch_test failure -------------------------------------------\n\n\n"
            exit 0
        fi
        sleep 3
        echo 3 > /proc/sys/vm/drop_caches
        cat /proc/meminfo | grep MemAvailable >> /tmp/rv1106_stresstest_result.log
    fi

    #frameRate switch test
    echo -e "--------------------------------------- <sample_isp_stresstest> frameRate_switch_test start -------------------------------------------\n"
    echo -e "<sample_isp_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --mode_test_type 3 --test_frame_count $frame_count --mode_test_loop $vi_framerate_switch_loop>\n"
    sample_isp_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --mode_test_type 3 --test_frame_count $frame_count --mode_test_loop $vi_framerate_switch_loop
    if [ $? -eq 0 ]; then
        echo "-------------------------3 <sample_isp_stresstest> isp frameRate switch test success" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_isp_stresstest> frameRate_switch_test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------3 <sample_isp_stresstest> isp frameRate switch test failure" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_isp_stresstest> frameRate_switch_test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1106_stresstest_result.log

    #LDCH mode test
    echo -e "--------------------------------------- <sample_isp_stresstest> LDCH mode test start -------------------------------------------\n"
    echo -e "<sample_isp_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --mode_test_type 4 --test_frame_count $frame_count --mode_test_loop $test_loop>\n"
    sample_isp_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --mode_test_type 4 --test_frame_count $frame_count --mode_test_loop $test_loop
    if [ $? -eq 0 ]; then
        echo "-------------------------4 <sample_isp_stresstest> LDCH mode switch test success" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_isp_stresstest> LDCH mode test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------4 <sample_isp_stresstest> LDCH mode switch test failure" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_isp_stresstest> LDCH mode test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1106_stresstest_result.log
    
     #venc resolution switch test
    echo -e "--------------------------------------- <sample_venc_stresstest> venc resolution switch test start -------------------------------------------\n"
    echo -e "<sample_venc_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpenWrap --mode_test_type 1 --mode_test_loop $test_loop --test_frame_count $frame_count>\n"
    sample_venc_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpenWrap --mode_test_type 1 --mode_test_loop $test_loop --test_frame_count $frame_count
    if [ $? -eq 0 ]; then
        echo "-------------------------5 <sample_venc_stresstest> venc resolution switch test success" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_venc_stresstest> venc resolution switch test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------5 <sample_venc_stresstest> venc resolution switch test failure" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_venc_stresstest> venc resolution switch test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1106_stresstest_result.log

    # encode type switch test
    echo -e "--------------------------------------- <sample_venc_stresstest> encode type switch test start -------------------------------------------\n"
    echo -e "<sample_venc_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpenWrap --mode_test_type 2 --mode_test_loop $test_loop --test_frame_count $frame_count>\n"
    sample_venc_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpenWrap --mode_test_type 2 --mode_test_loop $test_loop --test_frame_count $frame_count
    if [ $? -eq 0 ]; then
        echo "-------------------------6 <sample_venc_stresstest> encode type switch test success" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_venc_stresstest> encode type switch test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------6 <sample_venc_stresstest> encode type switch test failure" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_venc_stresstest> encode type switch test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1106_stresstest_result.log

    #smartp mode switch test
    echo -e "--------------------------------------- <sample_venc_stresstest> smartp mode switch test start -------------------------------------------\n"
    echo -e "<sample_venc_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpenWrap --mode_test_type 3 --mode_test_loop $test_loop --test_frame_count $frame_count>\n"
    sample_venc_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpenWrap --mode_test_type 3 --mode_test_loop $test_loop --test_frame_count $frame_count
    if [ $? -eq 0 ]; then
        echo "-------------------------7 <sample_venc_stresstest> smartp mode switch test success" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_venc_stresstest> smartp mode switch test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------7 <sample_venc_stresstest> smartp mode switch test failure" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_venc_stresstest> smartp mode switch test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1106_stresstest_result.log

    #SVC mode switch test
    echo -e "--------------------------------------- <sample_venc_stresstest> SVC mode switch test start -------------------------------------------\n"
    echo -e "<sample_venc_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpenWrap --mode_test_type 4 --mode_test_loop $test_loop --test_frame_count $frame_count>\n"
    sample_venc_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpenWrap --mode_test_type 4 --mode_test_loop $test_loop --test_frame_count $frame_count
    if [ $? -eq 0 ]; then
        echo "-------------------------8 <sample_venc_stresstest> SVC mode switch test success" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_venc_stresstest> SVC mode switch test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------8 <sample_venc_stresstest> SVC mode switch test failure" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_venc_stresstest> SVC mode switch test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1106_stresstest_result.log

    #motion deblur switch test
    echo -e "<sample_venc_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpenWrap --mode_test_type 5 --mode_test_loop $test_loop --test_frame_count $frame_count>\n"
    echo -e "--------------------------------------- <sample_venc_stresstest> motion deblur switch test start -------------------------------------------\n"
    sample_venc_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpenWrap --mode_test_type 5 --mode_test_loop $test_loop --test_frame_count $frame_count
    if [ $? -eq 0 ]; then
        echo "-------------------------9 <sample_venc_stresstest> motion deblur switch test success" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_venc_stresstest> motion deblur switch test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------9 <sample_venc_stresstest> motion deblur switch test failure" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_venc_stresstest> motion deblur switch test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1106_stresstest_result.log

    #force IDR switch test
    echo -e "--------------------------------------- <sample_venc_stresstest> force IDR switch test start -------------------------------------------\n"
    echo -e "<sample_venc_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpenWrap --mode_test_type 6 --mode_test_loop $test_loop --test_frame_count $frame_count>\n"
    sample_venc_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpenWrap --mode_test_type 6 --mode_test_loop $test_loop --test_frame_count $frame_count
    if [ $? -eq 0 ]; then
        echo "-------------------------10 <sample_venc_stresstest> force IDR switch test success" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_venc_stresstest> force IDR switch test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------10 <sample_venc_stresstest> force IDR switch test failure" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_venc_stresstest> force IDR switch test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1106_stresstest_result.log

    #venc chn rotation switch test
    echo -e "--------------------------------------- <sample_venc_stresstest> venc chn rotation test start -------------------------------------------\n"
    echo -e "<sample_venc_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpenWrap --mode_test_type 7 --mode_test_loop $test_loop --test_frame_count $frame_count>\n"
    sample_venc_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpenWrap --mode_test_type 7 --mode_test_loop $test_loop --test_frame_count $frame_count
    if [ $? -eq 0 ]; then
        echo "-------------------------11 <sample_venc_stresstest> venc chn rotation test success" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_venc_stresstest> venc chn rotation test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------11 <sample_venc_stresstest> venc chn rotation test failure" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_venc_stresstest> venc chn rotation test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 1
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1106_stresstest_result.log

    #rgn stress 
    echo -e "--------------------------------------- <sample_rgn_stresstest> Rgn attach and detach test start -------------------------------------------\n"
    echo -e "<sample_rgn_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpenWrap --inputBmp1Path /userdata/192x96.bmp --inputBmp2Path /userdata/160x96.bmp --mode_test_type 1 --mode_test_loop $test_loop --test_frame_count $frame_count>\n"
    sample_rgn_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --wrap $ifOpenWrap --inputBmp1Path /userdata/192x96.bmp --inputBmp2Path /userdata/160x96.bmp --mode_test_type 1 --mode_test_loop $test_loop --test_frame_count $frame_count
    if [ $? -eq 0 ]; then
        echo "-------------------------12 <sample_rgn_stresstest> Rgn attach and detach test success" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_rgn_stresstest> Rgn attach and detach test_result success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------12 <sample_rgn_stresstest> Rgn attach and detach test failure" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_rgn_stresstest> Rgn attach and detach test_result failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1106_stresstest_result.log

    if [ ! -n "$miniTest" ]; then
        #isp p/n mode switch
        echo -e "--------------------------------------- <sample_demo_vi_venc> isp p/n mode switch test start -------------------------------------------\n"
        echo -e "<sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --smartP $smartP --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 1>\n"
        sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --smartP $smartP --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 1
        if [ $? -eq 0 ]; then
            echo "-------------------------13 <sample_demo_vi_venc> isp p/n mode switch test success" >> /tmp/rv1106_stresstest_result.log
            echo -e "--------------------------------------- <sample_demo_vi_venc> isp p/n mode switch test success -------------------------------------------\n\n\n"
        else 
            echo "-------------------------13 <sample_demo_vi_venc> isp p/n mode switch test failure" >> /tmp/rv1106_stresstest_result.log
            echo -e "--------------------------------------- <sample_demo_vi_venc> isp p/n mode switch test failure -------------------------------------------\n\n\n"
            exit 0
        fi
        sleep 3
        echo 3 > /proc/sys/vm/drop_caches
        cat /proc/meminfo | grep MemAvailable >> /tmp/rv1106_stresstest_result.log

        #isp hdr mode switch test
        echo -e "--------------------------------------- <sample_demo_vi_venc> isp hdr mode switch switch test start -------------------------------------------\n"
        echo -e "<sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --smartP $smartP --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 2>\n"
        sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --smartP $smartP --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 2
        if [ $? -eq 0 ]; then
            echo "-------------------------14 <sample_demo_vi_venc> isp hdr mode switch test success" >> /tmp/rv1106_stresstest_result.log
            echo -e "--------------------------------------- <sample_demo_vi_venc> isp hdr mode switch switch test success -------------------------------------------\n\n\n"
        else 
            echo "-------------------------14 <sample_demo_vi_venc> isp hdr mode switch test failure" >> /tmp/rv1106_stresstest_result.log
            echo -e "--------------------------------------- <sample_demo_vi_venc> isp hdr mode switch switch test failure -------------------------------------------\n\n\n"
            exit 0
        fi
        sleep 3
        echo 3 > /proc/sys/vm/drop_caches
        cat /proc/meminfo | grep MemAvailable >> /tmp/rv1106_stresstest_result.log
    fi

    #isp framerate switch test
    echo -e "--------------------------------------- <sample_demo_vi_venc> isp framerate switch test start -------------------------------------------\n"
    echo -e "<sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --smartP $smartP --test_frame_count $frame_count --mode_test_loop $vi_framerate_switch_loop --mode_test_type 3\n>"
    sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --smartP $smartP --test_frame_count $frame_count --mode_test_loop $vi_framerate_switch_loop --mode_test_type 3
    if [ $? -eq 0 ]; then
        echo "-------------------------15 <sample_demo_vi_venc> isp framerate switch test success" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> isp framerate switch test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------15 <sample_demo_vi_venc> isp framerate switch test failure" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> isp framerate switch test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1106_stresstest_result.log

    #isp LDCH switch test
    echo -e "--------------------------------------- <sample_demo_vi_venc> isp LDCH switch test start -------------------------------------------\n"
    echo -e "<sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --smartP $smartP --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 4>\n"
    sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --smartP $smartP --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 4
    if [ $? -eq 0 ]; then
        echo "-------------------------16 <sample_demo_vi_venc> isp LDCH switch test success" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> isp LDCH  switch test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------16 <sample_demo_vi_venc> isp LDCH switch test failure" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> isp LDCH  switch test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1106_stresstest_result.log

    #venc resolution switch test
    echo -e "--------------------------------------- <sample_demo_vi_venc> venc resolution switch test start -------------------------------------------\n"
    echo -e "<sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --smartP $smartP --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 5>\n"
    sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --smartP $smartP --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 5
    if [ $? -eq 0 ]; then
        echo "-------------------------17 <sample_demo_vi_venc> venc resolution switch test success" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> venc resolution switch test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------17 <sample_demo_vi_venc> venc resolution switch test failure" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> venc resolution switch test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1106_stresstest_result.log

    # encode type switch test
    echo -e "--------------------------------------- <sample_demo_vi_venc> encode type switch test start -------------------------------------------\n"
    echo -e "<sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --smartP $smartP --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 6>\n"
    sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --smartP $smartP --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 6
    if [ $? -eq 0 ]; then
        echo "-------------------------18 <sample_demo_vi_venc> venc encode type switch test success" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> encode type switch test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------18 <sample_demo_vi_venc> venc encode type switch test failure" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> encode type switch test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1106_stresstest_result.log

    #smartp mode switch test
    echo -e "--------------------------------------- <sample_demo_vi_venc> smartp mode switch test start -------------------------------------------\n"
    echo -e "<sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --smartP $smartP --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 7>\n"
    sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --smartP $smartP --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 7
    if [ $? -eq 0 ]; then
        echo "-------------------------19 <sample_demo_vi_venc> venc smartp mode switch test success" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> smartp mode switch test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------19 <sample_demo_vi_venc> venc smartp mode switch test failure" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> smartp mode switch test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1106_stresstest_result.log

    #SVC mode switch test
    echo -e "--------------------------------------- <sample_demo_vi_venc> SVC mode switch test start -------------------------------------------\n"
    echo -e "<sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --smartP $smartP --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 8>\n"
    sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --smartP $smartP --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 8
    if [ $? -eq 0 ]; then
        echo "-------------------------20 <sample_demo_vi_venc> venc SVC mode switch test success" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> SVC mode switch test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------20 <sample_demo_vi_venc> venc SVC mode switch test failure" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> SVC mode switch test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1106_stresstest_result.log

    #motion deblur switch test
    echo -e "--------------------------------------- <sample_demo_vi_venc> motion deblur switch test start -------------------------------------------\n"
    echo -e "<sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --smartP $smartP --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 9>\n"
    sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --smartP $smartP --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 9
    if [ $? -eq 0 ]; then
        echo "-------------------------21 <sample_demo_vi_venc> venc motion deblur switch test success" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> motion deblur switch test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------21 <sample_demo_vi_venc> venc motion deblur switch test failure" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> motion deblur switch test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1106_stresstest_result.log

    #force IDR switch test
    echo -e "--------------------------------------- <sample_demo_vi_venc> force IDR switch test start -------------------------------------------\n"
    echo -e "<sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --smartP $smartP --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 10>\n"
    sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --smartP $smartP --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 10
    if [ $? -eq 0 ]; then
        echo "-------------------------22 <sample_demo_vi_venc> venc force IDR switch test success" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> force IDR switch test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------22 <sample_demo_vi_venc> venc force IDR switch test failure" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> force IDR switch test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1106_stresstest_result.log

    #venc chn rotation switch test
    echo -e "--------------------------------------- <sample_demo_vi_venc> venc chn rotation test start -------------------------------------------\n"
    echo -e "<sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --smartP $smartP --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 11>\n"
    sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --smartP $smartP --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 11
    if [ $? -eq 0 ]; then
        echo "-------------------------23 <sample_demo_vi_venc> venc chn rotation switch test success" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> venc chn rotation test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------23 <sample_demo_vi_venc> venc chn rotation switch test failure" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> venc chn rotation test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1106_stresstest_result.log

    #rgn detach/attach switch test
    echo -e "--------------------------------------- <sample_demo_vi_venc> rgn detach/attach test start -------------------------------------------\n"
    echo -e "<sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --smartP $smartP --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 12>\n"
    sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l -1 --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --smartP $smartP --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 12
    if [ $? -eq 0 ]; then
        echo "-------------------------24 <sample_demo_vi_venc> rgn detach/attach switch test success" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> rgn detach/attach test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------24 <sample_demo_vi_venc> rgn detach/attach switch test failure" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> rgn detach/attach test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1106_stresstest_result.log

    #ordinary stream test
    echo -e "--------------------------------------- <sample_demo_vi_venc> ordinary stream test start -------------------------------------------\n"
    echo -e "<sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l $ordinary_stream_test_framecount --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --smartP $smartP --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 0>\n"
    sample_demo_vi_venc -w 1920 -h 1080 -a /etc/iqfiles/ -l $ordinary_stream_test_framecount --inputBmp1Path /userdata/160x96.bmp --inputBmp2Path /userdata/192x96.bmp --smartP $smartP --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 0
    if [ $? -eq 0 ]; then
        echo "-------------------------25 <sample_demo_vi_venc> ordinary stream test success" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> ordinary stream test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------25 <sample_demo_vi_venc> ordinary stream test failure" >> /tmp/rv1106_stresstest_result.log
        echo -e "--------------------------------------- <sample_demo_vi_venc> ordinary stream test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 1
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/rv1106_stresstest_result.log

    echo "-------------------------ALL Test End" >> /tmp/rv1106_stresstest_result.log
    echo -e "--------------------------------------- ALL Test End -------------------------------------------\n\n"
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

cat /tmp/rv1106_stresstest_result.log
cat /tmp/testLog.txt
