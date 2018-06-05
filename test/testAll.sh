#!/bin/sh

#find test name
LIST=`find . -name test | grep -v .libs`
for data in $LIST
do
	#skip directory
	dircheck=`file $data | awk -F" " '{print $2}'`
	if [ "x${dircheck}" = "xdirectory" ]; then
		continue;
	fi

	basefile=`echo $data | xargs basename`
	length=`expr ${#data} - ${#basefile}`
	dirname=`echo $data | cut -c 1-${length}`

	cd $dirname
	./$basefile > /dev/null 2>&1
	if [ $? != 0 ]; then
		echo "test $data failed!!"
		exit 1
	fi
	cd -
	echo "===$data Success!!"
done

exit 0
