if [ "x$1" != "x" ]; then
	grep -3  "#####" $1
else
	#check result
	find . -name *gcov |xargs grep -3  "#####"
fi
