#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "arduino_secrets.h"
#include "thingProperties.h"

// CloudLight luz_quarto_direita;
// CloudLight luz_quarto_esquerda;
// CloudLight luz_quarto_tras;
// CloudLight luz_sala;
// CloudLight luz_garagem;

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
// WiFi password
#define WIFI_PASSWORD "9292929292"

#define INFLUXDB_URL "http://192.168.249.161:8086"
#define INFLUXDB_TOKEN "V-rxlZM710rNB7ywP-P-zzY-4HHrut8SczuRWjFAZKDfHhgk_izXZpZLmiaPFwHEfB4C99_OiB-fT5TJLNDqOg=="
#define INFLUXDB_ORG "hawktuah"
#define INFLUXDB_BUCKET "tralalerotralala"

// Time zone info
#define TZ_INFO "UTC0"

// Declare InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);

// Declare Data point
Point sensor("wifi_status");
Point distancia("distancia");
Point luz("luz");
Point temperatura("temperatura");
Point humidade("humidade");
Point lux("lux");

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

// Pinos dos leds a interagir com o IoT Cloud
#define LED_SALA 25
#define LED_QUARTO_ESQUERDA 32
#define LED_QUARTO_DIREITA 33
#define LED_QUARTO_TRAS 26
#define LED_GARAGEM 27

#define LDR_PIN 4
#define DHTPIN 16           // GPIO pin where the DHT22 is connected
#define DHTTYPE DHT22      // Define sensor type as DHT22
#define TRIG_PIN 17
#define ECHO_PIN 18
#define POT_PIN 19
#define BUZZ_PIN 14

float duration, distance, lux, temperature, humidity;
int potValue, tempValue, ldrValue;

DHT dht(DHTPIN, DHTTYPE);  // Create DHT sensor object

void setup() {
  Serial.begin(115200);
  delay(500);                 3
  // wait for the OLED to power up
  display.begin(0x3C, true);  // Address 0x3C default

  display.display();
  delay(1000);

  // Iniciar o Arduino IoT
  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  // Inicializar os pinos dos LEDs
  pinMode(LED_SALA, OUTPUT);
  pinMode(LED_QUARTO_ESQUERDA, OUTPUT);
  pinMode(LED_QUARTO_DIREITA, OUTPUT);
  pinMode(LED_QUARTO_TRAS, OUTPUT);
  pinMode(LED_GARAGEM, OUTPUT);

  pinMode(POT_PIN, INPUT);  // Potentiometer PIN
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

  // Conectar ao InfluxDB
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

  potValue = analogRead(POT_PIN);
  //LDR
  ldrValue = analogRead(LDR_PIN);
  lux = resistanceToLux(calculateLDRResistance(ldrValue));//CUIDADO COM RESISTANCETOLUX pode ter os parametros errados retirados do datasheet
  //DHT sensor
  temperature = getTemperatureCelsius();
  humidity = getHumidityPercentage();
  //medicao está sequencial, pode vir a ser necessario interrupt ou threads
  distance = getDistanceCM();

   
  // Send values to InfluxDB
  humidade.addField("humidade", humidity);
  temperatura.addField("temperatura", temperature);
  distancia.addField("distancia", distance);
  lux.addField("Lux", lux);//em lux


  pwmValue = map(potValue, 0, 4095, 0, 255);  // Scale to 8-bit PWM (0-255)

  display.clearDisplay();
  display.setCursor(0, 0);
  delay(10);  // Small delay to smooth readings

  yield();
  display.display();
  
  // Clear fields for reusing the point. Tags will remain the same as set above.
  sensor.clearFields();
  luz.clearFields();

  // Atualizar o Arduino IoT para possíveis mudanças no Led
  ArduinoCloud.update();

  // Store measured value into point
  // Report RSSI of currently connected network
  sensor.addField("rssi", WiFi.RSSI());

  // Print what are we exactly writing
  Serial.print("Writing: ");
  Serial.println(sensor.toLineProtocol());
  Serial.println(luz.toLineProtocol());

  // Check WiFi connection and reconnect if needed
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("Wifi connection lost");
  }

  // Enviar dados para o InfluxDB
  if (!client.writePoint(sensor) || !client.writePoint(luz)|| !client.writePoint(temperatura) || !client.writePoint(humidade) || !client.writePoint(lux)) || !client.writePoint(distancia)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  Serial.println("Waiting 1 second");
  delay(1000);
}

// Function to measure distance using HC-SR04
float getDistanceCM() {
  // Send a 10us HIGH pulse on the trigger pin
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure the length of the echo pulse
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // Timeout at 30ms (~5 meters)
  
  if (duration == 0) {
    Serial.println("No pulse received (timeout).");
    return -1.0;
  }

  // Convert time into distance (speed of sound = 343 m/s)
  float distance = (duration * 0.0343) / 2;

  if(disance < 2) //em cm
  {
    println("Object detected\n");
  }

  return distance;
}

// Function to read temperature in Celsius
float getTemperatureCelsius() {
  float temp = dht.readTemperature();  // Reads in Celsius by default

  if (isnan(temp)) {
    Serial.println("Failed to read temperature!");
    return -999.0; // Error code
  }
  if(temp < 20){//celcius
    //ligar o aquecedor (LED vermelho)
  }
  return temp;
}

// Function to read humidity in percentage
float getHumidityPercentage() {
  float hum = dht.readHumidity();
  if (isnan(hum)) {
    Serial.println("Failed to read humidity!");
    return -999.0; // Error code
  }
  if(hum > 60){ //humidade relativa 0-100%
    //turn buzzer ON
  }
  return hum;
}
/*
função para calcular a resistencia equivalente do LDR
*/
float calculateLDRResistance(int adcValue) {
    const float Vcc = 3.3;
    const float R_fixed = 15000.0; // 15kΩ

    float voltage = (adcValue / 4095.0) * Vcc;
    float resistance = (voltage * R_fixed) / (Vcc - voltage);
    return resistance;
}
/*
função para calcular a o valor em lux do LDR*/
#include <math.h>

float resistanceToLux(float resistance_ohms) {
    const float A = 100430.0;  // retirado da curva caracteristica do datasheet do LDR
    const float B = 0.952;
    float lux = pow((resistance_ohms / A), -1.0 / B);
    if(lux < 100){ //em lux obviamente, duhh, seu alienigena
      //ligar o led branco
    }
    return lux;
}


/*
  Since LuzSala1 is READ_WRITE variable, onLuzSala1Change() is
  executed every time a new value is received from IoT Cloud.
*/
void onLuzSalaChange()  {
  // Add your code here to act upon LuzSala1 change
  digitalWrite(LED_SALA, luz_sala);
  luz.addField("Luz da Sala", luz_sala);
}


/*
  Since LuzQuartoEsquerda is READ_WRITE variable, onLuzQuartoEsquerdaChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onLuzQuartoEsquerdaChange()  {
  // Add your code here to act upon LuzQuartoEsquerda change
  digitalWrite(LED_QUARTO_ESQUERDA, luz_quarto_esquerda);
  luz.addField("Luz do Quarto Esquerda", luz_quarto_esquerda);
}

/*
  Since LuzQuartoDireita is READ_WRITE variable, onLuzQuartoDireitaChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onLuzQuartoDireitaChange()  {
  // Add your code here to act upon LuzQuartoDireita change
  digitalWrite(LED_QUARTO_DIREITA, luz_quarto_direita);
  luz.addField("Luz do Quarto Direita", luz_quarto_direita);
}


/*
  Since LuzQuartoTras is READ_WRITE variable, onLuzQuartoTrasChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onLuzQuartoTrasChange()  {
  // Add your code here to act upon LuzQuartoTras change
  digitalWrite(LED_QUARTO_TRAS, luz_quarto_tras);
  luz.addField("Luz do Quarto Tras", luz_quarto_tras);
}

/*
  Since LuzGaragem is READ_WRITE variable, onLuzGaragemChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onLuzGaragemChange()  {
  // Add your code here to act upon LuzGaragem change
  digitalWrite(LED_GARAGEM, luz_garagem);
  luz.addField("Luz da Garagem", luz_garagem);
}
