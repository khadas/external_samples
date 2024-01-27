#/* GPL-2.0 WITH Linux-syscall-note OR Apache 2.0 */
#/* Copyright (c) 2024 Fuzhou Rockchip Electronics Co., Ltd */
#!/bin/sh
output=$2/simple/output
log=$2/simple/log
report=$2/simple/report
rm -rf $output
rm -rf $log
rm -rf $report
rm -rf /tmp/simple
mkdir -p $output
mkdir -p $log
mkdir -p $report

if [ "$2" != "/tmp" ]; then
ln -s $2/simple /tmp/simple
fi

while read line
do
echo "????? ${line} start test! ?????"
notecmd=$(echo "$line" | sed -n '/^\#/p')
if [ "$notecmd" != "" ]; then
	continue
fi

back=$(echo "$line" | sed -n '/&/p')
simple=$(echo "$line" | sed -n '/^simple/p')

#check md5sum
md5data=$(echo "$line" | sed -n 's/check.*$/\1/p')
md5cmd=$(echo "$line" | sed -n 's/^.*check/\1/p')

#set log level
logcmd=$(echo "$line" | sed -n 's/^.*log/\1/p')
if [ "$logcmd" != "" ]; then
	echo $logcmd > /tmp/rt_log_level
	continue
fi
#remove unexport char
log_name=$(echo "$line" | sed 's/ /_/g')
log_name=$(echo "$log_name" | sed 's/\./_/g')
log_name=$(echo "$log_name" | sed 's/\//_/g')

#check background run
if [ "$back" != "" ]; then
	cmd=$(echo "$line" | sed 's/&//g')
	log_name=$(echo "$log_name" | sed 's/&//g')
else
	cmd=$line
fi

#check simple cmd
if [ "$simple" != "" ]; then
	log_name=$log/$log_name".log"
	simpcmd=$(echo "$cmd" | sed 's/ /_/g')
	simpcmd=$(echo "$simpcmd" | sed 's/\./_/g')
	simpcmd=$(echo "$simpcmd" | sed 's/\//_/g')
else
	if [ "$md5cmd" != "" ]; then
		cmd=$md5cmd
		log_name=$output/$simpcmd".md5"
	else
		log_name=/dev/null
	fi
fi

#run cmd
if [ "$back" != "" ]; then
	$cmd &> $log_name&
else
	$cmd &> $log_name
fi

#check result
if [ $? -eq 0 ]; then
	#copy result
	if [ "$md5cmd" != "" ]; then
		md5file=$(sed -n 's/^.* /\1/p' $log_name)
		cp $md5file $output/$simpcmd".bin"
	else
		echo "^^^^^ ${line} success! ^^^^"
	fi
	#md5sum check
	if [ "$md5data" != "" ]; then
		md5newdata=$(sed -n 's/ .*$/\1/p' $log_name)
		if [ "$md5newdata" != "$md5data" ]; then
    		echo "$md5file md5 check fail"
    		exit
		fi
	fi
else
	echo "xxxxx ${line} failed! xxxxx"
	exit 1
fi
done < $1
