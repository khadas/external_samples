#!/bin/sh

set -x

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

print_help()
{
    echo "example: <test_mod=on> $0 <test_result_path> <test_loop> <test_frame> <ordinary_stream_test_framecount> <sensor_width> <sensor_height>"
    echo "example: PN_MODE=on HDR=on FRAMERATE=on AIISP=on VPSS_CHN0_RESOLUTION=on VPSS_VENC_CHN0_RESOLUTION=on ENCODE_TYPE=on VPSS_RGN_INIT_DEINIT=on VENC_RGN_INIT_DEINIT=on RESTART=on AIISP_FORCE=on ./demo_aiisp_streastest.sh /tmp/test.log 100 10 450000 2688 1520"
    echo "mod: 0.ORDINARY 1.PN_MODE 2.HDR 3.FRAMERATE 4.AIISP 5.VPSS_CHN0_RESOLUTION 6.VPSS_VENC_CHN0_RESOLUTION 7.ENCODE_TYPE 8.VPSS_RGN_INIT_DEINIT 9.VENC_RGN_INIT_DEINIT 10.RESTART 11.AIISP_FORCE"
    echo -e "
          \$1 --------test_result_path: /tmp/xxxx.log (require argument)\n
          \$2 --------test_loop: 10000 (require argument)\n
          \$3 --------test_frame: 10 (require argument)\n
          \$4 --------ordinary_stream_test_framecount: 450000\n
          \$5 --------sensor_width: 2688(require argument\n
          \$6 --------sensor_height: 1520(require argument)\n"

}

test_result_path=$1
if [ "$1" = "help" ]; then
    print_help
    exit 1
elif [ ! -n "$1" ]; then
    echo "------ error!!! lack test_result_path, please input test_result_path"
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

#set ordinary_stream_test_framecount
ordinary_stream_test_framecount=$4
if [ ! -n "$4" ]; then
    echo "----------------- error !!!!, lack ordinary_stream_test_framecount setting, please input setting"
fi

#set sensor width
sensor_width=$5
if [ ! -n "$5" ]; then
    echo "----------------- error!!!!, lack sensor width, please input sensor width"
    print_help
    exit 1
fi

#set sensor height
sensor_height=$6
if [ ! -n "$6" ]; then
    echo "----------------- error!!!!, lack sensor height, please input sensor height"
    print_help
    exit 1
fi

__echo_test_cmd_msg()
{
	echo -e "$1" | tee -a $test_result_path
	if [ $? -ne 0 ]; then
		echo -e "$1"
	fi
}
test_case()
{

    if [ "$PN_MODE" = "on" ]; then
        #isp p/n mode switch
        __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> isp p/n mode switch test start -------------------------------------------\n"
        __echo_test_cmd_msg "<sample_demo_aiisp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/ -e h264cbr -b 4096 -i /userdata/160x96.bmp -I /userdata/192x96.bmp --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 1>\n"
        sample_demo_aiisp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/ -e h264cbr -b 4096 -i /userdata/160x96.bmp -I /userdata/192x96.bmp --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 1
        if [ $? -eq 0 ]; then
            echo "-------------------------<sample_demo_aiisp_stresstest> isp p/n mode switch test success" >> $test_result_path
            __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> isp p/n mode switch test success -------------------------------------------\n\n\n"
        else
            echo "-------------------------<sample_demo_aiisp_stresstest> isp p/n mode switch test failure" >> $test_result_path
            __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> isp p/n mode switch test failure -------------------------------------------\n\n\n"
            exit 1
        fi
		__chk_cma_free
    fi

    if [ "$HDR" = "on" ]; then
        #isp hdr mode switch test
        __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> isp hdr mode switch switch test start -------------------------------------------\n"
        __echo_test_cmd_msg "<sample_demo_aiisp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/  -i /userdata/160x96.bmp -I /userdata/192x96.bmp  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 2 >\n"
        sample_demo_aiisp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/  -i /userdata/160x96.bmp -I /userdata/192x96.bmp  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 2 
        if [ $? -eq 0 ]; then
            echo "-------------------------<sample_demo_aiisp_stresstest> isp hdr mode switch test success" >> $test_result_path
            __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> isp hdr mode switch switch test success -------------------------------------------\n\n\n"
        else
            echo "-------------------------<sample_demo_aiisp_stresstest> isp hdr mode switch test failure" >> $test_result_path
            __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> isp hdr mode switch switch test failure -------------------------------------------\n\n\n"
            exit 1
        fi
		__chk_cma_free
    fi

    if [ "$FRAMERATE" = "on" ]; then
        #isp framerate switch test
        __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> isp framerate switch test start -------------------------------------------\n"
        __echo_test_cmd_msg "sample_demo_aiisp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/  -i /userdata/160x96.bmp -I /userdata/192x96.bmp  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 3 \n>"
        sample_demo_aiisp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/  -i /userdata/160x96.bmp -I /userdata/192x96.bmp  --test_frame_count $frame_count --mode_test_loop  $test_loop --mode_test_type 3 
        if [ $? -eq 0 ]; then
            echo "-------------------------<sample_demo_aiisp_stresstest> isp framerate switch test success" >> $test_result_path
            __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> isp framerate switch test success -------------------------------------------\n\n\n"
        else
            echo "-------------------------<sample_demo_aiisp_stresstest> isp framerate switch test failure" >> $test_result_path
            __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> isp framerate switch test failure -------------------------------------------\n\n\n"
            exit 1
        fi
		__chk_cma_free
    fi

    if [ "$AIISP" = "on" ]; then
        #aiisp mode switch test
        __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> aiisp mode switch test start -------------------------------------------\n"
        __echo_test_cmd_msg "sample_demo_aiisp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/  -i /userdata/160x96.bmp -I /userdata/192x96.bmp  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 4 \n>"
        sample_demo_aiisp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/  -i /userdata/160x96.bmp -I /userdata/192x96.bmp  --test_frame_count $frame_count --mode_test_loop  $test_loop --mode_test_type 4
        if [ $? -eq 0 ]; then
            echo "-------------------------<sample_demo_aiisp_stresstest> aiisp mode switch test success" >> $test_result_path
            __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> aiisp mode switch test success -------------------------------------------\n\n\n"
        else
            echo "-------------------------<sample_demo_aiisp_stresstest> aiisp mode switch test failure" >> $test_result_path
            __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> aiisp mode switch test failure -------------------------------------------\n\n\n"
            exit 1
        fi
		__chk_cma_free
    fi


    if [ "$VPSS_CHN0_RESOLUTION" = "on" ]; then
        #venc_chn0_resolution switch test
        __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> vpss_chn0_resolution_switch_test  start -------------------------------------------\n"
        __echo_test_cmd_msg "sample_demo_aiisp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/ -i /userdata/160x96.bmp -I /userdata/192x96.bmp --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 5 >\n"
        sample_demo_aiisp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/ -i /userdata/160x96.bmp -I /userdata/192x96.bmp  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 5 
        if [ $? -eq 0 ]; then
            echo "-------------------------<sample_demo_aiisp_stresstest> vpss_chn0_resolution switch test success" >> $test_result_path
            __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> vpss_chn0_resolution switch test success -------------------------------------------\n\n\n"
        else
            echo "-------------------------<sample_demo_aiisp_stresstest> vpss_chn0_resolution switch test failure" >> $test_result_path
            __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> vpss_chn0_resolution switch test failure -------------------------------------------\n\n\n"
            exit 1
        fi
		__chk_cma_free
    fi

    if [ "$VPSS_VENC_CHN0_RESOLUTION" = "on" ]; then
        #vpss_venc_chn0_resolution switch test
        __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> vpss_venc_chn0_resolution switch test start -------------------------------------------\n"
        __echo_test_cmd_msg "sample_demo_aiisp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/  -i /userdata/160x96.bmp -I /userdata/192x96.bmp  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 6 >\n"
        sample_demo_aiisp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/  -i /userdata/160x96.bmp -I /userdata/192x96.bmp --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 6 
        if [ $? -eq 0 ]; then
            echo "-------------------------<sample_demo_aiisp_stresstest> vpss_venc_chn0_resolution switch test success" >> $test_result_path
            __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> vpss_venc_chn0_resolution switch test success -------------------------------------------\n\n\n"
        else
            echo "-------------------------<sample_demo_aiisp_stresstest> vpss_venc_chn0_resolution switch test failure" >> $test_result_path
            __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> vpss_venc_chn0_resolution switch test failure -------------------------------------------\n\n\n"
            exit 1
        fi
		__chk_cma_free
    fi

        if [ "$ENCODE_TYPE" = "on" ]; then
        # encode type switch test
        __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> encode type switch test start -------------------------------------------\n"
        __echo_test_cmd_msg "sample_demo_aiisp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/  -i /userdata/160x96.bmp -I /userdata/192x96.bmp  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 7 >\n"
        sample_demo_aiisp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/  -i /userdata/160x96.bmp -I /userdata/192x96.bmp  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 7
        if [ $? -eq 0 ]; then
            echo "-------------------------<sample_demo_aiisp_stresstest> venc encode type switch test success" >> $test_result_path
            __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> encode type switch test success -------------------------------------------\n\n\n"
        else
            echo "-------------------------<sample_demo_aiisp_stresstest> venc encode type switch test failure" >> $test_result_path
            __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> encode type switch test failure -------------------------------------------\n\n\n"
            exit 1
        fi
		__chk_cma_free
    fi

    if [ "$VPSS_RGN_INIT_DEINIT" = "on" ]; then
        #rgn init and deinit test
        __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> vpss rgn init and deinit test start -------------------------------------------\n"
        __echo_test_cmd_msg "sample_demo_aiisp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/ --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 8 >\n"
        sample_demo_aiisp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 8 
        if [ $? -eq 0 ]; then
            echo "-------------------------<sample_demo_aiisp_stresstest> vpss rgn init and deinit test success" >> $test_result_path
            __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> vpss rgn init and deinit test success -------------------------------------------\n\n\n"
        else
            echo "-------------------------<sample_demo_aiisp_stresstest> vpss rgn init and deinit test failure" >> $test_result_path
            __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> vpss rgn init and deinit test failure -------------------------------------------\n\n\n"
            exit 1
        fi
		__chk_cma_free
    fi

    if [ "$VENC_RGN_INIT_DEINIT" = "on" ]; then
        #rgn init and deinit test
        __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> venc rgn init and deinit test start -------------------------------------------\n"
        __echo_test_cmd_msg "sample_demo_aiisp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/  -i /userdata/160x96.bmp -I /userdata/192x96.bmp  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 9 >\n"
        sample_demo_aiisp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/  -i /userdata/160x96.bmp -I /userdata/192x96.bmp  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 9 
        if [ $? -eq 0 ]; then
            echo "-------------------------<sample_demo_aiisp_stresstest> venc rgn init and deinit test success" >> $test_result_path
            __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> venc rgn init and deinit test success -------------------------------------------\n\n\n"
        else
            echo "-------------------------<sample_demo_aiisp_stresstest> venc rgn init and deinit test failure" >> $test_result_path
            __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> venc rgn init and deinit test failure -------------------------------------------\n\n\n"
            exit 1
        fi
		__chk_cma_free
    fi
    if [ "$ORDINARY" = "on" ]; then
        #ordinary stream test
        __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> ordinary stream test start -------------------------------------------\n"
        __echo_test_cmd_msg "sample_demo_aiisp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/ -l $ordinary_stream_test_framecount -i /userdata/160x96.bmp -I /userdata/192x96.bmp  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 0 >\n"
        sample_demo_aiisp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/ -l $ordinary_stream_test_framecount -i /userdata/160x96.bmp -I /userdata/192x96.bmp  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 0 
        if [ $? -eq 0 ]; then
            echo "-------------------------<sample_demo_aiisp_stresstest> ordinary stream test success" >> $test_result_path
            __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> ordinary stream test success -------------------------------------------\n\n\n"
        else
            echo "-------------------------<sample_demo_aiisp_stresstest> ordinary stream test failure" >> $test_result_path
            __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> ordinary stream test failure -------------------------------------------\n\n\n"
            exit 1
        fi
		__chk_cma_free
    fi

    if [ "$RESTART" = "on" ]; then
        #media_deinit_init test
        __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> media_deinit_init test start -------------------------------------------\n"
        __echo_test_cmd_msg "sample_demo_aiisp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/ --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 10  >\n"
        sample_demo_aiisp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/ --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 10  
        if [ $? -eq 0 ]; then
            echo "-------------------------<sample_demo_aiisp_stresstest> media_deinit_init test success" >> $test_result_path
            __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> media_deinit_init test success -------------------------------------------\n\n\n"
        else
            echo "-------------------------<sample_demo_aiisp_stresstest> media_deinit_init test failure" >> $test_result_path
            __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> media_deinit_init test failure -------------------------------------------\n\n\n"
            exit 1
        fi
		__chk_cma_free
    fi

    if [ "$AIISP_FORCE" = "on" ]; then
        #aiisp force switch test
        __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> aiisp force switch test start -------------------------------------------\n"
        __echo_test_cmd_msg "sample_demo_aiisp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/  -i /userdata/160x96.bmp -I /userdata/192x96.bmp  --test_frame_count $frame_count --mode_test_loop $test_loop --mode_test_type 11 \n>"
        sample_demo_aiisp_stresstest -w $sensor_width -h $sensor_height -a /etc/iqfiles/  -i /userdata/160x96.bmp -I /userdata/192x96.bmp  --test_frame_count $frame_count --mode_test_loop  $test_loop --mode_test_type 11
        if [ $? -eq 0 ]; then
            echo "-------------------------<sample_demo_aiisp_stresstest> aiisp force switch test success" >> $test_result_path
            __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> aiisp force switch test success -------------------------------------------\n\n\n"
        else
            echo "-------------------------<sample_demo_aiisp_stresstest> aiisp force switch test failure" >> $test_result_path
            __echo_test_cmd_msg "--------------------------------------- <sample_demo_aiisp_stresstest> aiisp force switch test failure -------------------------------------------\n\n\n"
            exit 1
        fi
		__chk_cma_free
    fi

    sleep 1
    echo 3 > /proc/sys/vm/drop_caches
    cat /proc/meminfo | grep MemAvailable >> $test_result_path

}

test_case

exit 0
