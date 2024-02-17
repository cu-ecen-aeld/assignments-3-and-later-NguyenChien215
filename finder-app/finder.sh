#!/bin/sh
# The first argument is the path to a directory on the file system
# The second argument is a text string which will be searched within these files
filesdir=$1
searchstr=$2

# Exits with return value 1 error and print statements if any of the parameters above were not specified
if [ $# -ne 2 ] 
then
	echo "Usage: $0 <filesdir> <searchstr>"
	exit 1
fi

#Exit with return value 1 error if filedir is not a directory
if [ ! -d ${filesdir} ] 
then
	echo "${filesdir} is not a directory in file system"
	exit 1
fi

# Count the  number of files and matching lines
file_count=$(find ${filesdir} -type f | wc -l)
match_count=$(grep -r ${searchstr} ${filesdir} | wc -l)

echo "The number of files are ${file_count} and the number of matching lines are ${match_count}"

