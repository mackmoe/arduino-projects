#!/usr/bin/env bash

# set -x 

function stop_bkgd_screen()
  {
    SCREENPID=$(ps -ef | grep -i [s]creen)
    if [[ $SCREENPID ]]; then
      rm -rf /run/screen/S-"$USER"/* && \
      screen -wipe '*'
    else
      echo -e "No screens are running in the background"
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

stop_bkgd_screen
stop_water_monitor_console