import serial
import requests
import time
import json

SERIAL_PORT = '/dev/ttyACM0'
BAUD_RATE = 9600
TELEGRAF_HTTP_URL = 'http://hostname-or-ip:8125/ro-flow'

def main():
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=2)

    while True:
        if ser.in_waiting:
            line = ser.readline().decode('utf-8').strip()
            try:
                data = json.loads(line)
                response = requests.post(TELEGRAF_HTTP_URL, json=data)
                print(f"Sent: {data}, Telegraf response: {response.status_code}")
            except json.JSONDecodeError:
                print(f"Ignored invalid JSON: {line}")
            except requests.RequestException as e:
                print(f"HTTP error: {e}")
            time.sleep(1)

if __name__ == "__main__":
    main()
