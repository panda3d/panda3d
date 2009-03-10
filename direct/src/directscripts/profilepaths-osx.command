#!/bin/bash

# We should only need to adjust the version in the future, hopefully.
PANDA_VERSION=$1
PANDA_PATH=/Applications/Panda3D/\$PANDA_VERSION
PROFILE=$HOME/.bash_profile
PROFILE_BACKUP=$HOME/.bash_profile_backup

# Build the block of stuff to put in the user's BASH profile
BASH_BLOCK='if [ -d $PANDA_PATH ]
then
    export PATH=$PANDA_PATH/bin:$PATH
    export PYTHONPATH=$PANDA_PATH/lib:$PYTHONPATH
    export DYLD_LIBRARY_PATH=$PANDA_PATH/lib:$DYLD_LIBRARY_PATH
    export MAYA_SCRIPT_PATH=$PANDA_PATH/plugins:$MAYA_SCRIPT_PATH
    export MAYA_PLUG_IN_PATH=$PANDA_PATH/plugins:$MAYA_PLUG_IN_PATH
fi
'

BASH_BLOCK="
PANDA_VERSION=$PANDA_VERSION
PANDA_PATH=$PANDA_PATH
$BASH_BLOCK"

# Let's get started!
clear
if [ -f $PROFILE ]
then
    # Back it up first
    cp $PROFILE $PROFILE_BACKUP
    
    # Check if it isn't already configured
    if [ "$(grep PANDA_VERSION=$PANDA_VERSION $PROFILE)" != "" ]
    then
        echo "It looks like your version of Panda3D is already configured!"
        echo ""
        echo "Exiting. You can close this window."
        echo ""
        exit
    elif [ "$(grep 'PANDA_VERSION=[0-9].[0-9].[0-9]' $PROFILE)" != "" ]
    then
        $(sed -e s@PANDA_VERSION=[0-9].[0-9].[0-9]@PANDA_VERSION=$PANDA_VERSION@ -e s@PANDA_PATH=/Applications/Panda3D/[0-9].[0-9].[0-9]@PANDA_PATH=$PANDA_PATH@ < $PROFILE_BACKUP > $PROFILE)
        echo "Success! Your version of Panda3D has been changed to $PANDA_VERSION."
        echo ""
        echo "All done! You can close this window."
        echo ""
        exit
    fi
fi

echo "This script will attempt to look at your BASH profile and add"
echo "appropriate entries so that Panda3D will work for you. This means"
echo "adding the following to $PROFILE :"
echo ""
echo "$BASH_BLOCK"
echo ""
echo "Continue? (Y/N)"
read CONTINUE
clear
if [ $CONTINUE != 'Y' -a $CONTINUE != 'y' ]
then
    echo "Please note that Panda3D will not function properly unless"
    echo "your environment is configured properly."
    echo ""
    echo "Exiting. You can close this window."
    echo ""
    exit
fi

if [ ! -f $PROFILE ]
then
    echo "No $PROFILE file found. Creating one."
    echo ""
    $(touch $PROFILE)
fi

if [ "$(grep -i panda3d $PROFILE)" != "" ]
then
    echo "Your profile already has some reference to 'panda3d'!"
    echo "It looks like you might already have the paths set up. If you're"
    echo "upgrading, might just need to change PANDA_VERSION to $PANDA_VERSION"
    echo ""
    echo "Open your profile in TextEdit so you can review it? (Y/N)"
    read OPEN_PROFILE
    if [ $OPEN_PROFILE == 'Y' -o $OPEN_PROFILE == 'y' ]
    then
        clear
        echo "Opening $PROFILE"
        echo "This is the sort of block you are looking for:"
        echo ""
        echo "$BASH_BLOCK"
        echo ""
        echo "Exiting. You can close this window."
        echo ""
        $(open /Applications/TextEdit.app $PROFILE)
        exit
    else
        clear
        echo "Cowardly refusing to touch your profile because you already"
        echo "have some reference to 'panda3d'. Here is what needs to be in"
        echo "$PROFILE :"
        echo ""
        echo "$BASH_BLOCK"
        echo ""
        echo "Exiting. You can close this window."
        echo ""
        exit
    fi
else
    echo "Adding the following to $PROFILE :"
    echo ""
    echo "$BASH_BLOCK"
    echo ""
    echo "$BASH_BLOCK" >> $PROFILE
    echo "All done! You can close this window."
    echo ""
fi

