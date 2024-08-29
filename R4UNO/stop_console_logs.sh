#!/usr/bin/env bash

# set -x 

function screen_session_magic() {
	screen -ls
	if [ $? -eq 0 ];then
	  screen -wipe
        fi      
}

function stop_water_monitor_console()
  {
    ps -ef | grep "[a]rduino-cli monitor"
    if [[ $? -eq 0 ]]; then
      ps -ef | grep "[a]rduino-cli monitor" | awk '{print $3}' | xargs sudo kill -9 && \
      echo -e "Stopped running log" >> "/var/log/WaterLogs/stoped_arduino_waterlog-$(date +"%m-%d-%Y_%H:%M").log"
    else
      echo -e "Logs were not running" >> "/var/log/WaterLogs/stoped_arduino_waterlog-$(date +"%m-%d-%Y_%H:%M").log"
    fi
}

stop_water_monitor_console
screen_session_magic