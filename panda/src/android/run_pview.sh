# This script can be used for launching the Panda viewer from the Android
# terminal environment, for example from within termux.  It uses a socket
# to pipe the command-line output back to the terminal.

port=12345

if [[ $# -eq 0 ]] ; then
    echo "Pass full path of model"
    exit 1
fi

am start --activity-clear-task -n org.panda3d.sdk/org.panda3d.android.PandaActivity --user 0 --es org.panda3d.OUTPUT_URI tcp://127.0.0.1:$port --grant-read-uri-permission --grant-write-uri-permission file://$(realpath $1)

nc -l -p $port
