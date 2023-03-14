#!/bin/sh

#test loop
test_loop=10000

#test frame
test_frame=10

#test result path
test_result_path=/tmp/rv1103_test_result.log

#vi framerate switch loop
vi_framerate_switch_loop=5000

#rv1103 set wrap
ifEnableWrap=1

# samartP mode
ifEnableSmartP=0

#ordinary_stream_test_framecount
ordinary_stream_test_framecount=450000

camera_model=$1
if [ "$camera_model" = "sc3336" ]; then
        echo "-----------Now your sensor model: sc3336"

    elif [ "$camera_model" = "sc4336" ]; then
        echo "-----------Now your sensor model: sc4336"

    elif [ "$camera_model" = "sc530ai" ]; then
        echo "-----------Now your sensor model: sc530ai"

    else
        echo "------- error, must input sensor model: sc3336, sc4336, sc530ai ----"
        echo "example: $0 sc3336"
        exit 0
fi

isp_stresstest()
{
    # ./script test_type if_open test_loop test_frame iq_file
    # $1 --------test_result_path
    # $2 --------test_loop
    # $3 --------test_frame
    # $4 --------vi_frame_switch_test_loop
    # $5 --------iq_file

    echo "enter isp stresstest" >> $test_result_path

    eval $1 ./isp_stresstest.sh $test_result_path $test_loop $test_frame $vi_framerate_switch_loop $iqfilePath
    if [ $? != 0 ]; then
        echo "$1 ./isp_stresstest.sh $test_result_path $test_loop $test_frame $vi_framerate_switch_loop $iqfilePath   -----test failure"
        exit 1
    fi

}

venc_stresstest()
{
    #example: $0 <test_result_path> <test_loop> <test_frame> <ifEnableWrap>"
    #      $1 --------test_result_path
    #      $2 --------test_loop
    #      $3 --------test_frame
    #      $4 --------ifEnableWrap

    echo "enter venc stresstest" >> $test_result_path

    eval $1 ./venc_stresstest.sh $test_result_path $test_loop $test_frame $ifEnableWrap
     if [ $? != 0 ]; then
        echo "$1 ./venc_stresstest.sh $test_result_path $test_loop $test_frame $ifEnableWrap ----------test failure"
        exit 1
    fi

}

rgn_stresstest()
{
    #example: $0 <test_type> <test_result_path> <test_loop> <test_frame> <ifEnableWrap>"
    #      $2 --------test_result_path
    #      $3 --------test_loop
    #      $4 --------test_frame
    #      $5 --------ifEnableWrap

    echo "enter rgn stresstest" >> $test_result_path

    eval $1 ./rgn_stresstest.sh $test_result_path $test_loop $test_frame $ifEnableWrap
     if [ $? != 0 ]; then
        echo "$1 ./rgn_stresstest.sh $test_result_path $test_loop $test_frame $ifEnableWrap ----------test failure"
        exit 1
    fi
}

vpss_stresstest()
{
    # echo "example: <>$0 <test_result_path> <test_loop> <test_frame>"
    # test_mod: VPSS_RESTART RESOLUTION
    # echo -e "
    #       \$1 --------test_result_path\n
    #       \$2 --------test_loop\n
    #       \$3 --------test_frame\n"

    echo "enter vpss stresstest" >> $test_result_path

    eval $1 ./vpss_stresstest.sh $test_result_path $test_loop $test_frame
     if [ $? != 0 ]; then
        echo "$1 ./vpss_stresstest.sh $test_result_path $test_loop $test_frame----------test failure"
        exit 1
    fi

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

    echo "enter demo_vi_venc_stresstest stresstest" >> $test_result_path

    eval $1 ./demo_vi_venc_stresstest.sh $test_result_path $test_loop $test_frame $ifEnableWrap $ifEnableSmartP $ordinary_stream_test_framecount $vi_framerate_switch_loop $sensor_width $sensor_height
    if [ $? != 0 ]; then
        echo -e "$1 ./demo_vi_venc_stresstest.sh $test_result_path $test_loop $test_frame $ifEnableWrap $ifEnableSmartP $ordinary_stream_test_framecount  $vi_framerate_switch_loop $sensor_width $sensor_height ----------test failure"
        exit 1
    fi

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

echo "start rv1126 stresstest" > $test_result_path
echo "start record rv1126 meminfo" > /tmp/testLog.txt

#1.isp stresstest
if [[ "$camera_model" = "sc3336" ]] || [[ "$camera_model" = "sc4336" ]]; then
    test_mod="PN_MODE=on HDR=off FRAMERATE=on LDCH=on ISP_RESTART=on"
elif [ "$camera_model" = "sc530ai" ]; then
    test_mod="PN_MODE=on HDR=on FRAMERATE=on LDCH=on ISP_RESTART=on"
else
    echo " the camera model: $camera_model is't support, exit !!!"
    exit 1
fi
isp_stresstest "$test_mod"

#2.venc stresstest
test_mod="RESOLUTION=on ENCODE_TYPE=on SMART_P=on SVC=on MOTION=on IDR=on"
venc_stresstest "$test_mod"

#3.rgn stresstest
test_mod="DETACH_ATTACH=on"
rgn_stresstest "$test_mod"

#4.vpss stresstest
test_mod="RESTART=on RESOLUTION=on"
vpss_stresstest "$test_mod"

#5.demo vi venc stresstest
if [[ "$camera_model" = "sc3336" ]] || [[ "$camera_model" = "sc4336" ]]; then
    test_mod="PN_MODE=on HDR=off FRAMERATE=on LDCH=on RESOLUTION=on ENCODE_TYPE=on SMART_P=off SVC=on MOTION=on IDR=on DETACH_ATTACH=on ORDINARY=on"
elif [ "$camera_model" = "sc530ai" ]; then
    test_mod="PN_MODE=on HDR=on FRAMERATE=on LDCH=on RESOLUTION=on ENCODE_TYPE=on SMART_P=off SVC=on MOTION=on IDR=on DETACH_ATTACH=on ORDINARY=on"
else
    echo " the camera model: $camera_model is't support, exit !!!"
    exit 1
fi
demo_vi_venc_stresstest "$test_mod"

#print test result
cat $test_result_path
cat /tmp/testLog.txt
