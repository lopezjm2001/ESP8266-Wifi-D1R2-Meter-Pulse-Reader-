 # ESP8266-Wifi-D1R2-Meter-Pulse-Reader-Sketch
This project is about reading the light pulses from an electricity meter e.g. Elster A1100. The light pulses will give the user the values of power and energy being imported or exported at the time.
This project uses an Arduino ESP8266 Wifi D1 from Wemos like this one http://www.instructables.com/id/Programming-the-WeMos-Using-Arduino-SoftwareIDE/.
And an Arduino light photosensitive sensor with digital output 3.3volts from ebay like this one http://www.ebay.com.au/itm/161869818445?_trksid=p2060353.m1438.l2649&ssPageName=STRK%3AMEBIDX%3AIT. 
Need to add ZIP files to Arduino program library for the sketch to work from here: i.e.https://github.com/PaulStoffregen/Time/tree/18ce15b202641771eec6ec3f4687aead5069d2d8 and
https://github.com/arduino-libraries/NTPClient/tree/7016636dba0270c16fc639bb5612b07171826ddf
The sketch downloads the current time from the internet. And every five minutes will send the power (watts) and energy (kwh) to www.pvoutput.org. The user will need to register an output with www.pvoutput.org and get an API key and System ID.  
