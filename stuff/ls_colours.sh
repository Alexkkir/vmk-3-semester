#!/bin/bash
eval $(echo "no:global default;fi:normal file;di:directory;ln:symbolic link;pi:named pipe;so:socket;do:door;bd:block device;cd:character device;or:orphan symlink;mi:missing file;su:set uid;sg:set gid;tw:sticky other writable;ow:other writable;st:sticky;ex:executable;"|sed -e 's/:/="/g; s/\;/"\n/g')           
{      
	IFS=:
   	iter=0	
	n=10
	for i in $LS_COLORS     
		do 
		 	if [ $iter -lt 18 ]
		 	then	 
				echo -e "\e[${i#*=}m$( x=${i%=*}; [ "${!x}" ] && echo "${!x}" || echo "$x" )\e[m" 
				iter=$((iter + 1))
			fi
		done      
} 

