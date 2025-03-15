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
  //  #define WIFI_SSID "OPPO Find X5 Pro"
  //  #define WIFI_SSID "DIGIFIBRA-t6X9"
  #define WIFI_SSID "Sala-Adultos_WiFi"
  // WiFi password
  //  #define WIFI_PASSWORD "9292929292"
  //  #define WIFI_PASSWORD "SQGfKXRz5X4u"
  #define WIFI_PASSWORD "@sl_adlt_da%gUesT"
  
  #define INFLUXDB_URL "http://192.168.3.104:8086"
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
  
const int potPin = 33;  // Pin connected to the potentiometer
const int ledPin = 25;  // Pin connected to the LED
const int buttonC = 17;

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

// OLED FeatherWing buttons map to different pins depending on board:
#if defined(ESP8266)
  #define BUTTON_A  0
  #define BUTTON_B 16
  #define BUTTON_C  2
#elif defined(ESP32) && \
    !defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2) && \
    !defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3) && \
    !defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_NOPSRAM)
  #define BUTTON_A 15
  #define BUTTON_B 32
  #define BUTTON_C 14
#elif defined(ARDUINO_STM32_FEATHER)
  #define BUTTON_A PA15
  #define BUTTON_B PC7
  #define BUTTON_C PC5
#elif defined(TEENSYDUINO)
  #define BUTTON_A  4
  #define BUTTON_B  3
  #define BUTTON_C  8
#elif defined(ARDUINO_NRF52832_FEATHER)
  #define BUTTON_A 31
  #define BUTTON_B 30
  #define BUTTON_C 27
#else // 32u4, M0, M4, nrf52840, esp32-s2, esp32-s3 and 328p
  #define BUTTON_A  5
  #define BUTTON_B  6
  #define BUTTON_C  9
#endif

 #define BUTTON_A  18
void setup() {
  Serial.begin(115200);
  Serial.println("128x64 OLED FeatherWing test");
  delay(500); // wait for the OLED to power up
  display.begin(0x3C, true); // Address 0x3C default

  Serial.println("OLED begun");

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  display.setRotation(1);
  Serial.println("Button test");

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

  // text display tests
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0,0);
  display.print("gemidao do esp32\nOOOOOOOOOOUUUUUUUAAAAHHHHHHHHHH\n       c: ");
  //display.print("Connecting to SSID\n'DIGIFIBRA-tX09':");
  //display.print("connected!");
  //display.println("IP: 10.0.1.23");
  //display.println("Sending val #0");
  display.display(); // actually display all of the above
  pinMode(potPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonC, INPUT_PULLUP);

  display.setRotation(2);
  display.setCursor(0,0);
  display.clearDisplay();
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
  if(!digitalRead(BUTTON_A)) {
  //  Serial.println("A read!");
  //  analogWrite(ledPin, HIGH);
  //  display.print("A");
  };
  if(!digitalRead(BUTTON_B)) display.print("B");
  if(!digitalRead(BUTTON_C)) display.print("C");
  delay(10);
  
  int potValue = analogRead(potPin);
  int pwmValue = map(potValue, 0, 4095, 0, 255);  // Scale to 8-bit PWM (0-255)
  //Serial.println(pwmValue);  // Print potentiometer value to Serial Monitor  // Read the potentiometer value (0-4095)
  display.clearDisplay();
  display.setCursor(0,0);
  //analogWrite(ledPin, pwmValue);  // Set the LED brightness
  //display.println("Value read: ");
  //display.print(pwmValue);
  delay(10);  // Small delay to smooth readings
 
  if(digitalRead(buttonC)==LOW){
    display.println("Button state: 1");
    Serial.println("Button state: 1");
    digitalWrite(ledPin, HIGH); // Turn LED ON
    } else {
    display.println("Button state: 0");
    Serial.println("Button state: 0");
    digitalWrite(ledPin, LOW);  // Turn LED OFF
  }
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
