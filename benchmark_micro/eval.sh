for FILE in ./instance/*.gz; 
do   
	gzip -d "$FILE"
	../test $1 12345 $2 "${FILE::-3}"
	gzip "${FILE::-3}"
done
