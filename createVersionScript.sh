#!/bin/sh

output_conf_map() {
	#{
	#  global:
	#    function_name;
	# ...
	#  local: *;
	#}

	#only get function list from header file
	HEADER_FUNC_LIST=`grep "[a-zA-Z](" -r $1 | grep -v "@brief" | grep -v "#define" | awk -F"(" '{print $1}' | awk -F " " '{print $NF}'`
	#template
	echo "{"
	echo "  global:"

	#show all function
	for data in $HEADER_FUNC_LIST
	do
		echo "    $data;"
	done

	#template end
	echo "  local: *;"
	echo "};"
}

INCLUDE_LIST=`find . -name include`
for inc_dir in $INCLUDE_LIST
do
	prevdir=`echo $inc_dir | awk -F"/" '{i=NF-1; print $i}'`
	outfile=${prevdir}.map
	echo $inc_dir
	output_conf_map  $inc_dir > ./${prevdir}/lib/$outfile
done

