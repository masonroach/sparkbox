#!/bin/bash
# To easily open files for Mason

# Default files to open
DEFAULT=( "main" "video" "sprite" )

# Use either default values or input arguments
if [ $# -eq 0 ]
then
	ARGS=${DEFAULT[@]}
else
	ARGS=$@
fi

result="vim -p"

first=0
for arg in $ARGS
do
	if [ $first -eq 0 ]
	then
		result+=" inc/$arg.h -c 'vs src/$arg.c'"
		first+=1;
	else
		result+=" inc/$arg.h -c 'tabn' -c 'vs src/$arg.c'"
	fi
done

result+=" -c 'tabn'"

eval $result
