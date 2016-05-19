#!/bin/bash

#get the most recent library copy

printf "\nUpdating Current Library\n"
svn update

#get the log for the library
printf "\nUpdating Library Change Log\n"
svn log -v | sed -e 's/^[ \t]*//' | sed -e 's/^[+]/*/' | head -n -1> change_log_dvrc.txt

#get the log for the core
printf "Updating Core Change Log\n"
svn log -v http://oss.mars.asu.edu/svn/davinci/davinci/trunk | sed -e 's/^[ \t]*//' | sed -e 's/^[+]/*/' | head -n -1> change_log_core.txt
svn log -v http://oss.mars.asu.edu/svn/davinci/davinci/trunk/version.h | sed -e 's/^[ \t]*//' | sed -e 's/^[+]/*/' | head -n -1> change_log_version.txt

#make the update_library.txt download file
printf "Building List: update_library.txt\n"
find -L . -type f | grep -v "~" | grep -v "#" | grep -v "msff.dvrc" | grep -v "library_mars.dvrc" | grep -v "svn" | cut -c 3- | tail -n +2 | grep -v "update_library" > update_library.txt


if [ -e "../public_library/update_version.sh" ]
then
		printf "\nUpdating Past Library Versions\n"
		cd ../public_library/
		./update_version.sh
fi