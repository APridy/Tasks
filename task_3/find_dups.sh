#!/bin/bash

DIRECTORY=./
MODE=name
SCRIPTNAME=rm_dups.sh

while getopts d:m:s: option
do
	case "${option}" in
	d) DIRECTORY=${OPTARG};;
	m) MODE=${OPTARG};;
	s) SCRIPTNAME=${OPTARG};;
	esac
done

[[ -d $DIRECTORY ]] || (echo Invalid directory!; exit)

MODES=("name" "cont")
if [[ ! " ${MODES[*]} " =~ " $MODE " ]] ; then
	echo Invalid mode!
	exit
fi

touch $SCRIPTNAME
chmod +x $SCRIPTNAME
echo -e '#!/bin/bash'"\n" > $SCRIPTNAME

FILES=$(find $DIRECTORY -type f)

OIFS="$IFS"
IFS=$'\n'

if [[ "$MODE" == "${MODES[0]}" ]]; then
	for filepath in $FILES
	do
		if [[ ! " ${KNOWN_FILEPATHS[*]} " =~ " $(basename $filepath) " ]]; then
			MATCHES=0
			for cmp_filepath in $FILES
        	do
				if [[ $(basename $filepath) == $(basename $cmp_filepath) ]]; then
					((MATCHES=MATCHES+1))
					case $MATCHES in
					1)
						KNOWN_FILEPATHS+="$(basename $filepath) "
						;;
					2)	
						echo "###"----$(basename $filepath)----"###" >> ./$SCRIPTNAME
						echo "#"rm "\""$filepath"\"" >> ./$SCRIPTNAME
						echo "#"rm "\""$cmp_filepath"\"" >> ./$SCRIPTNAME
						;;
					*)	
						echo "#"rm "\""$cmp_filepath"\"" >> ./$SCRIPTNAME
						;;
					esac
				fi
        	done
		fi
	done
else	
	GROUPS_COUNT=0
	for filepath in $FILES
	do  
		if [[ ! " ${KNOWN_MD5SUMS[*]} " =~ " $(md5sum $filepath | cut -d" " -f1) " ]]; then
			MATCHES=0
			for cmp_filepath in $FILES
			do
				if [[ $(md5sum $filepath | cut -d" " -f1) == $(md5sum $cmp_filepath | cut -d" " -f1) ]]; then
					((MATCHES=MATCHES+1))
					case $MATCHES in          
					1)                  
						KNOWN_MD5SUMS+="$(md5sum $filepath | cut -d" " -f1) "
						;;
					2) 	
						((GROUPS_COUNT=GROUPS_COUNT+1))				
						echo -e "\n""###"-------Group $GROUPS_COUNT-----------"###" >> ./$SCRIPTNAME
						echo "#"rm "\""$filepath"\"" >> ./$SCRIPTNAME
						echo "#"rm "\""$cmp_filepath"\"" >> ./$SCRIPTNAME
						;;                      
					*)                  
						echo "#"rm "\""$cmp_filepath"\"" >> ./$SCRIPTNAME
						;;
					esac
				fi
			done
		fi
	done
fi

IFS="$OIFS"
