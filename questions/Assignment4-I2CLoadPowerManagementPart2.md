Please include your answers to the questions below with your submission, entering into the space below each question
See [Mastering Markdown](https://guides.github.com/features/mastering-markdown/) for github markdown formatting if desired.

*Be sure to take measurements in the "Default" configuration of the profiler to ensure your logging logic is not impacting current/time measurements.*

*Please include screenshots of the profiler window detailing each current measurement captured.  See [Shared document](https://docs.google.com/document/d/1Ro9G2Nsr_ZXDhBYJ6YyF9CPivb--6UjhHRmVhDGySag/edit?usp=sharing) for instructions.* 

1. What is the average current per period?
   Answer:
   <br>Screenshot: The average current per period is 26.55uA
   ![avg_current_per_period](screenshots/assignment4/avg_current_per_period.jpg)  

2. What is the average current when the Si7021 is Load Power Management OFF?
   Answer:
   <br>Screenshot:  The avg current when load power management is off is 4.97uA
   ![Avg_current_LPM_Off](screenshots/assignment4/avg_current_lpm_off.jpg)  

3. What is the average current when the Si7021 is Load Power Management ON?
   Answer:
   <br>Screenshot:  The avg current when load power management is on is 351.68uA
   ![Avg_current_LPM_Off](screenshots/assignment4/avg_current_lpm_on.jpg)  

4. How long is the Si7021 Load Power Management ON for 1 temperature reading?
   Answer:
   <br>Screenshot:  The load power management is on for 96 ms for 1 temperature reading
   ![duration_lpm_on](screenshots/assignment4/avg_current_lpm_on.jpg)  

5. What is the total operating time of your design for assignment 4 in hours assuming a 1000mAh supply?
ans. 1000mAh/26.55uA=37,664.7834 hours is the total operating time of my design

6. How has the power consumption performance of your design changed since the previous assignment?
ans.     88.39uW Is the avg power consumed compared to 495.27 uW in previous assignment

7. Describe how you have tested your code to ensure you are sleeping in EM1 mode during I2C transfers.
 ans.   I tested my code by checking the Sleep_sleep return value to see if i2c is going in EM1
