#!/bin/sh

#set test loop
test_loop=10000

#set frame count for every loop
frame_count=10

test_case()
{
    #1: PN mode switch
    echo -e "--------------------------------------- <sample_mulit_isp_stresstest> PN mode switch test start (no-group)-------------------------------------------\n"
    echo -e "<sample_mulit_isp_stresstest -a /oem/usr/share/iqfiles -c 2 -w 1920 -h 1080 --modeTestType 1 --modeTestLoop $test_loop --testFrameCount $frame_count --ispLaunchMode 0>\n"
    sample_mulit_isp_stresstest -a /oem/usr/share/iqfiles -c 2 -w 1920 -h 1080 --modeTestType 1 --modeTestLoop $test_loop --testFrameCount $frame_count --ispLaunchMode 0
    if [ $? -eq 0 ]; then
        echo "-------------------------1 <sample_mulit_isp_stresstest> PN mode switch test success (no-group)" > /tmp/multi_isp_stresstest_result.log
        echo -e "--------------------------------------- <sample_mulit_isp_stresstest> PN mode switch test_result success (no-group)-------------------------------------------\n\n\n"
    else 
        echo "-------------------------1 <sample_mulit_isp_stresstest> PN mode switch test failure (no-group)" > /tmp/multi_isp_stresstest_result.log
        echo -e "--------------------------------------- <sample_mulit_isp_stresstest> PN mode switch test_result failure (no-group)-------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/multi_isp_stresstest_result.log

    #2: hdr mode switch test
    echo -e "--------------------------------------- <sample_mulit_isp_stresstest> hdr_mode_switch_test start (no-group)-------------------------------------------\n"
    echo -e "<sample_mulit_isp_stresstest -a /oem/usr/share/iqfiles -c 2 -w 1920 -h 1080 --modeTestType 2 --modeTestLoop $test_loop --testFrameCount $frame_count --ispLaunchMode 0>\n"
    sample_mulit_isp_stresstest -a /oem/usr/share/iqfiles -c 2 -w 1920 -h 1080 --modeTestType 2 --modeTestLoop $test_loop --testFrameCount $frame_count --ispLaunchMode 0
    if [ $? -eq 0 ]; then
        echo "-------------------------2 <sample_mulit_isp_stresstest> HDR mode switch test success (no-group)" >> /tmp/multi_isp_stresstest_result.log
        echo -e "--------------------------------------- <sample_mulit_isp_stresstest> hdr_mode_switch_test success (no-group)-------------------------------------------\n\n\n"
    else 
        echo "-------------------------2 <sample_mulit_isp_stresstest> HDR mode switch test failure (no-group)" >> /tmp/multi_isp_stresstest_result.log
        echo -e "--------------------------------------- <sample_mulit_isp_stresstest> hdr_mode_switch_test failure (no-group)-------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/multi_isp_stresstest_result.log

    #3: frameRate switch test
    echo -e "--------------------------------------- <sample_mulit_isp_stresstest> frameRate_switch_test start (no-group)-------------------------------------------\n"
    echo -e "<sample_mulit_isp_stresstest -a /oem/usr/share/iqfiles -c 2 -w 1920 -h 1080 --modeTestType 3 --modeTestLoop $test_loop --testFrameCount $frame_count --ispLaunchMode 0>\n"
    sample_mulit_isp_stresstest -a /oem/usr/share/iqfiles -c 2 -w 1920 -h 1080 --modeTestType 3 --modeTestLoop $test_loop --testFrameCount $frame_count --ispLaunchMode 0
    if [ $? -eq 0 ]; then
        echo "-------------------------3 <sample_mulit_isp_stresstest> isp frameRate switch test success (no-group)" >> /tmp/multi_isp_stresstest_result.log
        echo -e "--------------------------------------- <sample_mulit_isp_stresstest> frameRate_switch_test success (no-group)-------------------------------------------\n\n\n"
    else 
        echo "-------------------------3 <sample_mulit_isp_stresstest> isp frameRate switch test failure (no-group)" >> /tmp/multi_isp_stresstest_result.log
        echo -e "--------------------------------------- <sample_mulit_isp_stresstest> frameRate_switch_test failure (no-group)-------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/multi_isp_stresstest_result.log

    #4: LDCH mode test
    echo -e "--------------------------------------- <sample_mulit_isp_stresstest> LDCH mode test start (no-group)-------------------------------------------\n"
    echo -e "<sample_mulit_isp_stresstest -a /oem/usr/share/iqfiles -c 2 -w 1920 -h 1080 --modeTestType 5 --modeTestLoop $test_loop --testFrameCount $frame_count --ispLaunchMode 0>\n"
    sample_mulit_isp_stresstest -a /oem/usr/share/iqfiles -c 2 -w 1920 -h 1080 --modeTestType 5 --modeTestLoop $test_loop --testFrameCount $frame_count --ispLaunchMode 0
    if [ $? -eq 0 ]; then
        echo "-------------------------4 <sample_mulit_isp_stresstest> LDCH mode switch test success (no-group)" >> /tmp/multi_isp_stresstest_result.log
        echo -e "--------------------------------------- <sample_mulit_isp_stresstest> LDCH mode test success (no-group)-------------------------------------------\n\n\n"
    else 
        echo "-------------------------4 <sample_mulit_isp_stresstest> LDCH mode switch test failure (no-group)" >> /tmp/multi_isp_stresstest_result.log
        echo -e "--------------------------------------- <sample_mulit_isp_stresstest> LDCH mode test failure (no-group)-------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/multi_isp_stresstest_result.log


    #5: PN mode switch
    echo -e "--------------------------------------- <sample_mulit_isp_stresstest> PN mode switch test start (group)-------------------------------------------\n"
    echo -e "<sample_mulit_isp_stresstest -a /oem/usr/share/iqfiles -c 2 -w 1920 -h 1080 --modeTestType 1 --modeTestLoop $test_loop --testFrameCount $frame_count --ispLaunchMode 0>\n"
    sample_mulit_isp_stresstest -a /oem/usr/share/iqfiles -c 2 -w 1920 -h 1080 --modeTestType 1 --modeTestLoop $test_loop --testFrameCount $frame_count --ispLaunchMode 1
    if [ $? -eq 0 ]; then
        echo "-------------------------1 <sample_mulit_isp_stresstest> PN mode switch test success (group)" > /tmp/multi_isp_stresstest_result.log
        echo -e "--------------------------------------- <sample_mulit_isp_stresstest> PN mode switch test_result success (group)-------------------------------------------\n\n\n"
    else 
        echo "-------------------------1 <sample_mulit_isp_stresstest> PN mode switch test failure (group)" > /tmp/multi_isp_stresstest_result.log
        echo -e "--------------------------------------- <sample_mulit_isp_stresstest> PN mode switch test_result failure (group)-------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/multi_isp_stresstest_result.log

    #6: hdr mode switch test
    echo -e "--------------------------------------- <sample_mulit_isp_stresstest> hdr_mode_switch_test start (group)-------------------------------------------\n"
    echo -e "<sample_mulit_isp_stresstest -a /oem/usr/share/iqfiles -c 2 -w 1920 -h 1080 --modeTestType 2 --modeTestLoop $test_loop --testFrameCount $frame_count --ispLaunchMode 0>\n"
    sample_mulit_isp_stresstest -a /oem/usr/share/iqfiles -c 2 -w 1920 -h 1080 --modeTestType 2 --modeTestLoop $test_loop --testFrameCount $frame_count --ispLaunchMode 1
    if [ $? -eq 0 ]; then
        echo "-------------------------2 <sample_mulit_isp_stresstest> HDR mode switch test success (group)" >> /tmp/multi_isp_stresstest_result.log
        echo -e "--------------------------------------- <sample_mulit_isp_stresstest> hdr_mode_switch_test success (group)-------------------------------------------\n\n\n"
    else 
        echo "-------------------------2 <sample_mulit_isp_stresstest> HDR mode switch test failure (group)" >> /tmp/multi_isp_stresstest_result.log
        echo -e "--------------------------------------- <sample_mulit_isp_stresstest> hdr_mode_switch_test failure (group)-------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/multi_isp_stresstest_result.log

    #7: frameRate switch test
    echo -e "--------------------------------------- <sample_mulit_isp_stresstest> frameRate_switch_test start (group)-------------------------------------------\n"
    echo -e "<sample_mulit_isp_stresstest -a /oem/usr/share/iqfiles -c 2 -w 1920 -h 1080 --modeTestType 3 --modeTestLoop $test_loop --testFrameCount $frame_count --ispLaunchMode 0>\n"
    sample_mulit_isp_stresstest -a /oem/usr/share/iqfiles -c 2 -w 1920 -h 1080 --modeTestType 3 --modeTestLoop $test_loop --testFrameCount $frame_count --ispLaunchMode 1
    if [ $? -eq 0 ]; then
        echo "-------------------------3 <sample_mulit_isp_stresstest> isp frameRate switch test success (group)" >> /tmp/multi_isp_stresstest_result.log
        echo -e "--------------------------------------- <sample_mulit_isp_stresstest> frameRate_switch_test success (group)-------------------------------------------\n\n\n"
    else 
        echo "-------------------------3 <sample_mulit_isp_stresstest> isp frameRate switch test failure (group)" >> /tmp/multi_isp_stresstest_result.log
        echo -e "--------------------------------------- <sample_mulit_isp_stresstest> frameRate_switch_test failure (group)-------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/multi_isp_stresstest_result.log

    #8: LDCH mode test
    echo -e "--------------------------------------- <sample_mulit_isp_stresstest> LDCH mode test start (group)-------------------------------------------\n"
    echo -e "<sample_mulit_isp_stresstest -a /oem/usr/share/iqfiles -c 2 -w 1920 -h 1080 --modeTestType 5 --modeTestLoop $test_loop --testFrameCount $frame_count --ispLaunchMode 0>\n"
    sample_mulit_isp_stresstest -a /oem/usr/share/iqfiles -c 2 -w 1920 -h 1080 --modeTestType 5 --modeTestLoop $test_loop --testFrameCount $frame_count --ispLaunchMode 1
    if [ $? -eq 0 ]; then
        echo "-------------------------4 <sample_mulit_isp_stresstest> LDCH mode switch test success (group)" >> /tmp/multi_isp_stresstest_result.log
        echo -e "--------------------------------------- <sample_mulit_isp_stresstest> LDCH mode test success (group)-------------------------------------------\n\n\n"
    else 
        echo "-------------------------4 <sample_mulit_isp_stresstest> LDCH mode switch test failure (group)" >> /tmp/multi_isp_stresstest_result.log
        echo -e "--------------------------------------- <sample_mulit_isp_stresstest> LDCH mode test failure (group)-------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/multi_isp_stresstest_result.log

    #9: isp_deinit_init_test
    echo -e "--------------------------------------- <sample_mulit_isp_stresstest> isp_deinit_init_test start (no-group)-------------------------------------------\n"
    echo -e "<sample_mulit_isp_stresstest -a /oem/usr/share/iqfiles -c 2 -w 1920 -h 1080 --modeTestType 6 --modeTestLoop $test_loop --testFrameCount $frame_count --ispLaunchMode 0>\n"
    sample_mulit_isp_stresstest -a /oem/usr/share/iqfiles -c 2 -w 1920 -h 1080 --modeTestType 6 --modeTestLoop $test_loop --testFrameCount $frame_count --ispLaunchMode 0
    if [ $? -eq 0 ]; then
        echo "-------------------------4 <sample_mulit_isp_stresstest> isp_deinit_init_test success (no-group)" >> /tmp/multi_isp_stresstest_result.log
        echo -e "--------------------------------------- <sample_mulit_isp_stresstest> isp_deinit_init test success (no-group)-------------------------------------------\n\n\n"
    else 
        echo "-------------------------4 <sample_mulit_isp_stresstest> isp_deinit_init test failure (no-group)" >> /tmp/multi_isp_stresstest_result.log
        echo -e "--------------------------------------- <sample_mulit_isp_stresstest> isp_deinit_init test failure (no-group)-------------------------------------------\n\n\n"
        exit 0
    fi
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/multi_isp_stresstest_result.log

    #10: isp_deinit_init_test
    echo -e "--------------------------------------- <sample_mulit_isp_stresstest> isp_deinit_init_test start (group)-------------------------------------------\n"
    echo -e "<sample_mulit_isp_stresstest -a /oem/usr/share/iqfiles -c 2 -w 1920 -h 1080 --modeTestType 6 --modeTestLoop $test_loop --testFrameCount $frame_count --ispLaunchMode 0>\n"
    sample_mulit_isp_stresstest -a /oem/usr/share/iqfiles -c 2 -w 1920 -h 1080 --modeTestType 6 --modeTestLoop $test_loop --testFrameCount $frame_count --ispLaunchMode 1
    if [ $? -eq 0 ]; then
        echo "-------------------------4 <sample_mulit_isp_stresstest> isp_deinit_init_test success (group)" >> /tmp/multi_isp_stresstest_result.log
        echo -e "--------------------------------------- <sample_mulit_isp_stresstest> isp_deinit_init test success (group)-------------------------------------------\n\n\n"
    else 
        echo "-------------------------4 <sample_mulit_isp_stresstest> isp_deinit_init test failure (group)" >> /tmp/multi_isp_stresstest_result.log
        echo -e "--------------------------------------- <sample_mulit_isp_stresstest> isp_deinit_init test failure (group)-------------------------------------------\n\n\n"
        exit 0
    fi

    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/multi_isp_stresstest_result.log

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
echo "multi_isp_stresstest_result is:"
cat /tmp/multi_isp_stresstest_result.log
cat /tmp/testLog.txt

