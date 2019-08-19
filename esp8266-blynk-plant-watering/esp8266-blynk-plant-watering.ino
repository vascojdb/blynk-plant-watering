/*************************************************************
  Date: 1st August 2019
  Made by: Vasco Jose Dias Baptista
  GitHub: https://github.com/vascojdb/blynk-plant-watering
 *************************************************************/
#define BLYNK_PRINT Serial  // Comment this out to disable prints and save space

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DS18B20.h>

// ==================== EDIT YOUR SETTINGS FROM HERE ====================
// Code settings:
#define USE_TEMP_SENSOR                         // Comment this out if you are not using a 1-Wire temperature sensor (DS18B20)
#define USE_SOIL_MOISTURE_SENSOR                // Comment this out if you are not using an analog (3 wire) soil moisture sensor

// Physical settings:
#define PUMP_PIN                        4       // Physical pin where you connected the pump relay/driver
#define PUMP_LOGIC_INVERTED             false   // Select true if the pump turns ON when the output is low (0V)
#define TEMP_SENSOR_PIN                 14      // Physical pin where you connected the 1-Wire temperature sensor
#define MOISTURE_SENSOR_POWER_PIN       5       // Physical ping where you connect the soil moisture sensor power (+) pin. Analog is connected to AN0

// Blynk App settings:
#define BLYNK_APP_PUMPTIMERVALUE_VPIN   V0      // Pump timer value (in seconds) tied to a slider (values: from 1 to a reasonable number like 600)
#define BLYNK_APP_PUMPONTIMER_VPIN      V1      // Push button to turn pump ON for the timer value above (values: 0 or 1)
#define BLYNK_APP_PUMPONOFF_VPIN        V2      // ON/OFF button (or timer/scheduler) to turn pump ON/OFF manually (values: 0 or 1)
#define BLYNK_APP_PUMPTIMERSTATUS_VPIN  V3      // Status (duration set) of the pump timer (in seconds): (values: from 1 to a reasonable number)
#define BLYNK_APP_PUMPSTATUS_VPIN       V4      // Status of the pump every time its status changes (values: "OFF" or "ON"). In the app the reading frequency should be set to PUSH
#define BLYNK_APP_TEMP_VPIN             V5      // Value for temperature sensor. In the app the reading frequency should be set to PUSH
#define BLYNK_APP_SOILMOISTURE_VPIN     V6      // Value for the soil humidity. In the app the reading frequency should be set to PUSH

// You should get Auth Token in the Blynk App. Go to the Project Settings (nut icon).
char auth[] = "auth-token";

// Your WiFi credentials. Set password to "" for open networks.
char ssid[] = "supersecretwifi";
char pass[] = "supersecretpassword";
// ==================== EDIT YOUR SETTINGS TILL HERE ====================

#ifdef USE_TEMP_SENSOR
DS18B20         ds(TEMP_SENSOR_PIN);
#endif
BlynkTimer      timerSystem;
bool            readSensors_flag = false;
int             pumpOnTimeDuration = 1;
int             pumpOnTimer_numTimer = -1;
const unsigned  reading_count = 10;
unsigned int    analogVals[reading_count];
unsigned int    values_avg = 0;

// Create a delay function compatible with Blynk and timers:
void Blynk_Delay(int milli) {
    int end_time = millis() + milli;
    while(millis() < end_time) {
        Blynk.run();
        Blynk.run();
        timerSystem.run();
        yield();
    }
}

// Called to control the power of the pump relay/driver.
// Reports back to Blynk app as well:
void setPumpPower(bool power = false) {
    if (power) {
        Serial.println("Pump powered ON");
        Blynk.virtualWrite(BLYNK_APP_PUMPSTATUS_VPIN, "ON");
        if (PUMP_LOGIC_INVERTED) digitalWrite(PUMP_PIN, LOW);
        else digitalWrite(PUMP_PIN, HIGH);
    }
    else {
        Serial.println("Pump powered OFF");
        Blynk.virtualWrite(BLYNK_APP_PUMPSTATUS_VPIN, "OFF");
        if (PUMP_LOGIC_INVERTED) digitalWrite(PUMP_PIN, HIGH);
        else digitalWrite(PUMP_PIN, LOW);
    }
}

void setPumpTimerDuration(int duration) {
    if (duration < 1) duration = 1;
    pumpOnTimeDuration = duration;
    Blynk.virtualWrite(BLYNK_APP_PUMPTIMERSTATUS_VPIN, pumpOnTimeDuration);
    Serial.printf("Pump duration set to %u seconds\r\n", pumpOnTimeDuration);
}

void pumpTimerTimeout() {
    Serial.println("Timer expired");
    timerSystem.disable(pumpOnTimer_numTimer);
    timerSystem.deleteTimer(pumpOnTimer_numTimer);
    pumpOnTimer_numTimer = -1;
    setPumpPower(false);
}

// Called every time we change the duration time of the water pump
// through the app (for example can be tied to a slider):
BLYNK_WRITE(BLYNK_APP_PUMPTIMERVALUE_VPIN) {
    setPumpTimerDuration(param.asInt());
}

// Called every time we press the push button to turn the pump ON
// for the duration of time specified by 'pumpOnTimeDuration':
BLYNK_WRITE(BLYNK_APP_PUMPONTIMER_VPIN) {
    if (param.asInt() == 1) {
        if (pumpOnTimer_numTimer < 0) {
            Serial.printf("Starting timer of %u seconds\r\n", pumpOnTimeDuration);
            setPumpPower(true);
            pumpOnTimer_numTimer = timerSystem.setTimeout(pumpOnTimeDuration*1000L, pumpTimerTimeout);
        }
        else {
            Serial.println("Pump was already on a timer. Ending timer...");
            pumpTimerTimeout();
        }
    }
}

// Called every time we press the ON/OFF button to turn the pump ON or OFF:
BLYNK_WRITE(BLYNK_APP_PUMPONOFF_VPIN) {
    if (param.asInt() == 0) setPumpPower(false);
    else setPumpPower(true);
}

// Called to measure and average the soil moisture level:
#ifdef USE_SOIL_MOISTURE_SENSOR
int getSoilMoisture() {
    // Turn the sensor ON:
    digitalWrite(MOISTURE_SENSOR_POWER_PIN, HIGH);
    Blynk_Delay(1000);
    
    for (int counter = 0; counter < reading_count; counter++) {
        analogVals[reading_count] = analogRead(A0);
        Blynk_Delay(100);
        values_avg = (values_avg + analogVals[reading_count]);
    }
    values_avg = values_avg/reading_count;

    // Turn the sensor OFF:
    digitalWrite(MOISTURE_SENSOR_POWER_PIN, LOW);
    return values_avg;
}
#endif

// Called every time the timer to read sensors expires:
void readSensors() {
    readSensors_flag = true;
}

// Setup section:
void setup() {
    // Debug console
    Serial.begin(115200);
    
    Serial.println("Preparing...");
    
    // Set pins:
    pinMode(PUMP_PIN, OUTPUT);
    if (PUMP_LOGIC_INVERTED) digitalWrite(PUMP_PIN, HIGH);
    else digitalWrite(PUMP_PIN, LOW);
    #ifdef USE_TEMP_SENSOR
    pinMode(TEMP_SENSOR_PIN, INPUT_PULLUP);
    #endif
    #ifdef USE_SOIL_MOISTURE_SENSOR
    pinMode(MOISTURE_SENSOR_POWER_PIN, OUTPUT);
    digitalWrite(MOISTURE_SENSOR_POWER_PIN, LOW);
    #endif

    // Start connection to WiFi and Blynk system:
    Blynk.begin(auth, ssid, pass);
    
    // Set initial status, which will also report to Blynk:
    setPumpTimerDuration(1);
    setPumpPower(false);
    
    #ifdef USE_TEMP_SENSOR
    // Select the temperature sensor and set parameters:
    uint8_t temp_sensor_addr[8];
    while (ds.selectNext()) {
        ds.getAddress(temp_sensor_addr);
    }
    ds.select(temp_sensor_addr);
    ds.setResolution(10);
    #endif
    
    // Read the sensors every 60 seconds:
    timerSystem.setInterval(60000L, readSensors);
    
    Serial.println("Ready...");
}

// Loop section:
void loop() {
    Blynk.run();
    timerSystem.run();
    
    // Read sensors here as soon as the flag is set:
    // Reads all the sensors and reports back to Blynk App
    if (readSensors_flag) {
        #ifdef USE_TEMP_SENSOR
        float temperature = ds.getTempC();
        Serial.print("Read temperature: ");
        Serial.println(temperature);
        Blynk.virtualWrite(BLYNK_APP_TEMP_VPIN, temperature);
        #endif
        #ifdef USE_SOIL_MOISTURE_SENSOR
        int soilMoisture = getSoilMoisture();
        Serial.print("Read soil moisture: ");
        Serial.println(soilMoisture);
        Blynk.virtualWrite(BLYNK_APP_SOILMOISTURE_VPIN, soilMoisture);
        #endif
        readSensors_flag = false;
    }
}
