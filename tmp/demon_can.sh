#!/bin/sh

### BEGIN INIT INFO
# Provides:          demon_can
# Required-Start:    $all
# Required-Stop:     $all
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Inicjowanie interefejsu can0
# Description:       Inicjowanie interefejsu can0
### END INIT INFO

# Change the next 3 lines to suit where you install your script and what you want to call it
DIR=/home/pi/py/can
DAEMON=$DIR/demon_can.py
DAEMON_NAME=demon_can

# Add any command line options for your daemon here
DAEMON_OPTS=""

# This next line determines what user the script runs as.
# Root generally not recommended but necessary if you are using the Raspberry Pi GPIO from Python.
DAEMON_USER=root

# The process ID of the script when it runs is stored here:
PIDFILE=/var/run/$DAEMON_NAME.pid

. /lib/lsb/init-functions

do_start () {
    log_daemon_msg "Starting system $DAEMON_NAME daemon Olek"
    ifconfig can0 down
    ip link set can0 type can bitrate 50000
    ifconfig can0 up
    sleep 10 #odczekaæ aby can0 siê zainicjowa³o 
    log_daemon_msg "Up can0"
    start-stop-daemon --start --background --pidfile $PIDFILE --make-pidfile --user $DAEMON_USER --chuid $DAEMON_USER --startas $DAEMON -- $DAEMON_OPTS
    log_end_msg $?
}
do_stop () {
    log_daemon_msg "Stopping system $DAEMON_NAME daemon Olek"
    start-stop-daemon --stop --pidfile $PIDFILE --retry 10
    ifconfig can0 down
    log_daemon_msg "Down can0"
    log_end_msg $?
}

case "$1" in

    start|stop)
        do_${1}
        ;;

    restart|reload|force-reload)
        do_stop
        do_start
        ;;

    status)
        status_of_proc "$DAEMON_NAME" "$DAEMON" && exit 0 || exit $?
        ;;

    *)
        echo "Usage: /etc/init.d/$DAEMON_NAME {start|stop|restart|status} olek"
        exit 1
        ;;

esac
exit 0