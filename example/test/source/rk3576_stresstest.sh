#!/bin/sh

set -x
#test loop
test_loop=10000

#test frame
test_frame=50

#test result path
test_result_path=/tmp/rk3576_test_result.log

#vi framerate switch loop
vi_framerate_switch_loop=5000

#rv1103 set wrap
ifEnableWrap=0

# samartP mode
ifEnableSmartP=0

#ordinary_stream_test_framecount
ordinary_stream_test_framecount=450000

# get script path
script_file_path=$(dirname $0)

if [ "$1" = "os04a10" ]; then
    echo "-----------Now your sensor model: os04a10"
    sensor_width=2688
    sensor_height=1520
    iqfilePath=/etc/iqfiles/os04a10_CMK-OT1607-FV1_M12-40IRC-4MP-F16.xml
    echo "sensor_width: $sensor_width, sensor_height: $sensor_height, iqfilePath: $iqfilePath"
elif [ "$1" = "imx464" ]; then
    echo "-----------Now your sensor model: imx464"
    sensor_width=2560
    sensor_height=1440
    iqfilePath=/etc/iqfiles/imx464_CMK-OT1980-PX1_SHG102.json
    echo "sensor_width: $sensor_width, sensor_height: $sensor_height, iqfilePath: $iqfilePath"
elif [ "$1" = "imx415" ]; then
    echo "-----------Now your sensor model: imx415"
    sensor_width=3840
    sensor_height=2160
    iqfilePath=/etc/iqfiles/imx415_YT10092_IR0147-36IRC-8M-F20.xml
    echo "sensor_width: $sensor_width, sensor_height: $sensor_height, iqfilePath: $iqfilePath"
else
    echo "------- error, must input sensor model: os04a10, imx464, imx415 ----"
    echo "example: $0 os04a10"
    exit 0
fi

__echo_test_cmd_msg()
{
	echo -e "$1" | tee -a $test_result_path
	if [ $? -ne 0 ]; then
		echo -e "$1"
	fi
}

__chk_cma_free()
{
	local f
	if [ ! -f "/proc/rk_dma_heap/alloc_bitmap" ];then
		echo "[$0] not found /proc/rk_dma_heap/alloc_bitmap, ignore"
		return
	fi
	f=`head  /proc/rk_dma_heap/alloc_bitmap |grep Used|awk '{print $2}'`
	if [ $f -gt 12 ];then
		echo "[$0] free cma error"
		exit 2
	fi
}

test_cmd()
{
	if [ -z "$*" ];then
		echo "not found cmd, return"
		return
	fi
	__echo_test_cmd_msg "TEST    [$*]"
	eval $*
	__chk_cma_free
	if [ $? -eq 0 ]; then
		__echo_test_cmd_msg "SUCCESS [$*]"
	else
		__echo_test_cmd_msg "FAILURE [$*]"
		exit 1
	fi
}

isp_stresstest()
{
    # ./script test_type if_open test_loop test_frame iq_file
    # $1 --------test_result_path
    # $2 --------test_loop
    # $3 --------test_frame
    # $4 --------vi_frame_switch_test_loop
    # $5 --------iq_file

    echo "-----------------enter isp stresstest-----------------" >> $test_result_path

    #1 PN mode switch
    test_cmd sample_isp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/ --chn_id 1 --test_frame_count $test_frame --mode_test_loop $test_loop --mode_test_type 1

    #2 HDR mode test
    test_cmd sample_isp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/ --chn_id 1 --test_frame_count $test_frame --mode_test_loop $test_loop --mode_test_type 2

    #3 framerate switch test
    test_cmd sample_isp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/ --chn_id 1 --test_frame_count $test_frame --mode_test_loop $vi_framerate_switch_loop --mode_test_type 3

    #4 LDCH mode test
    test_cmd sample_isp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/ --chn_id 1 --test_frame_count $test_frame --mode_test_loop $test_loop --mode_test_type 4

    #6 isp_deinit_init test
    test_cmd sample_isp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/ --chn_id 1 --test_frame_count $test_frame --mode_test_loop $test_loop --mode_test_type 6

    #6 aiisp_deinit_init test
    test_cmd sample_isp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/ --chn_id 1 --test_frame_count $test_frame --mode_test_loop $test_loop --mode_test_type 7

    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> $test_result_path

    echo "-----------------exit isp stresstest-----------------" >> $test_result_path
}

venc_stresstest()
{
    #example: $0 <test_result_path> <test_loop> <test_frame> <ifEnableWrap>"
    #      $1 --------test_result_path
    #      $2 --------test_loop
    #      $3 --------test_frame
    #      $4 --------ifEnableWrap

    echo "-----------------enter venc stresstest-----------------" >> $test_result_path

    #venc resolution switch test
    test_cmd sample_venc_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/ --wrap 0 --mode_test_type 1 --mode_test_loop $test_loop --test_frame_count $test_frame

    # encode type switch test
    test_cmd sample_venc_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/ --wrap 0 --mode_test_type 2 --mode_test_loop $test_loop --test_frame_count $test_frame

    #smartp mode switch test
    test_cmd sample_venc_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/ --wrap 0 --mode_test_type 3 --mode_test_loop $test_loop --test_frame_count $test_frame

    #SVC mode switch test
    test_cmd sample_venc_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/ --wrap 0 --mode_test_type 4 --mode_test_loop $test_loop --test_frame_count $test_frame

    #motion deblur switch test
    test_cmd sample_venc_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/ --wrap 0 --mode_test_type 5 --mode_test_loop $test_loop --test_frame_count $test_frame

    #force IDR switch test
    test_cmd sample_venc_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/ --wrap 0 --mode_test_type 6 --mode_test_loop $test_loop --test_frame_count $test_frame

    #venc chn rotation switch test
	test_cmd sample_venc_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/ --wrap 0 --mode_test_type 7 --mode_test_loop $test_loop --test_frame_count $test_frame

    sleep 1
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> $test_result_path

    echo "-----------------exit venc stresstest-----------------" >> $test_result_path
}

rgn_stresstest()
{
    #example: $0 <test_type> <test_result_path> <test_loop> <test_frame> <ifEnableWrap>"
    #      $1 --------test_result_path
    #      $2 --------test_loop
    #      $3 --------test_frame
    #      $4 --------ifEnableWrap

    echo "-----------------enter rgn stresstest-----------------" >> $test_result_path

    #rgn detach attach test
    test_cmd sample_rgn_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/ -l -1 --test_frame_count $test_frame --mode_test_loop $test_loop --mode_test_type 1

    #rgn detach attach test for hardware vpss
    test_cmd sample_rgn_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/ -l -1 --test_frame_count $test_frame --mode_test_loop $test_loop --mode_test_type 2

    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> $test_result_path

    echo "-----------------exit rgn stresstest-----------------" >> $test_result_path
}

vpss_stresstest()
{
    # echo "example: <>$0 <test_result_path> <test_loop> <test_frame>"
    # test_mod: VPSS_RESTART RESOLUTION
    # echo -e "
    #       \$1 --------test_result_path\n
    #       \$2 --------test_loop\n
    #       \$3 --------test_frame\n"

    echo "-----------------enter vpss stresstest-----------------" >> $test_result_path

    #1. vpss_deinit_ubind_test
    test_cmd sample_vpss_stresstest --vi_size 2560x1440 --vpss_size 2560x1440 -a /etc/iqfiles/ --mode_test_type 1 --mode_test_loop $test_loop --test_frame_count $test_frame

    #2. vpss_resolution_test
    test_cmd sample_vpss_stresstest --vi_size 2560x1440 --vpss_size 2560x1440 -a /etc/iqfiles/ --mode_test_type 2 --mode_test_loop $test_loop --test_frame_count $test_frame

    #3. hardware vpss_deinit_ubind_test
    test_cmd sample_vpss_stresstest --vi_size 2560x1440 --vpss_size 2560x1440 -a /etc/iqfiles/ --mode_test_type 3 --mode_test_loop $test_loop --test_frame_count $test_frame

    #4. hardware vpss_resolution_test
    test_cmd sample_vpss_stresstest --vi_size 2560x1440 --vpss_size 2560x1440 -a /etc/iqfiles/ --mode_test_type 4 --mode_test_loop $test_loop --test_frame_count $test_frame

    echo "-----------------exit vpss stresstest-----------------" >> $test_result_path
}

demo_vi_venc_stresstest()
{
    # "example: $0 <test_result_path> <test_loop> <test_frame> <ifEnableWrap> <ifEnableSmartP> <ordinary_stream_test_framecount> <vi_framerate_switch_loop> <sensor_width> <sensor_height>"
    #   mod: PN_MODE HDR FRAMERATE LDCH RESOLUTION ENCODE_TYPE SMART_P SVC MOTION IDR ROTATION DETACH_ATTACH ORDINARY RESOLUTION_RV1126
    #   \$1 --------test_result_path: /tmp/xxxx.log\n
    #   \$2 --------test_loop: 10000\n
    #   \$3 --------test_frame: 10\n
    #   \$4 --------ifEnableWrap: 0:close, 1:open\n
    #   \$5 --------ifEnableSmartP: 0:close, 1:open\n
    #   \$6 --------ordinary_stream_test_framecount: 450000\n
    #   \$7 --------vi_framerate_switch_loop\n
    #   \$8 --------sensor_width\n
    #   \$9 --------sensor_height\n"

    echo "-----------------enter demo_vi_venc stresstest-----------------" >> $test_result_path

    echo "-----------------exit demo_vi_venc stresstest-----------------" >> $test_result_path
}

killall rkipc
while true
do
    ps|grep rkipc |grep -v grep
    if [ $? -ne 0 ]; then
        echo "rkipc exit"
        break
    else
        sleep 1
        echo "rkipc active"
    fi
done

echo "start rk3576 stresstest" > $test_result_path
echo "start record rk3576 meminfo" > /tmp/testLog.txt

#1.isp stresstest
test_mod="PN_MODE=on HDR=on FRAMERATE=on LDCH=on IQFILE=on ISP_RESTART=on"
isp_stresstest "$test_mod"

#2.venc stresstest
test_mod="ENCODE_TYPE=on SMART_P=off IDR=on"
venc_stresstest "$test_mod"

#3.rgn stresstest
test_mod="DETACH_ATTACH=on"
rgn_stresstest "$test_mod"

#4.vpss stresstest
test_mod="RESTART=on RESOLUTION=on"
vpss_stresstest "$test_mod"

#5.demo vi venc stresstest
test_mod="PN_MODE=on HDR=on FRAMERATE=on LDCH=on ENCODE_TYPE=on SMART_P=off IDR=on DETACH_ATTACH=on ORDINARY=on RESOLUTION_RV1126=on RESTART=on"
demo_vi_venc_stresstest "$test_mod"

#print test result
cat $test_result_path
cat /tmp/testLog.txt
