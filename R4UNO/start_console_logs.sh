#!/usr/bin/env bash

#set -x

function initialize_console()
  {
    pushd /home/$USER/Git/arduino-projects/R4UNO/Water_Flow_Sensor
    arduino-cli compile -t -b arduino:avr:uno -p /dev/ttyACM0 ./Water_Flow_Console_Log.ino --upload
    popd
}

function chkpid()
  {
    ps -ef | grep "[a]rduino-cli monitor"
    if [ $? -eq 0 ]; then
      ps -ef | grep "[a]rduino-cli monitor" | awk '{print $3}' | xargs sudo kill -9 && \
      initialize_console
    else
      initialize_console
    fi
}

function start_water_monitor_log()
  {
        screen -Sdm "water-monitoring-logs" bash -c "arduino-cli monitor -p /dev/ttyACM0 --timestamp > /var/log/WaterLogs/arduino_waterlog-$(date +"%m-%d-%Y_%H:%M").log"
}

function screen_session_magic() {
	screen -ls
	if [ $? -eq 0 ];then
	  screen -wipe
        fi      
}

chkpid
screen_session_magic
start_water_monitor_log
