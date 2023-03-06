#!/bin/sh

#set test loop
test_loop=10000

#set frame count for every loop
frame_count=10

test_case()
{
    #1. avs_deinit_ubind_test
    echo -e "--------------------------------------- <sample_avs_stresstest> avs_deinit_ubind_test start -------------------------------------------\n"
    echo -e "<sample_avs_stresstest --vi_size 1920x1080 --avs_size 3840x1080 -a /etc/iqfiles/ --mode_test_type 1 --mode_test_loop $test_loop --test_frame_count $frame_count>\n"
    sample_avs_stresstest --vi_size 1920x1080 --avs_size 3840x1080 -a /etc/iqfiles/ --mode_test_type 1 --mode_test_loop $test_loop --test_frame_count $frame_count
    if [ $? -eq 0 ]; then
        echo "-------------------------1 <sample_avs_stresstest> avs_deinit_ubind_test success" > /tmp/avs_stresstest_result.log
        echo -e "--------------------------------------- <sample_avs_stresstest> avs_deinit_ubind_test success -------------------------------------------\n\n\n"
    else 
        echo "-------------------------1 <sample_avs_stresstest> avs_deinit_ubind_test failure" > /tmp/avs_stresstest_result.log
        echo -e "--------------------------------------- <sample_avs_stresstest> avs_deinit_ubind_test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable > /tmp/avs_stresstest_result.log


    #2. avs_resolution_test
    echo -e "--------------------------------------- <sample_avs_stresstest> avs_resolution_test start -------------------------------------------\n"
    echo -e "<sample_avs_stresstest --vi_size 1920x1080 --avs_size 3840x1080 -a /etc/iqfiles/ --mode_test_type 2 --mode_test_loop $test_loop --test_frame_count $frame_count>\n"
    sample_avs_stresstest --vi_size 1920x1080 --avs_size 3840x1080 -a /etc/iqfiles/ --mode_test_type 2 --mode_test_loop $test_loop --test_frame_count $frame_count
    if [ $? -eq 0 ]; then
        echo "-------------------------2 <sample_avs_stresstest> avs_resolution_test success" >> /tmp/avs_stresstest_result.log
        echo -e "--------------------------------------- <sample_avs_stresstest> avs_resolution_test success -------------------------------------------\n\n\n"
    else
        echo "-------------------------2 <sample_avs_stresstest> avs_resolution_test failure" >> /tmp/avs_stresstest_result.log
        echo -e "--------------------------------------- <sample_avs_stresstest> avs_resolution_test failure -------------------------------------------\n\n\n"
        exit 0
    fi
    
    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> /tmp/avs_stresstest_result.log

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
echo "avs_stresstest_result is:"
cat /tmp/avs_stresstest_result.log
cat /tmp/testLog.txt

