#!/bin/bash

function proof { #source, tested, proof
	echo "= [SOURCE] $1 ===="
	cat -n $1
	if [ -f $3 ]; then
		echo "= [OLDPROOF] ===="
		cat $3
	fi
	echo "= [RESULT] $2 ===="
	cat $2
	echo -n "==== Okay? [Y/n] "
	read INPUT
	if [ "$INPUT" != "n" ]; then
		echo "proof $2 $3"
		mv -f $2 $3
		git add $1
		git add $3
	fi
	echo
	echo
	echo
}

FILES=$(ls *_tested)

for FILE in ${FILES[@]}
do
	SOURCE="${FILE%.*}"
	if [ -f $SOURCE ]; then
		PROOF=$(basename $FILE _tested)
		PROOF="${PROOF}_proof"
		if [ -f $PROOF ]; then
			md5tested=$(cat $FILE | md5)
			md5proof=$(cat $PROOF | md5)
			echo "$FILE: $md5tested"
			echo "$PROOF: $md5proof"
			if [ "$md5tested" != "$md5proof" ]; then
				proof $SOURCE $FILE $PROOF
			fi
		else
			proof $SOURCE $FILE $PROOF
		fi
	fi
done


