#!/usr/bin/env bash

#set -x

function initialize_console()
  {
    pushd /home/$USER/Git/arduino-projects/R4WiFi/Water_Flow_Sensor
    arduino-cli compile -t -b arduino:renesas_uno:unor4wifi -p /dev/ttyACM1 ./Water_Flow_Console_Log.ino --upload
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
        screen -Sdm "wifi-water-monitoring-logs" bash -c "arduino-cli monitor -p /dev/ttyACM1 --timestamp > /var/log/WaterLogs/wifi_arduino_waterlog-$(date +"%m-%d-%Y_%H:%M").log"
}

function screen_session_magic() {
	screen -ls
	if [ $? -eq 0 ];then
	  screen -wipe
	else
	  echo "Starting Console Log"	
        fi      
}

chkpid
screen_session_magic
start_water_monitor_log
