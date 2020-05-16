# GPS GPRS Tracking system for covid-19 patient

this project is based on Atmega328p SIM28 as a GPS sensor and SIM800 for sending data to firebase (remote server) and it is battery powered device .

[alt text](https://github.com/jawher-Mansour/pics/blob/master/Track1.jpg")
[alt text](https://github.com/jawher-Mansour/pics/blob/master/Track.jpg")

###Requirements

*first of all you need to upload some js functions to firebase , you can find those functions in this [repo](https://github.com/jawher-Mansour/GPRS-Firebase)
*arduino IDE or any other IDE for AVR microcontrollers
*tiny GPS library 

###work to do 
*include function to send PMTK commands to SIM28 in order to improve position accuracy
*include function to enter the ATmega328p in deep sleep mode to improve the tracking unit autonomy 
