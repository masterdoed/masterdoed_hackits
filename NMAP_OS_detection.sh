#!/bin/bash

for ((i=201; i < 255; i++)) ; do 
	ip=192.168.41.$i
	nmap -O $ip >> OSDet_192.168.41.201-254.txt; 
done
