#!/bin/bash

for ((i=201; i < 255; i++)) ; do 
	ip=192.168.41.$i

	echo ""
        echo "-------------------------- smtp-vuln-cve2010-4344"
        echo ""
	nmap -v -p 25 --script=smtp-vuln-cve2010-4344 --script-args=unsafe=1 $ip 
	
	echo ""
	echo "-------------------------- smtp-vuln-cve2011-1720"
	echo ""
	nmap -v -p 25 --script=smtp-vuln-cve2011-1720 --script-args=unsafe=1 $ip

	echo ""
        echo "-------------------------- smtp-vuln-cve2011-1764"
        echo ""
	nmap -v -p 25 --script=smtp-vuln-cve2011-1764 --script-args=unsafe=1 $ip
done

