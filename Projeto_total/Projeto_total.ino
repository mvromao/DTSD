#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>


#if defined(ESP32)
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#define DEVICE "ESP32"
#elif defined(ESP8266)
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#define DEVICE "ESP8266"
#endif

#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

// WiFi AP SSID
#define WIFI_SSID "OPPO Find X5 Pro"
//  #define WIFI_SSID "DIGIFIBRA-t6X9"
//  #define WIFI_SSID "Sala-Adultos_WiFi"
// WiFi password
#define WIFI_PASSWORD "9292929292"
//  #define WIFI_PASSWORD "SQGfKXRz5X4u"
//  #define WIFI_PASSWORD "@sl_adlt_da%gUesT"

#define INFLUXDB_URL "http://192.168.249.161:8086"
//#define INFLUXDB_URL "http://localhost:8086"
#define INFLUXDB_TOKEN "V-rxlZM710rNB7ywP-P-zzY-4HHrut8SczuRWjFAZKDfHhgk_izXZpZLmiaPFwHEfB4C99_OiB-fT5TJLNDqOg=="
#define INFLUXDB_ORG "hawktuah"
#define INFLUXDB_BUCKET "tralalerotralala"

// Time zone info
#define TZ_INFO "UTC0"

// Declare InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);

// Declare Data point
Point sensor("wifi_status");


Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

#define LED_PIN 25
#define TEMP_PIN 16
#define TRIG_PIN 17
#define ECHO_PIN 18
#define POT_PIN 16

float duration, distance;

void setup() {
  Serial.begin(115200);
  delay(500);               // wait for the OLED to power up
  display.begin(0x3C, true);  // Address 0x3C default

  display.display();
  delay(1000);

  pinMode(POT_PIN, INPUT);  // Potentiometer PIN
  pinMode(LED_PIN, OUTPUT); // LED PIN for output
  pinMode(TEMP_PIN, INPUT); // Temperature Pin

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  display.setRotation(1);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);

  display.println("Attemptingto connectto network");
  display.println(WIFI_SSID);
  display.display();
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  while (wifiMulti.run() != WL_CONNECTED) {
    display.print(".");
    display.display();
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  display.println("\nConnection Success!");
  display.display();

  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  sensor.addTag("device", DEVICE);
  sensor.addTag("SSID", WiFi.SSID());
}

void loop() {
  int potValue = analogRead(POT_PIN);
  int tempValue = analogRead(TEMP_PIN);
  
  int pwmValue = map(potValue, 0, 4095, 0, 255);  // Scale to 8-bit PWM (0-255)

  int
    //Serial.println(pwmValue);  // Print potentiometer value to Serial Monitor  // Read the potentiometer value (0-4095)
    display.clearDisplay();
  display.setCursor(0, 0);
  //analogWrite(ledPin, pwmValue);  // Set the LED brightness
  //display.println("Value read: ");
  //display.print(pwmValue);
  delay(10);  // Small delay to smooth readings

  //analogWrite(ledPin, LOW);
  yield();
  display.display();
  // Clear fields for reusing the point. Tags will remain the same as set above.
  sensor.clearFields();

  // Store measured value into point
  // Report RSSI of currently connected network
  sensor.addField("rssi", WiFi.RSSI());

  // Print what are we exactly writing
  Serial.print("Writing: ");
  Serial.println(sensor.toLineProtocol());

  // Check WiFi connection and reconnect if needed
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("Wifi connection lost");
  }

  // Write point
  if (!client.writePoint(sensor)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  Serial.println("Waiting 1 second");
  delay(1000);
}
