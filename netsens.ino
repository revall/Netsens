#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <NTPClient.h>
#include <Wire.h>
#include "NS_EEPROM.h"
#include "NS_HTTP.h"
#include "netsens.h"


#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);


#include <SPI.h>
#include "SSD1306.h"
#include "SSD1306Ui.h"
#include "images.h"
// Pin definitions for I2C
#define OLED_SDA    D1  // pin 14
#define OLED_SDC    D2  // pin 12
#define OLED_ADDR   0x3C
SSD1306   display(OLED_ADDR, 5, 4);    // For I2C
SSD1306Ui ui     ( &display );

// Test code
bool msOverlay(SSD1306 *display, SSD1306UiState* state) {
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(128, 0, String(millis()));
  return true;
}

bool drawFrame1(SSD1306 *display, SSD1306UiState* state, int x, int y) {
  // draw an xbm image.
  // Please note that everything that should be transitioned
  // needs to be drawn relative to x and y

  // if this frame need to be refreshed at the targetFPS you need to
  // return true
  display->drawXbm(x + 34, y + 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
  return false;
}

bool drawFrame2(SSD1306 *display, SSD1306UiState* state, int x, int y) {
  // Demonstrates the 3 included default sizes. The fonts come from SSD1306Fonts.h file
  // Besides the default fonts there will be a program to convert TrueType fonts into this format
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(0 + x, 10 + y, "Arial 10");

  display->setFont(ArialMT_Plain_16);
  display->drawString(0 + x, 20 + y, "Arial 16");

  display->setFont(ArialMT_Plain_24);
  display->drawString(0 + x, 34 + y, "Arial 24");

//  return false;
}

bool drawFrame3(SSD1306 *display, SSD1306UiState* state, int x, int y) {
  // Text alignment demo
  display->setFont(ArialMT_Plain_10);

  // The coordinates define the left starting point of the text
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0 + x, 11 + y, "Left aligned (0,10)");

  // The coordinates define the center of the text
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 22, "Center aligned (64,22)");

  // The coordinates define the right end of the text
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(128 + x, 33, "Right aligned (128,33)");
  return false;
}

bool drawFrame4(SSD1306 *display, SSD1306UiState* state, int x, int y) {
  // Demo for drawStringMaxWidth:
  // with the third parameter you can define the width after which words will be wrapped.
  // Currently only spaces and "-" are allowed for wrapping
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawStringMaxWidth(0 + x, 10 + y, 128, "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore.");
  return false;
  
}

// this array keeps function pointers to all frames
// frames are the single views that slide from right to left
bool (*frames[])(SSD1306 *display, SSD1306UiState* state, int x, int y) = { drawFrame1, drawFrame2, drawFrame3, drawFrame4 };

// how many frames are there?
int frameCount = 4;

bool (*overlays[])(SSD1306 *display, SSD1306UiState* state)             = { msOverlay };
int overlaysCount = 1;


// DHT sensor config
#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE, 20);


// Relay pin config
#define BUILTIN_LED  16
#define RESET_BUTTON_PIN 14
#define RELAY_PIN  16


WiFiClient espClient;
PubSubClient mqttClient(espClient);
NTPClient timeClient();
//NTPClient timeClient;
long lastMsg = 0;
int value = 0;
char Tmsg[50];
char Hmsg[50];
char Pmsg[50];
char  CurentTime[50];





/**
 * Соединение с WiFi
 */
bool ConnectWiFi(void) {

  // Если WiFi не сконфигурирован
  if ( strcmp(EC_Config.AP_SSID, "none")==0 ) {
     Serial.printf("WiFi is not config ...\n");
     return false;
  }

  WiFi.mode(WIFI_STA);

  // Пытаемся соединиться с точкой доступа
  Serial.printf("\nConnecting to: %s/%s\n", EC_Config.AP_SSID, EC_Config.AP_PASS);
  WiFi.begin(EC_Config.AP_SSID, EC_Config.AP_PASS);
  delay(1000);

  // Максиммум N раз проверка соединения (30 секунд)
  for ( int j = 0; j < 30; j++ ) {
  if (WiFi.status() == WL_CONNECTED) {
      Serial.print("\nWiFi connect: ");
      Serial.print(WiFi.localIP());
      Serial.print("/");
      Serial.print(WiFi.subnetMask());
      Serial.print("/");
      Serial.println(WiFi.gatewayIP());
      return true;
    }
    delay(1000);
    Serial.print(WiFi.status());
    Serial.print(" - ");
  }
  Serial.printf("\nConnect WiFi failed ...\n");
  return false;
}

/**
 * Старт WiFi
 */
void WiFi_begin(void){
// Подключаемся к WiFi
  isAP = false;
  if( ! ConnectWiFi()  ){
      Serial.printf("Start AP %s\n",EC_Config.ESP_NAME);
      WiFi.mode(WIFI_STA);
      WiFi.softAP(EC_Config.ESP_NAME);
      isAP = true;
      Serial.printf("Open http://192.168.4.1/config in your browser\n");
  }
  else {
// Получаем статический IP если нужно
      if( EC_Config.IP != 0 ){
         WiFi.config(EC_Config.IP,EC_Config.GW,EC_Config.MASK);
         // Запускаем MDNS
             MDNS.begin(EC_Config.ESP_NAME);
             Serial.print("Open http://");
             Serial.print(WiFi.localIP());
             Serial.println(" in your browser");
             Serial.printf("Or by name: http://%s.local\n",EC_Config.ESP_NAME);
                 }
   }


}



void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] \t");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, HIGH);
    digitalWrite(RELAY_PIN, HIGH);
     Serial.print("ON\n");
    // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {

  digitalWrite(BUILTIN_LED,  LOW);
  digitalWrite(RELAY_PIN, LOW);
   Serial.print("OFF\n");
  }

}

void reset_to_default() {

  if (digitalRead(RESET_BUTTON_PIN ) == HIGH) {  // when button pressed pin should get low, button connected to ground
    Serial.println(F("Reset Button Pressed"));
    Serial.println(F("You have 5 seconds to Cancel"));
    Serial.println(F("This will be remove all records and cannot be undone"));
    delay(5000);                        // Give user enough time to cancel operation
    if (digitalRead(RESET_BUTTON_PIN) == HIGH) {    // If button still be pressed, wipe EEPROM
      Serial.println(F("Starting reset settings"));
      EC_default();
      EC_save();
      delay(100);
      }
    else {
      Serial.println(F("Reset Cancelled"));
      }
  }

}



void i2c_scan()
{
  String stringOne;
  byte error, address;
  int nDevices;
  char status;
  double T,P,p0,a;


  #define ALTITUDE 1655.0 // Altitude of SparkFun's HQ in Boulder, CO. in meters
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

if (error == 0)
  {
  String stringOne =  String(address, HEX);
  Serial.print("0x");     Serial.print(stringOne); Serial.print(" - ");
    if(stringOne=="0A") Serial.println("'Motor Driver'");
    if(stringOne=="0F") Serial.println("'Motor Driver'");
    if(stringOne=="1D") Serial.println("'ADXL345 Input 3-Axis Digital Accelerometer'");
    if(stringOne=="1E") Serial.println("'HMC5883 3-Axis Digital Compass'");
    if(stringOne=="5A") Serial.println("'Touch Sensor'");
    if(stringOne=="5B") Serial.println("'Touch Sensor'");
    if(stringOne=="5C") Serial.println("'BH1750FVI digital Light Sensor' OR 'Touch Sensor"  );
    if(stringOne=="5D") Serial.println("'Touch Sensor'");
    if(stringOne=="20") Serial.println("'PCF8574 8-Bit I/O Expander' OR 'LCM1602 LCD Adapter' ");
    if(stringOne=="21") Serial.println("'PCF8574 8-Bit I/O Expander'");
    if(stringOne=="22") Serial.println("'PCF8574 8-Bit I/O Expander'");
    if(stringOne=="23") Serial.println("'PCF8574 8-Bit I/O Expander' OR 'BH1750FVI digital Light Sensor'");
    if(stringOne=="24") Serial.println("'PCF8574 8-Bit I/O Expander'");
    if(stringOne=="25") Serial.println("'PCF8574 8-Bit I/O Expander'");
    if(stringOne=="26") Serial.println("'PCF8574 8-Bit I/O Expander'");
    if(stringOne=="27") Serial.println("'PCF8574 8-Bit I/O Expander' OR 'LCM1602 LCD Adapter '");
    if(stringOne=="39") Serial.println("'TSL2561 Ambient Light Sensor'");
    if(stringOne=="40") Serial.println("'BMP180 barometric pressure sensor'"    );
    if(stringOne=="48") Serial.println("'ADS1115 Module 16-Bit'");
    if(stringOne=="49") Serial.println("'ADS1115 Module 16-Bit' OR 'SPI-to-UART'");
    if(stringOne=="4A") Serial.println("'ADS1115 Module 16-Bit'");
    if(stringOne=="4B") Serial.println("'ADS1115 Module 16-Bit'");
    if(stringOne=="50") Serial.println("'AT24C32 EEPROM'");
    if(stringOne=="53") Serial.println("'ADXL345 Input 3-Axis Digital Accelerometer'");
    if(stringOne=="68") Serial.println("'DS3231 real-time clock' OR 'MPU-9250 Nine axis sensor module'");
    if(stringOne=="7A") Serial.println("'LCD OLED 128x64'");
    if(stringOne=="76") Serial.println("'BMP280 barometric pressure sensor'");
    if(stringOne=="77") Serial.println("'BMP180 barometric pressure sensor' OR 'BMP280 barometric pressure sensor'");
    if(stringOne=="3C") Serial.println("'LCD OLED 128x64'" );
   nDevices++;
  }
    else if (error==4)
    {
      Serial.print("Unknow error at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");


  delay(5000);
}

/**************************************************************************/
/*
    Displays some basic information on this sensor from the unified
    sensor API sensor_t type (see Adafruit_Sensor for more information)
*/
/**************************************************************************/
void displaySensorDetails(void)
{
  sensor_t sensor;
  bmp.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" hPa");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" hPa");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" hPa");
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}

void getPressure_test()
{
  /* Get a new sensor event */
  sensors_event_t event;
  bmp.getEvent(&event);

  /* Display the results (barometric pressure is measure in hPa) */
  if (event.pressure)
  {
    /* Display atmospheric pressue in hPa */
    Serial.print("Pressure:    ");
    Serial.print(event.pressure);
    Serial.println(" hPa");

    /* Calculating altitude with reasonable accuracy requires pressure    *
     * sea level pressure for your position at the moment the data is     *
     * converted, as well as the ambient temperature in degress           *
     * celcius.  If you don't have these values, a 'generic' value of     *
     * 1013.25 hPa can be used (defined as SENSORS_PRESSURE_SEALEVELHPA   *
     * in sensors.h), but this isn't ideal and will give variable         *
     * results from one day to the next.                                  *
     *                                                                    *
     * You can usually find the current SLP value by looking at weather   *
     * websites or from environmental information centers near any major  *
     * airport.                                                           *
     *                                                                    *
     * For example, for Paris, France you can check the current mean      *
     * pressure and sea level at: http://bit.ly/16Au8ol                   */

    /* First we get the current temperature from the BMP085 */
    float temperature;
    bmp.getTemperature(&temperature);
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" C");

    /* Then convert the atmospheric pressure, and SLP to altitude         */
    /* Update this next line with the current SLP for better results      */
    float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;
    Serial.print("Altitude:    ");
    Serial.print(bmp.pressureToAltitude(seaLevelPressure,
                                        event.pressure));
    Serial.println(" m");
    Serial.println("");
  }
  else
  {
    Serial.println("Sensor error");
  }
  delay(1000);
}


void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);
  digitalWrite(RELAY_PIN, HIGH);
  Serial.begin(115200);

  // Инициализация EEPROM
  EC_begin();
  // Reset to defaul procedure
  pinMode(RESET_BUTTON_PIN, INPUT);
  if (digitalRead(RESET_BUTTON_PIN ) == HIGH) reset_to_default();

  // INIT !!!
  EC_read();
  NTPClient timeClient(EC_Config.NTP_SERVER1,EC_Config.TZ*3600 , 60000);
  WiFi_begin();
  mqttClient.setServer(EC_Config.MQTT_SERVER, 1883);
  mqttClient.setCallback(callback);
  HTTP_begin();
  char str[20];


  Wire.begin(5,4);
  if(!bmp.begin())
{
  /* There was a problem detecting the BMP085 ... check your connections */
  Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
  while(1);
}

/* Display some basic information on this sensor */
displaySensorDetails();

ui.setTargetFPS(30);

ui.setActiveSymbole(activeSymbole);
ui.setInactiveSymbole(inactiveSymbole);

// You can change this to
// TOP, LEFT, BOTTOM, RIGHT
ui.setIndicatorPosition(BOTTOM);

// Defines where the first frame is located in the bar.
ui.setIndicatorDirection(LEFT_RIGHT);

// You can change the transition that is used
// SLIDE_LEFT, SLIDE_RIGHT, SLIDE_TOP, SLIDE_DOWN
ui.setFrameAnimation(SLIDE_LEFT);

// Add frames
ui.setFrames(frames, frameCount);

// Add overlays
ui.setOverlays(overlays, overlaysCount);

// Inital UI takes care of initalising the display too.
ui.init();

display.flipScreenVertically();

}




void reconnect() {
  // Loop until we're reconnected
if (isAP) return;
  while (!mqttClient.connected()) {
    Serial.printf("Attempting MQTT connection to %s ...",EC_Config.MQTT_SERVER);
    // Attempt to connect
    if (mqttClient.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqttClient.publish("/myhome/in/1/", "hello world");
      // ... and resubscribe
      mqttClient.subscribe("/myhome/in/1");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void loop() {

if (!isAP) {
NTPClient timeClient(EC_Config.TZ*3600);
  if (!mqttClient.connected()) {
    reconnect();
    }
  mqttClient.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
  lastMsg = now;
   ++value;
     float h = dht.readHumidity();
     float t = dht.readTemperature();
     float p;
     bmp.getPressure(&p);
     timeClient.update();


     // Check if any reads failed and exit early (to try again).
        if (isnan(h) || isnan(t)) {
          Serial.println("Failed to read from DHT sensor!");
          return;
     }

    dtostrf(t, 3, 2, Tmsg);
    Serial.print("Publish message T=");
    Serial.println(Tmsg);
    mqttClient.publish("/myhome/out/temperature001/state",Tmsg);
    dtostrf(h, 3, 2, Hmsg);
    Serial.print("Publish message H=");
    Serial.println(Hmsg);
    mqttClient.publish("/myhome/out/hum001/state",Hmsg);
    dtostrf(p/100, 3, 2, Pmsg);
    Serial.print("Publish message P=");
    Serial.println(Pmsg);
    mqttClient.publish("/myhome/out/press001/state",Pmsg);
    strcpy( CurentTime, timeClient.getFormattedTime().c_str() );
    Serial.println(CurentTime);
 }
}



 httpServer.handleClient();
//i2c_scan();
int remainingTimeBudget = ui.update();

if (remainingTimeBudget > 0) {
  // You can do some work here
  // Don't do stuff if you are below your
  // time budget.
  delay(remainingTimeBudget);
}


}
