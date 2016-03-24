#!bin/bash
echo "" > result

src/json_modbus -C weatherstation.conf /dev/ttyUSB0 >> result

cat result

# value=[] #declare an array to store values
# #idx=0 #initialize a counter to zero
# parameters=`jq -r '.weatherstation|keys |.[]' result`
# ##echo "$parameters"

# for para in $parameters
# do
#    value[idx]=`jq ".[] | .$para" result`
#    echo "$para"
#    idx=$((idx+1))  #increment the counter
# done
#
# echo ${value[*]} #your result



## src/json_modbus -s -I -b 9600 -S --reg "uint32 brightness_north 31215" /dev/ttyUSB0
