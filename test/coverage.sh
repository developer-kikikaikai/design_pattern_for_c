#!/bin/sh
result_check() {
	result=`echo "if( $1 > 90.00 ) 1 else 0" | bc`
	return ${result}
}

result=0
#get coverage
LIST=`find . -name lib`
for data in $LIST;
do
	cd  ${data}
	DATA=`ls .libs/*.gcno`
	for info in $DATA
	do
		coverage=`gcov -s . $info | grep "Lines executed" | cut -c 16-20`
		result_check $coverage
		if [ $? != 1 ]; then
			echo "#####test ${data}/$info coverage failed ($coverage %)!! Please check test case"
			result=-1
		else
			echo "${data}/$info coverage is $coverage %!!"
		fi
	done
	cd - > /dev/null
done

exit 0
