#!
shopt -s lastpipe
echo "$@"| read x

python3 setup.py build_ext -i && python3 -m tests.compose && printf "Testing...\n" && python3 -m tests.run &&  {
st=$( tail -1 results.csv | awk -F"," '{print $1}')
if [ "$st" = 'OK' ]
then
  n=$(wc -l < results.csv)
  for (( i=2; i<=$n; i++ ))
  do
    line=$(head -"$i" < results.csv | tail -1)
    video=$(echo "$line" | awk -F"," '{print $2}' | awk -F"/" '{printf("%-15s", $(NF))}')
    res=$(echo "$line" | awk -F"," '{printf("%-4s %s-%s\t%s\t%s\t", $4, substr($5,1,6), substr($5,16,1), substr($7,1,5), substr($8,1,5))}')

    if [ -s log.csv ]
    then
            false
    else
            echo "" | awk '{printf("%-15s q\tpsnr\tspeed\tdisp\t comment\n", "video")}' > log.csv
    fi
    echo "$video" "$res" "$x" >> log.csv
  done
  awk '{if($1 == "source.avi" || $1 == "video") print $0}' log.csv
else
    echo "Testing status not OK"
fi
}