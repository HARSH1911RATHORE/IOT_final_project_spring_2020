# ECEN 5823 Assignment Skeleton

This project contains skeleton code used for coursework in University of Colorado [ECEN 5823 IoT Embedded Firmware](https://sites.google.com/colorado.edu/ecen5823/home).

Below is an overview of the sequence used to generate this repository:
* The project was generated starting with the new project Wizard built into [Simplicity Studio 4](https://www.silabs.com/products/development-tools/software/simplicity-studio).  
* The AppBuilder project was used with application type "Bluetooth SDK" and application type "SOC - Empty".
* Configurations were setup to target GNU Arm 4.9.3 and 7.2.1.  4.9.3 is set as the default project target due to existing project code file support requirements.
* Workspace paths were setup to pull in emlib functions needed for upcoming assignments.
* Relevant emlib project files were copied from SiliconLabs\SimplicityStudio\v4\developer\sdks\gecko_sdk_suite\v2.4\platform as needed and added into the respective directories at the root.
* The main.c file in the root folder was renamed [gecko_main.c](gecko_main.c).  Contents of the main while loop were moved into functions and the main() function was #ifdef'd out.
* The [src](src) subfolder was added to contain code specific to the ECEN 5823 course and source files were added to support ECEN 5823 and the simplicity studio exercise assignment.
* The [questions](questions) subfolder was added to contain questions to be answered for each assignment, along with the questions for the first assignment.
# [](https://github.com/HARSH1911RATHORE/IOT_final_project_spring_2020/tree/1262f5e660c0273681406807c0b4457386b7123b#iot_final_project_spring_2020)IOT_final_project_spring_2020

# [](https://github.com/HARSH1911RATHORE/IOT_final_project_spring_2020/tree/1262f5e660c0273681406807c0b4457386b7123b#pollution-monituring-system)Pollution Monituring System

## [](https://github.com/HARSH1911RATHORE/IOT_final_project_spring_2020/tree/1262f5e660c0273681406807c0b4457386b7123b#project-status)Project Status

-   The Air quality sensor has been interfaced to the board with Pin number P31 for PD 13, SCL and Pin number P33 for PD 14, SDA. State machine for Humidity based sensor was created with data being measured in no hold master mode .
-   Gpio even based interrupt was configured which gets the status of button press. This button press will be used for bonding the board with the client to prevent man in the middle attack.
-   Persistent data was stored in the flash. The data will be stored for every value of data which exceeds the previous maximum value for that particular sensor. This data will be displayed on the server board every time a external button pb0 is pressed.
-   Communication was established between the board and the mobile application and was verified

--Sensor air quality ccs811 yet to work and give proper readings.

The repository contains the following files:

-   Server.c - Contains code for ble server which will update the data captured from sensors to the client.

-   Client.c - Contains code for ble client which will get updated with new data from the server.

-   state_machine.c - Contains code for state machine implementation for humidity and aqi sensor

-   gpio.c - Contains code for button interrupt and pin mode set and configuration

-   I2C_INTERRUPT.c - Contains code for i2c based interrupt functions to write, read and the i2c0 interrupt handler

-   i2c.c - Contains code for polling based implementation of i2c

-   display.c - Contains code for lcd display

-   log.c - contains initialization and functions for logging

-   letimer.c - Contains code for letimer functions initialization including clock. Also contains the letimer irq handler


## [](https://github.com/HARSH1911RATHORE/IOT_final_project_spring_2020/tree/1262f5e660c0273681406807c0b4457386b7123b#google-drive-link)[Google drive link](https://drive.google.com/drive/u/1/folders/1FM8G4nL0fplihENYMFtwnbRBXwX1w5KG)
