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

#remove Library and examples
if [ -e /Library/davinci ]
    then
    echo rm -rf /Library/davinci
    sudo rm -rf /Library/davinci
fi

#remove main install location
if [ -e /usr/local/davinci ]
    then
    echo rm -rf /usr/local/davinci
    sudo rm -rf /usr/local/davinci
fi

#remove .app file
if [ -e /Applications/davinci.app ]
    then
    echo rm -rf /Applications/davinci.app
    sudo rm -rf /Applications/davinci.app
fi

#remove the reciepts
if [ -e /Library/Receipts/davinci_library.pkg ]
    then
    echo rm -rf /Library/Receipts/davinci*.pkg
    sudo rm -rf /Library/Receipts/davinci*.pkg
fi

echo 
echo davinci has been sucessfully removed!
