#!/bin/bash 

tshark -r capture.pcap -Y "udp" -T fields -e udp.srcport -Y ip.src==10.0.0.66 > ports.txt
ARRAY=(`cat ports.txt`)
arrayindex=0
flag=""
chr() {
  printf \\$(printf '%03o' $1)
}


for i in "${ARRAY[@]}"
do
   portnum=( "${ARRAY[arrayindex]}" )
   decimalportnum=${portnum:1}
   if [[ ${decimalportnum:0:1} == "0" ]]
   then
      decimalportnum=${decimalportnum:1}
   fi
   flagchar=(`chr $decimalportnum`)
   flag+=$flagchar
   arrayindex=$((arrayindex+1))
done

echo $flag