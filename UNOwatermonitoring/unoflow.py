import serial
import time

# Configure arduino serial port and wait for connection
comport = '/dev/cu.usbmodem1401'
ser = serial.Serial(comport, 9600)
time.sleep(2)

# Open a file to log data
with open('water_usage_log.csv', 'w') as file:
    file.write("Time (hours),Flow Rate (L/min),Total Volume (Liters)\n")
    
    while True:
        if ser.in_waiting > 0:
            # Read the line from the serial port and send output data to the file
            line = ser.readline().decode('utf-8').strip()
            print(line)
            file.write(line + '\n')
            file.flush()