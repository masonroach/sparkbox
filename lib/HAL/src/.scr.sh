#!/bin/bash
for file in $( find . "*.h")
do
	mv "$file" "${file/\.\//\.\/\.}"
done
