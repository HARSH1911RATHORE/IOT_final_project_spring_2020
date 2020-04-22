`CONTRIBUTOR - HARSH RATHORE  `


# IOT_final_project_spring_2020

# Pollution Monituring System

## Project Status
* The Air quality sensor has been interfaced to the board with Pin number P31 for PD 13, SCL and Pin number P33 for PD 14, SDA. State machine for Humidity based sensor was created with data being measured in no hold master mode . 
* Gpio even based interrupt was configured which gets the status of button press. This button press will be used for bonding the board with the client to prevent man in the middle attack.
* Persistent data was stored in the flash. The data will be stored for every value of data which exceeds the previous maximum value for that particular sensor. This data will be displayed on the server board every time a external button pb0 is pressed.
* Communication was established between the board and the mobile application and was verified

The repository contains the following files:

* Server.c - Contains code for ble server which will update the data captured from sensors to the client.

* Client.c - Contains code for ble client which will get updated with new data from the server.

* state_machine.c - Contains code for state machine implementation for humidity and aqi sensor

* gpio.c - Contains code for button interrupt and pin mode set and configuration

* I2C_INTERRUPT.c - Contains code for i2c based interrupt functions to write, read and the i2c0 interrupt handler

* i2c.c - Contains code for polling based implementation of i2c

* display.c - Contains code for lcd display

* log.c - contains initialization and functions for logging

* letimer.c - Contains code for letimer functions initialization including clock. Also contains the letimer irq handler


## [Google drive link](https://drive.google.com/drive/u/1/folders/1FM8G4nL0fplihENYMFtwnbRBXwX1w5KG)


