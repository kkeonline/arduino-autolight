# arduino-autolight
Auto light
This is my Arduino project to turn on a light at night and turn off at morning (with ldr sensor)
with motion detect to turn on another light for a few seconds (only at night time)
Hardware:
1. Arduino board (Nano V3)
2. Relay module 2ch 5V (active low)
3. RTC module I2C (DS3231)
4. PIR module (SC-HR501)
5. LDR + 10K VR
6. 2 pin jumper or DIP switch (for debug status)
7. AC-DC switching 5V 650mA

Note: you have to set Date/Time to RTC before using it with the project.
