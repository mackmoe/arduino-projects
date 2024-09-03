function screen_session_magic() {
	screen -ls
	if [[ $? -eq 0 ]];then
	  screen -wipe
	fi
}

function stop_water_monitor_console()
  {
    ps -ef | grep "[a]rduino-cli monitor"
    if [[ $? -eq 0 ]]; then
      ps -ef | grep "[a]rduino-cli monitor" | awk '{print $3}' | xargs sudo kill -9
    fi
}

stop_water_monitor_console
screen_session_magic
echo "Ready to start water console logs"