#!/bin/sh

## this file needs to go to /etc/network/if-up.d/
FLAGFILE=/var/run/coala_server_launched

case "$IFACE" in
    lo)
        # The loopback interface does not count.
        # only run when some other interface comes up
        exit 0
        ;;
    *)
        ;;
esac

if [ -e $FLAGFILE ]; then
    exit 0
else
    touch $FLAGFILE
fi

: here, do the real work.
touch /var/run/coalawaslaunched
cd /home/root/src/control_loop/projects/COALA1/
./control_loop -webserver=10000 &> /dev/null &

