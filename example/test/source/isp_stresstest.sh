#!/bin/sh

set -x
__echo_test_cmd_msg()
{
	echo -e "$1" | tee -a $test_result_path
	if [ $? -ne 0 ]; then
		echo -e "$1"
	fi
}
print_help()
{
    echo "example: <test_mod=on> $0 <test_result_path> <test_loop> <test_frame> <vi_frame_switch_test_loop> <iq_file_path>"
    echo "mod: 1.PN_MODE 2.HDR 3.FRAMERATE 4.LDCH 5.IQFILE 6.ISP_RESTART"
    echo -e "
          \$1 --------test_result_path: /tmp/stresstest.log\n
          \$2 --------test_loop: 10000\n
          \$3 --------test_frame: 10\n
          \$4 --------vi_frame_switch_test_loop: 5000\n
          \$5 --------iq_file_path\n"
}

test_result_path=$1
if [ "$1" = "help" ]; then
    print_help
    exit 1
elif [ ! -n "$1" ]; then
    echo "----------------- error!!! lack test_result_path, please input test_result_path"
    print_help
    exit 1
else
    echo " the test_result_path your input is: $1"
fi

#set test loop
test_loop=$2
if [ ! -n "$2" ]; then
    echo "----------------- error!!!, lack test_loop, please input test loop"
    print_help
    exit 1
fi

#set frame count for every loop
frame_count=$3
if [ ! -n "$3" ]; then
    echo "----------------- error!!!!, lack frame_count, please input test frame"
    print_help
    exit 1
fi

vi_framerate_switch_loop=$4
if [ ! -n "$4" ]; then
    echo "----------------- warning!!!!, lack vi_framerate_switch_loop"
fi

iqfilePath=$5
if [ ! -n "$5" ]; then
    echo "----------------- warning!!!!, lack iqfilePath"
fi

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

test_case()
{
    if [ "$PN_MODE" = "on" ]; then
        #1 PN mode switch
        __echo_test_cmd_msg "--------------------------------------- <sample_isp_stresstest> PN mode switch test start -------------------------------------------\n"
        __echo_test_cmd_msg "<sample_isp_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --chn_id 1 --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 1 >\n"
        sample_isp_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --chn_id 1 --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 1
        if [ $? -eq 0 ]; then
            __echo_test_cmd_msg "---------------------------------------1 <sample_isp_stresstest> PN mode switch test_result success -------------------------------------------\n\n\n"
        else
            __echo_test_cmd_msg "---------------------------------------1 <sample_isp_stresstest> PN mode switch test_result failure -------------------------------------------\n\n\n"
            exit 1
        fi
    fi

	__chk_cma_free

    if [ "$HDR" = "on" ]; then
        #2 HDR mode test
        __echo_test_cmd_msg "--------------------------------------- <sample_isp_stresstest> HDR mode test start -------------------------------------------\n"
        __echo_test_cmd_msg "<sample_isp_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --chn_id 1 --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 2 >\n"
        sample_isp_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --chn_id 1 --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 2
        if [ $? -eq 0 ]; then
            __echo_test_cmd_msg "---------------------------------------2 <sample_isp_stresstest> HDR mode test success -------------------------------------------\n\n\n"
        else
            __echo_test_cmd_msg "---------------------------------------2 <sample_isp_stresstest> HDR mode test failure -------------------------------------------\n\n\n"
            exit 1
        fi
    fi

	__chk_cma_free

    if [ "$FRAMERATE" = "on" ]; then
        #3 framerate switch test
        __echo_test_cmd_msg "--------------------------------------- <sample_isp_stresstest> frameRate_switch_test start -------------------------------------------\n"
        __echo_test_cmd_msg "<sample_isp_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --chn_id 1 --test_frame_count $frame_count --mode_test_loop $vi_framerate_switch_loop --mode_test_type 3 >\n"
        sample_isp_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --chn_id 1 --test_frame_count $frame_count --mode_test_loop $vi_framerate_switch_loop --mode_test_type 3
        if [ $? -eq 0 ]; then
            __echo_test_cmd_msg "---------------------------------------3 <sample_isp_stresstest> frameRate_switch_test success -------------------------------------------\n\n\n"
        else
            __echo_test_cmd_msg "---------------------------------------3 <sample_isp_stresstest> frameRate_switch_test failure -------------------------------------------\n\n\n"
            exit 1
        fi
    fi

	__chk_cma_free

    if [ "$LDCH" = "on" ]; then
        #4 LDCH mode test
        __echo_test_cmd_msg "--------------------------------------- <sample_isp_stresstest> LDCH mode test start -------------------------------------------\n"
        __echo_test_cmd_msg "<sample_isp_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --chn_id 1 --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 4 >\n"
        sample_isp_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --chn_id 1 --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 4
        if [ $? -eq 0 ]; then
            __echo_test_cmd_msg "---------------------------------------4 <sample_isp_stresstest> LDCH mode test success -------------------------------------------\n\n\n"
        else
            __echo_test_cmd_msg "---------------------------------------4 <sample_isp_stresstest> LDCH mode test failure -------------------------------------------\n\n\n"
            exit 1
        fi
    fi


	__chk_cma_free

    if [ "$IQFILE" = "on" ]; then
        #5 iqfile switch test
        __echo_test_cmd_msg "--------------------------------------- <sample_isp_stresstest> iqfile switch test start -------------------------------------------\n"
        __echo_test_cmd_msg "<sample_isp_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --chn_id 1 --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 5 --iqfilePath $iqfilePath>\n"
        sample_isp_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --chn_id 1 --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 5 --iqfilePath $iqfilePath
        if [ $? -eq 0 ]; then
            __echo_test_cmd_msg "---------------------------------------5 <sample_isp_stresstest> iqfile switch test success -------------------------------------------\n\n\n"
        else
            __echo_test_cmd_msg "---------------------------------------5 <sample_isp_stresstest> iqfile switch test failure -------------------------------------------\n\n\n"
            exit 1
        fi
    fi

	__chk_cma_free

    if [ "$ISP_RESTART" = "on" ]; then

        #6 isp_deinit_init test
        __echo_test_cmd_msg "--------------------------------------- <sample_isp_stresstest> isp_deinit_init test start -------------------------------------------\n"
        __echo_test_cmd_msg "<sample_isp_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --chn_id 1 --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 6 >\n"
        sample_isp_stresstest -w 1920 -h 1080 -a /etc/iqfiles/ --chn_id 1 --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 6
        if [ $? -eq 0 ]; then
            __echo_test_cmd_msg "---------------------------------------6 <sample_isp_stresstest> isp_deinit_init test success -------------------------------------------\n\n\n"
        else
            __echo_test_cmd_msg "---------------------------------------6 <sample_isp_stresstest> isp_deinit_init test failure -------------------------------------------\n\n\n"
            exit 1
        fi
    fi

	__chk_cma_free

    sleep 3
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> $test_result_path

}

test_case

exit 0
