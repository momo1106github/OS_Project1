#! /usr/bin/env bash

extension="_stdout.txt"
extension2="_dmesg.txt"
context=$(ls ./OS_PJ1_Test)
for i in $context
do
	#remove the .txt
	prefix=$(echo $i | sed "s/....$//g")
	echo $prefix
	dmesg -C
	./main < "./OS_PJ1_Test/$i" > ./output/"$prefix$extension"
	sleep 1
	dmesg | grep Project1 > ./output/"$prefix$extension2"
	
done
