#!/bin/sh -f


#remove dvlog
if [ -e ~/.dvlog ]
    then
    echo rm ~/.dvlog 
    rm ~/.dvlog 
fi

#remove inputrc
if [ -e ~/.inputrc ]
    then
    echo rm ~/.inputrc
    rm ~/.inputrc
fi

#remove .dvrc
if [ -e ~/.dvrc ]
    then
    echo rm ~/.dvrc
    rm ~/.dvrc
fi

#remove .app file
if [ -e /Applications/davinci.app ]
    then
    echo rm -rf /Applications/davinci.app
    sudo rm -rf /Applications/davinci.app
fi

echo 
echo davinci has been sucessfully removed!
