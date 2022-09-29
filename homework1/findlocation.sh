#!/bin/bash/
# This bash script is used to search for a designated file for a specific input pattern
# in this case the text file 'nanpa'

# Passsed Arguments:
#    $0 - script name
#    $1 - phone_prefix

# BASH Syntax
#    -z flag

# REG_EXP Syntax
#    ^ start of entry
#    $ end of entry 


if [ -z "$1" ] || [[ ! $1 =~ ^[[:digit:]]+$ ]]
then
  echo "Missing 6 digit North American phone number prefix"
  exit 1
fi

location=$(grep $1 nanpa )

if [ -z "$location" ]
then
  echo "NO MATCH FOUND IN NANPA"
  exit 1
fi

location=`echo $location | sed 's/*$//'` # Remove portion after last character
location=`echo $location | sed "s/$1//"` # Remove prefix leaving only location text
echo $location



: <<'COMMENT'
read -p "Please enter a 6 dgit prefix consisting of [0-9]: " STR
echo "You entered, "$STR

# check if argument file is readable
if [$STR = ""];then
    exit 1;
fi

# check if correct length
if[[ ${STR} != 6 ]];then
    exit 1;
fi

# check if it consists of purely digits 
if[[$STR =~ [0-9] ]];then
    if grep ${STR} nanpa;then
	echo ${grep ${STR} nanpa | sed -r 's/'${STR}'//')
    else
	echo "Prefix not found in nanpa"
    fi
fi
COMMENT


      
			    
	
	    


