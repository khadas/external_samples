#!/bin/sh
killall nginx
sleep 10
counter=0
while [ $counter -lt 10000 ]
do
    rkipc_pid=$(ps |grep rkipc|grep -v grep |awk '{print $1}')
    kill -9 "$rkipc_pid"
    while true
    do
        ps|grep rkipc |grep -v grep
        if [ $? -ne 0 ]; then
            echo "kill -9 rkipc exit, run once: count = $counter"
            break
        else
            sleep 1
            echo "rkipc active"
        fi
    done
    rkipc -a /oem/usr/share/iqfiles &
    sleep 5
    counter=$(( counter + 1 ))
done

killall nginx
sleep 10
counter=0
while [ $counter -lt 10000 ]
do
    killall rkipc
    while true
    do
        ps|grep rkipc |grep -v grep
        if [ $? -ne 0 ]; then
            echo "killall rkipc exit, run once: count = $counter"
            break
        else
            sleep 1
            echo "killall rkipc active"
        fi
    done
    rkipc -a /oem/usr/share/iqfiles &
    sleep 5
    counter=$(( counter + 1 ))
done
