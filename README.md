
# [](https://github.com/HARSH1911RATHORE/IOT_final_project_spring_2020/tree/1262f5e660c0273681406807c0b4457386b7123b#iot_final_project_spring_2020)IOT_final_project_spring_2020

# [](https://github.com/HARSH1911RATHORE/IOT_final_project_spring_2020/tree/1262f5e660c0273681406807c0b4457386b7123b#pollution-monituring-system)Pollution Monituring System

# Overview
The project is a pollution monituring system which monitors the environmental pollution and air quality in the form of ppm value of co2 and humidity in the air. This data is recorded by the server and send to the client mobile application

**The repository contains the following files:**

-   Server.c - Contains code for ble server which will update the data captured from sensors to the client.

-   Client.c - Contains code for ble client which will get updated with new data from the server.

-   state_machine.c - Contains code for state machine implementation for humidity and aqi sensor

-   gpio.c - Contains code for button interrupt and pin mode set and configuration

-   I2C_INTERRUPT.c - Contains code for i2c based interrupt functions to write, read and the i2c0 interrupt handler

-   i2c.c - Contains code for polling based implementation of i2c

-   display.c - Contains code for lcd display

-   log.c - contains initialization and functions for logging

-   letimer.c - Contains code for letimer functions initialization including clock. Also contains the letimer irq handler

# Features 
* Interfacing CCS811 air quality sensor and on board humidity sensor
* Sends sensor values to the client mobile application
* Records the maximum sensor reading in the persistent memory which is displayed on the lcd through a button press after bonding has been established
* State machines utilising non polling mode and polling mode otherwise

# Hardware Compoenents
* SI LABS bluegecko13 board
* CCS811 Sensor and Si7021 sensor
* Mobile 

# Block Diagram
![](https://github.com/HARSH1911RATHORE/IOT_final_project_spring_2020/blob/master/BLOCK_DIAGRAM_IOT.png)

## [](https://github.com/HARSH1911RATHORE/IOT_final_project_spring_2020/tree/1262f5e660c0273681406807c0b4457386b7123b#project-status)Project Status

-   The Air quality sensor has been interfaced to the board with Pin number P31 for PD 13, SCL and Pin number P33 for PD 14, SDA. State machine for Humidity based sensor was created with data being measured in no hold master mode .
-   Gpio even based interrupt was configured which gets the status of button press. This button press will be used for bonding the board with the client to prevent man in the middle attack.
-   Persistent data was stored in the flash. The data will be stored for every value of data which exceeds the previous maximum value for that particular sensor. This data will be displayed on the server board every time a external button pb0 is pressed.
-   Communication was established between the board and the mobile application and was verified

# Documentation
## HARSH RATHORE [](https://github.com/HARSH1911RATHORE/IOT_final_project_spring_2020/tree/1262f5e660c0273681406807c0b4457386b7123b#google-drive-link)[Google drive link](https://drive.google.com/drive/u/1/folders/1FM8G4nL0fplihENYMFtwnbRBXwX1w5KG)
