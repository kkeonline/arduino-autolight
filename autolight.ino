#include "Wire.h"
#define DS3231_I2C_ADDRESS 0x68

/* Connection Pin
 * A4 = RTC SDA
 * A5 = RTC SCL
 * A0 = LDR
 * D2 = Motion 1
 * D6 = Relay 1 
 * D8 = Relay 2
 * D4 = debug sw
*/

#define ledPin 13
#define ldrPin A0
#define pirPin1 2
#define debugPin 4
#define relayPin1 6
#define relayPin2 8

// Value reading from LDR via A0 pin to trig Light on/off
int uplimit = 920; // value to turn off light in Morning
int lowlimit = 860; // value to turn on light in Evening

// Active Hour
int Morning = 5;
int Evening = 18;

int var = 0;
bool dark = false;
bool night = false;
bool willday = true;
bool willnight = true;

bool debug = false;

void debugMsg(String myString, bool ln=true) {
 if (debug) {
   if (ln) {
     Serial.println(myString);
   } else {
     Serial.print(myString);
   }
 }
}

void setup()
{
  Wire.begin();
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(ldrPin, INPUT);
  pinMode(pirPin1, INPUT);
  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  pinMode(debugPin, INPUT_PULLUP);
  digitalWrite(ledPin, LOW);
  digitalWrite(relayPin1, HIGH);
  digitalWrite(relayPin2, HIGH);
  Serial.println("To turn on debug message, Set D4 pin to LOW.");
  delay(200);
  debug = !digitalRead(debugPin);
  if (debug) {
      delay(1000);
      displayTime(); // display the real-time clock data on the Serial Monitor,
  }
}

// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return( (val/16*10) + (val%16) );
}

void readDS3231time(byte *second,
byte *minute,
byte *hour,
byte *dayOfWeek,
byte *dayOfMonth,
byte *month,
byte *year)
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}

int checkLight()
{
  int light = 0;
  int avglight = 0;
  for (int i=0; i<10; i++) {
       light = analogRead(ldrPin);
       avglight += light;
       debugMsg(String(light), false);
       debugMsg(", ", false);
       delay(100);
  }
  avglight = avglight / 10;
  debugMsg(String(avglight));
  return avglight;
}

void checkTime()
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month,
  &year);

  if ((hour > Morning) and (hour < Evening)) {
     night = false;
     willnight = true;
     debugMsg("Day time (06:00-17:59): night=false, willnight=true");
  }
  if ((hour > Evening) or (hour < Morning)) {
     night = true;
     willday = true;
     debugMsg("Night time (19:00-04:59): night=true, willday=true");
  }
  if ((hour == Morning) and willday) {
     debugMsg("Morning time (05:xx): ", false);
     if (checkLight() < uplimit) {
         dark = false;
     } else {
         dark = true;
     }

     if (!dark) {
        willday = false;
        night = false;
        debugMsg("LDR Not Dark, willday=false, night=false");
     } else {
        setrelay1(LOW);
        night = true;
        debugMsg("LDR Dark, Turn Light ON, night=true");
     }
  }
  if ((hour == Evening) and willnight) {
     debugMsg("Evening time (18:xx): ", false);
     if (checkLight() > lowlimit) {
         dark = true;
     } else {
         dark = false;
     }
     
     if (dark) {
        willnight = false;
        night = true;
        debugMsg("LDR Dark, willnight=false, night=true");
     } else {
        debugMsg("LDR not Dark, skip");
     }
  }
}

void setrelay1(int val)
{
   digitalWrite(relayPin1, val);
}

void setrelay2(int val)
{
   digitalWrite(relayPin2, val);
}

void displayTime()
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month,
  &year);
  // send it to the serial monitor
  Serial.println("");
  Serial.print("RTC current time: ");
  Serial.print(hour, DEC);
  // convert the byte variable to a decimal number when displayed
  Serial.print(":");
  if (minute<10)
  {
    Serial.print("0");
  }
  Serial.print(minute, DEC);
  Serial.print(":");
  if (second<10)
  {
    Serial.print("0");
  }
  Serial.print(second, DEC);
  Serial.print(" ");
  Serial.print(dayOfMonth, DEC);
  Serial.print("/");
  Serial.print(month, DEC);
  Serial.print("/");
  Serial.print(year, DEC);
  Serial.print(" Day of week: ");
  switch(dayOfWeek){
  case 1:
    Serial.print("Sunday");
    break;
  case 2:
    Serial.print("Monday");
    break;
  case 3:
    Serial.print("Tuesday");
    break;
  case 4:
    Serial.print("Wednesday");
    break;
  case 5:
    Serial.print("Thursday");
    break;
  case 6:
    Serial.print("Friday");
    break;
  case 7:
    Serial.print("Saturday");
    break;
  }
  Serial.println("");
}

void loop()
{
int j;
/*
  // Check if any message send via serial to trig debug mode on/off
  var = Serial.read();
  if (var > 0) {
     while(var > 0) {
        var = Serial.read();
     }
     debug = !debug;
     if (debug) {
      Serial.println("Debug mode: ON");
      Serial.println("To turn off debug mode, send any message via serial again.");
     } else {
      Serial.println("Debug mode: OFF");
      Serial.println("To turn on debug mode, send any message via serial again.");
     }
  }
*/

  if (debug == digitalRead(debugPin)) {
     debug = !debug;
     if (debug) {
      Serial.println("Debug mode: ON");
      Serial.println("To turn off debug message, Set D4 pin to HIGH.");
     } else {
      Serial.println("Debug mode: OFF");
      Serial.println("To turn on debug message, Set D4 pin to LOW.");
     }
  }
  
  if (debug) {
      displayTime(); // display the real-time clock data on the Serial Monitor,
  }

  checkTime();
  if (night) {
      setrelay1(LOW);
      debugMsg("Night time: Turn Light ON");

    if (debug) {
        j=10;
       } else {
        j=200;
       }
    for (int i=0; i<j; i++) {
      if (digitalRead(pirPin1) == HIGH) {
          digitalWrite(ledPin, HIGH);
          setrelay2(LOW);
          debugMsg("Motion detected: Turn Light ON");
      } else {
          digitalWrite(ledPin, LOW);
          setrelay2(HIGH);
          debugMsg("Motion not detected: Turn Light OFF");
      }
    delay(300);
    }
  } else {
      setrelay1(HIGH);
      setrelay2(HIGH);
      debugMsg("Day time: Turn Light OFF, sleep 5 Minutes.");
      if (debug) {
        checkLight();
        delay(3000);
      } else {
        delay(180000);
      }
  }
}
