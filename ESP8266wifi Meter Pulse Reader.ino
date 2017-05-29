
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <TimeLib.h>

const char ssid[] = "Insert your SSID here";
const char pass[] = "Insert your SSID password here";

//Meter Pulse
long pulseCount = 0, lastPulseCount = 0;
unsigned long pulseTime, lastTime, savedTime;
double power, TotalkWh, lastpower;
int ppwh = 1; //1000 pulses/kwh = 1 pulse per wh
int pulse = 0;

int sw = 0; // 0 - serial connected, 1 - serial disconected. Must be disconnected from mini USB otherwise program will hang.
String result;
String Directory = "GET /service/r2/addstatus.jsp";
String MySID = "51923";// your system ID for pvoutput must be placed here.
String MyKey = "1ca25ec13241e202c4bc939fc4a1323b050ba8bd"; // your SSID key for pvoutput must be placed here.
String months, days, hours, mins, EG, PG, EU, PU, Temp, volts, IMP;

const unsigned int meter_pulses = 1000; // set for your meter eg 1000/kWh, 800/kWh for Sprint meters

// Create an instance of the server and specify the port to listen on

WiFiServer server(80);

char servername[] = "pvoutput.org"; // remote server we will connect to

static const char ntpServerName[] = "us.pool.ntp.org";
const int timeZone = +10;     // AUSTRALIA EST

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

time_t getNtpTime();
void digitalClockDisplay();
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);

WiFiClient client;

void deleteHttpHeader()//pvoutput
{
  if (result.endsWith("Content-Type: text/plain")) result = "";
}

void sendGET() //pvoutput
{
  time_t prevDisplay = 0; // when the digital clock was displayed
  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) { //update the display only if time has changed
      prevDisplay = now();
      if (sw == 0) digitalClockDisplay();
    }
  }
  if (month() < 10) months = "0" + String(month());
  else months = String(month());
  if (day() < 10) days = "0" + String(day());
  else days = String(day());
  if (hour() < 10) hours = "0" + String(hour());
  else hours = String(hour());
  if (minute() < 10) mins = "0" + String(minute());
  else mins = String(minute());
  if (sw == 0) {
    Serial.println("DATE: " + String(year()) + "/" + months + "/" + days);
    Serial.println("TIME: " + hours + ":" + mins);
  }
  if (client.connect(servername, 80)) {  //Send to pvoutput
    if (sw == 0) {
      Serial.println("connected");
      Serial.println(Directory + "?key=" + MyKey + "&sid=" + MySID + "&d=" + String(year()) + months + days + "&t=" + hours + ":" + mins + "&v1=" + EG + "&v2=" + PG + "&v3=" + EU + "&v4=" + PU + "&v5=" + Temp + "&v6=" + volts + "&v7=" + IMP);
    }
    client.println(Directory + "?key=" + MyKey + "&sid=" + MySID + "&d=" + String(year()) + months + days + "&t=" + hours + ":" + mins + "&v1=" + EG + "&v2=" + PG + "&v3=" + EU + "&v4=" + PU + "&v5=" + Temp + "&v6=" + volts + "&v7=" + IMP);
    client.println(); //end of get request
  }
  else {
    if (sw == 0) {
      Serial.println("connection failed"); //error message if no client connect
      Serial.println();
    }
  }
  delay(1000);
  while (client.connected() && !client.available()) delay(1); //waits for data
  while (client.connected() || client.available()) { //connected or data available
    char c = client.read(); //gets byte from ethernet buffer from pvoutput
    result = result + c;
    deleteHttpHeader();
  }
  if (sw == 0) {
    Serial.print("Received from server: ");
    Serial.println(result);
  }
  client.stop(); //stop client
}

void meterupdate() {
  cli();//disable interrupts
  if (pulse == 1)   {
    TotalkWh = (1.0 * pulseCount / (ppwh * 1000)); //multiply by 1000 to convert pulses per wh to kwh
    power = (3600000000.0 / (pulseTime - lastTime)) / ppwh;
    pulse = 0;
  }
  EG = 1; //Energy Generated
  PG = 1;  //Power Generated
  EU = (TotalkWh * 1000);  //Energy Used
  PU = power;  //Power Used
  Temp = 1;  // Temperature
  volts = 1;  // AC volts
  IMP = 1;  // Grid impedance
  sei();    //enable interrupts
  if (sw == 0) {
    if (pulseCount > lastPulseCount) {
      Serial.print("Pulses: ");
      Serial.println(pulseCount);
      Serial.print("Time between IRS: ");
      Serial.println(pulseTime - lastTime);
      Serial.print("Power: ");
      Serial.print(power, 0);
      Serial.print(" watts");
      Serial.println(" ");
      Serial.print("Energy Total: ");
      Serial.print(TotalkWh, 3);
      Serial.println(" kwh");
      Serial.print("Time: ");
      Serial.print(hour());
      Serial.print(":");
      Serial.print(minute());
      Serial.print(":");
      Serial.println(second());
      lastPulseCount = pulseCount;
    }
  }
}

void heartbeat() {
  for (int x = 0; x <= 30; x++) {
    meterupdate();
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);   // Turn the LED on (Note that LOW is the voltage level
    meterupdate();
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);  // Turn the LED off by making the voltage LOW
    meterupdate();
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);   // Turn the LED on (Note that LOW is the voltage level
    meterupdate();
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);  // Turn the LED off by making the voltage LOW
    meterupdate();
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
    meterupdate();
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    meterupdate();
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);   // Turn the LED on (Note that LOW is the voltage level
    meterupdate();
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);  // Turn the LED off by making the voltage LOW
    meterupdate();
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);   // Turn the LED on (Note that LOW is the voltage level
    meterupdate();
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);  // Turn the LED off by making the voltage LOW
  }
}

void setup() {  //Meter Pulse.
  // KWH interrupt attached to IRQ 1 = pin4
  attachInterrupt(5, onPulse, FALLING);

  pinMode(LED_BUILTIN, OUTPUT);     // Initialize HEARTBEAT
  if (sw == 0) Serial.begin(115200);
  delay(250);
  if (sw == 0) {
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
  }
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (sw == 0) Serial.print(".");
  }
  //timeclock
  if (sw == 0) {
    Serial.print("IP number assigned by DHCP is ");
    Serial.println(WiFi.localIP());
    Serial.println("Starting UDP");
  }
  Udp.begin(localPort);
  if (sw == 0) {
    Serial.print("Local port: ");
    Serial.println(Udp.localPort());
    Serial.println("waiting for sync");
  }
  setSyncProvider(getNtpTime);
  setSyncInterval(300);
  if (sw == 0) {
    Serial.println("END SETUP");
    Serial.println(" ");
  }
  result = "";
  delay(1000);

}//end of setup




void loop() {
  heartbeat();
  sendGET();
  if (sw == 0) digitalClockDisplay();
}


// Meter - The interrupt routine
void onPulse()
{
  savedTime = lastTime;
  lastTime = pulseTime;  //used to measure time between pulses.
  pulseTime = micros();
  if ((pulseTime - lastTime) > 180000) {  //180mS filter for max power of 20kw reading = 3600000000/20000 = 180000uS = 180mS
    pulse = 1;
    pulseCount++;  // Count of the number of pulses since program started.
  }
  else {
    pulseTime = lastTime;
    lastTime = savedTime;
  }
}

void digitalClockDisplay()//timeclock
{
  // digital clock display of the time
  if (sw == 0) {
    Serial.print(hour());
    printDigits(minute());
    printDigits(second());
    Serial.println(" " + String(day()) + "." + String(month()) + "." + String(year()) + " ");
  }
}

void printDigits(int digits)//timeclock
{
  // utility for digital clock display: prints preceding colon and leading 0
  if (sw == 0) Serial.print(":");
  if (digits < 10) {
    if (sw == 0) Serial.print('0');
  }
  if (sw == 0) Serial.print(digits);
}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  if (sw == 0) {
    Serial.println(" ");
    Serial.println("Transmit NTP Request");
  }
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  if (sw == 0) {
    Serial.print(ntpServerName);
    Serial.print(": ");
    Serial.println(ntpServerIP);
  }
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      if (sw == 0) Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  if (sw == 0) Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

