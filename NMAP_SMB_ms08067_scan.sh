#!/bin/bash

for ((i=201; i < 255; i++)) ; do 
	ip=192.168.41.$i
	nmap -v -p 139,445 --script=smb-vuln-ms08-067 --script-args=unsafe=1 $ip >> SMBms08067_192.168.41.200-254.txt; 
done
