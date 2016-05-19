#!/bin/bash

dir=`dirname $0`
echo $dir

echo copying davinci.app to /Applications
cp -r $dir/Components/davinci.app /Applications/

inquire ()  {
  echo  -n "$1 [$2/$3]? "
  read answer
  finish="-1"
  while [ "$finish" = '-1' ]
  do
    finish="1"
    if [ "$answer" = '' ];
    then
      answer=""
    else
      case $answer in
        y | Y | yes | YES ) answer="y";;
        n | N | no | NO ) answer="n";;
        *) finish="-1";
           echo -n 'Invalid response -- please reenter:';
           read answer;;
       esac
    fi
  done
}

inquire "Do you want to install .dvrc and .inputrc components?" Y N

if [ "$answer" = "y" ]
then
		echo "Copying .dvrc and .inputrc components to ~/"
		cp $dir/Components/dvrc ~/.dvrc
		cp $dir/Components/inputrc ~/.inputrc
else 
		echo "Skipping .dvrc and .inputrc components"
fi

echo Copying other libraries as necessary
if [ ! -e /usr/lib/libltdl.3.dylib ]
then
  sudo cp $dir/Components/libltdl.3.dylib /usr/lib/
  sudo chown root:wheel /usr/lib/libltdl.3.dylib
  echo Copying libltdl.3
fi

if [ ! -e /usr/lib/libltdl.7.dylib ]
then
  sudo cp $dir/Components/libltdl.3.dylib /usr/lib/
  sudo chown root:wheel /usr/lib/libltdl.3.dylib
  echo Copying libltdl.7
fi

if [ ! -e /usr/lib/libcurl.4.dylib ]
then
 	sudo cp $dir/Components/libcurl.4.dylib /usr/lib/
	sudo chown root:wheel /usr/lib/libcurl.4.dylib
  echo Copying libcurl.4
fi

if [ ! -e /usr/bin/pdshead ]
then
  sudo cp $dir/Components/pdshead /usr/bin/
  sudo chown root:wheel /usr/bin/pdshead
	  echo Copying pdshead
fi
