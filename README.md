# Plant Watering with ESP8266 and Blynk App
This project was created to fill in the gap we had while going for holidays: Our flowers and plants needed to be watered and I was too busy to create a complex system.
I decided to uyse the simplicity of Blynk as it requires low to almost no need of messing up with code and hardware.  
Apart from controlling the pump, I also added a temperature sensor and a soil moisture sensor.

## What you will need:
   - 1x NodeMCU or similar ESP8266 unit with access to I/O
   - 1x Relay board (to control the water pump)
   - 1x DS18S20 1-Wire digital thermometer (I used the ones soldered already on a long cable)
   - 1x Soil moisture sensor (analog, with 3 pins)
   - 1x Mini water pump (12V)
   - 2x Power supply (5V USB for the NodeMCU + 12V for the water pump)
   - *OPTIONAL:* 1x Voltage regulator module (in case you just want to use a 12V power supply and generate the 5V from it)

## Requirements for programming your ESP8266:
   - [Arduino IDE](https://www.arduino.cc/en/Main/Software)
   - [ESP8266 Board manager](https://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/)
   - [Blynk library](http://help.blynk.cc/en/articles/512105-how-to-install-blynk-library-for-arduino)
   - Blynk account on your phone and an Authentication Token for your project
   