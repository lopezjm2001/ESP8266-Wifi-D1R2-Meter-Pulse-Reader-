ESP8266-Wifi-D1R2-Meter-Pulse-Reader-Sketch
This project is about reading the red light pulses from a digital electricity meter e.g. Elster A1100. The light pulses will give the user the values of power and energy being imported or exported at the moment of time. 1000 pulses = 1kWh, The time measured between pulses gives the current power reading. The Elster A1100 meter is a nett meter and will produce light pulses for both import and export. 

The Energy KWH is derived simply by adding the number of pulses over a period of time. 1kwh = 1000 pulses.

Hence power is calculated over a period of time between any two pulses.

Energy(1kwh) = Power(1kw) x time(1hour)
Power(1kW) = Energy(1kWh) / time(1hour), 
1 hour = 60min x 60sec = 3600 seconds = 3600000 milliSeconds = 3600000000 microSeconds
power(watts) = (3600000000.0 / (pulseTime(micro seconds) - lastTime(micro seconds)) / ppws
1kwh = 1000 pulses
ppws(pulse per watt second) = 1 

This project uses an Arduino ESP8266 Wifi D1 from Wemos like this one http://www.instructables.com/id/Programming-the-WeMos-Using-Arduino-SoftwareIDE/.
And an Arduino light photosensitive sensor with a digital output 3.3volts from ebay like this one http://www.ebay.com.au/itm/161869818445?_trksid=p2060353.m1438.l2649&ssPageName=STRK%3AMEBIDX%3AIT. 

A digital input interrupt request is used on the falling edge of the pulse signals. The time is measured between falling edges in micro-seconds. A software filter is used to ignore falling edges from noise if the measured time between pulses is less than 180mS. 180mS is set for a maximum 20kw power reading.

Wiring connections (see attached photos of meter) :

1. Yellow wire - The digital output (DO) of light photosensor module is connected to pin 4(SCL/D1) of the ESP8266wifi D1R2.
2. Red wire - The 3.3V positive supply, (VCC) of photosensor module to (3.3v) on the ESP8266wifi D1R2. 
3. Black wire - ground wire connects to (GND) between photosensor module and the ESP8266wifi D1R2.

Need to add ZIP files to Arduino program library for the sketch to work, from the two links below: 

https://github.com/PaulStoffregen/Time/tree/18ce15b202641771eec6ec3f4687aead5069d2d8 and
https://github.com/arduino-libraries/NTPClient/tree/7016636dba0270c16fc639bb5612b07171826ddf

The sketch downloads the current time from the internet through your wifi router. And every five minutes it will send the current power (watts) and total energy (kwh) to www.pvoutput.org. 

The user will need to register an output with www.pvoutput.org and get an API key and System ID.  
