# Arduino Projects

Tweaked some scripts from other peoples projects that didn't exactly fit my needs but stil... Giving credit where credit is due;
  -  [Water flow sensor project](https://forum.arduino.cc/t/water-flow-sensor-project/646119)
  -  [Random Nerd Tutorials - ESP32 Web Server Adruino IDE](https://randomnerdtutorials.com/esp32-web-server-arduino-ide/)

## Example
To see this code in action, head over to my self hosted [Water Usage Monitor App](http://wum.molovestoshare.com:8080).

## Table of Contents

- [Introduction](#introduction)
- [Features](#features)
- [Installation](#installation)
- [Contributing](#contributing)
- [License](#license)

## Introduction

I created this project because I wanted a water flow sensor to help determine when and how much water is being used on this [(100GPD) RO filter I got from amazon](https://www.amazon.com/gp/product/B00DOG64FM/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1). It also assists with knowing around when to replace the filters for the system. Both wired and wireless should work but let me explain:

* The "UNOwatermonitoing" folder contains code for the (wired) [ArduinoUNO](https://www.amazon.com/gp/product/B008GRTSV6/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1), using [this hall effect sensor](https://www.amazon.com/gp/product/B07QS17S6Q/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)
* The "WiFiAdvancedWaterFlowMonitor" folder contains code for the (wireless) [Arduino UNO R4 Wifi](https://www.amazon.com/gp/product/B0C8V88Z9D/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1), using [this hall effect sensor](https://www.amazon.com/gp/product/B07DLZYSHT/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)

## Features

Key features of the project:

- Both scripts display water useage measurement in gallons, pretty accurately too! 😎
- The wireless version of the code has a simple dashboard displaying water usage [(see the example)](#example).
- The wired version has a python script that prints output from the console but, this script still needs work in order to achieve similar functionality to the wireless version.

## Installation

Instructions on how to set up and run the project locally are as follows:
Copy the 'arduino_secrets.h' file to the same directory as the script. Then use it store the sensitive data when compiling and uploading the code to your arduino in the Arduino IDE.

1. Clone [(or download)](https://github.com/mackmoe/arduino-projects/archive/refs/heads/main.zip) the repository:
   ```bash
   git clone https://github.com/mackmoe/arduino-projects.git
   ```
2. Open your [Arduino IDE](https://www.arduino.cc/en/software), any recent release should work.
3. Connect your Arduino to your computer and once the IDE recognizes it, make sure to update the IDE with all the dependencies for your board. *You  may also need some adittional packages for the script later but they should be downloaded automatically for you.
4. Open the repository folder (after it's been extracted) and navigate to the folder that has the code for your specific Arduino (wired or wireless).
5. Place a copy of the 'arduino_secrets.h' file in the folder and update the variables.
6. Verify and upload in the Arduino IDE


## Contributing

Instructions on how to contribute to the project:

1. Fork the repository.
2. Create a new branch (`git checkout -b feature/feature-name`).
3. Make your changes and commit them (`git commit -m 'Add feature'`).
4. Push to the branch (`git push origin feature/feature-name`).
5. Open a pull request.

## License

This project is licensed under the [MIT License](LICENSE).
---
