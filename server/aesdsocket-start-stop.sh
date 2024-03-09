# !/bin/sh
# Author: Chien

echo -n "Number of args: "
echo $#

if [ $# -ne 1 ]; then
    echo "Error, you must provide either: -S or -K"
    exit 1
fi

w_mode=$1

case $w_mode in
    start)
        echo "Starting socket 9000" 
        start-stop-daemon -S -n aesdsocket -a /usr/bin/aesdsocket -- -d
        ;;
    stop)
        echo "Stopping socket 9000" 
        start-stop-daemon -K -n aesdsocket
        ;;
    *)
        echo "Invalid option!"
    exit 1
esac

exit 0