#include <SPI.h>
#include <Wire.h>
#include <math.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SH110X.h>
#include "arduino_secrets.h"
//#include <InfluxDbClient.h> // Influx DB Libraries
//#include <InfluxDbCloud.h>	// Influx DB Libraries
#include <ESP32Servo.h>
#include <DHT.h>
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#define DEVICE "ESP32"

// Declare InfluxDB client instance with preconfigured InfluxCloud certificate
//InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);

// Declare Data point
// Point sensor("wifi_status");
// Point distancia("distancia");
// Point luz("luz");
// Point temperatura("temperatura");
// Point humidade("humidade");
// Point lux("lux");

//Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);
/*
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
#define SERVO_PIN 5
*/
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
#define SERVO_PIN 5

DHT dht(DHTPIN, DHTTYPE);  // Create DHT sensor object
Servo servoMotor; // Create a Servo object

float durationValue, distanceValue, luxValue, temperatureValue, humidityValue;
int potValue, tempValue, ldrValue, pwmValue;

void startLEDs()
{
	// Inicializar os pinos dos LEDs
	pinMode(LED_SALA, OUTPUT);
	pinMode(LED_QUARTO_ESQUERDA, OUTPUT);
	pinMode(LED_QUARTO_DIREITA, OUTPUT);
	pinMode(LED_QUARTO_TRAS, OUTPUT);
	pinMode(LED_GARAGEM, OUTPUT);

	//pinMode(POT_PIN, INPUT);  // Potentiometer PIN

	pinMode(TRIG_PIN, OUTPUT);
	pinMode(ECHO_PIN, INPUT);

	//VAI TE FODER MARCO0S
	
	pinMode(LDR_PIN, INPUT);
}

void startSerial()
{
	Serial.begin(9600);
	delay(500);
}

void setup(){

	startSerial();
	startLEDs();
	Serial.println("Starting...");
	// wait for the OLED to power up
	//display.begin(0x3C, true); // Address 0x3C default

	servoMotor.attach(SERVO_PIN); // Attach the servo to the defined pin
    servoMotor.write(0);          // Set the initial position of the servo to 0 degrees

	//display.display();
	delay(1000);

	// Clear the buffer.
	// display.clearDisplay();
	// display.display();
	// display.setRotation(1);
	// display.setTextSize(1);
	// display.setTextColor(SH110X_WHITE);
	// display.setCursor(0, 0);
	// display.println("Attemptingto connectto network");
	// display.println(SECRET_SSID);
	// display.display();
	// WiFi.mode(WIFI_STA);
	// wifiMulti.addAP(SECRET_SSID, SECRET_OPTIONAL_PASS);

	// while (wifiMulti.run() != WL_CONNECTED)
	// {
	// 	// display.print(".");
	// 	// display.display();
	// 	Serial.print(".");
	// 	delay(100);
	// }
	// Serial.println();

	// display.println("\nConnection Success!");
	// display.display();

	// timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

	// Conectar ao InfluxDB
	// if (client.validateConnection())
	// {
	// 	Serial.print("Connected to InfluxDB: ");
	// 	Serial.println(client.getServerUrl());
	// }
	// else
	// {
	// 	Serial.print("InfluxDB connection failed: ");
	// 	Serial.println(client.getLastErrorMessage());
	// }

	// sensor.addTag("device", DEVICE);
	// sensor.addTag("SSID", WiFi.SSID());
	Serial.println("Finished setup");
}

void loop()
{
	// Read potentiometer value
	//potValue = analogRead(POT_PIN);
	//pwmValue = map(potValue, 0, 4095, 0, 255); // Scale to 8-bit PWM (0-255)
	Serial.print("Potentiometer Value: ");
	Serial.println(potValue);

	// Read LDR value
	ldrValue = analogRead(LDR_PIN);

	// Read LDR resistance and lux value
	//luxValue = resistanceToLux(calculateLDRResistance(ldrValue));
	luxValue = LDRToLux(ldrValue); // Scale to 0-1000 lux
	Serial.print("LDR Value: ");
	Serial.println(ldrValue);
	Serial.print("Lux Value: ");
	Serial.println(luxValue);

	// DHT sensor
	temperatureValue = getTemperatureCelsius()*2;
	humidityValue = getHumidityPercentage();
	Serial.print("Temperature: ");
	Serial.print(temperatureValue);
	Serial.print(" °C, Humidity: ");
	Serial.print(humidityValue);
	Serial.println(" %");

	// Medicao está sequencial, pode vir a ser necessario interrupt ou threads
	distanceValue = getDistanceCM();
	Serial.print("Distance: ");
	Serial.print(distanceValue);
	Serial.println(" cm");

	// Change house parameters
	checkTemperature(temperatureValue);
	checkDistance(distanceValue);
	checkLight(luxValue);
  	checkHumidity(humidityValue);

	// Send values to InfluxDB
	// humidade.addField("humidade", humidityValue);
	// temperatura.addField("temperatura", temperatureValue);
	// distancia.addField("distancia", distanceValue);
	// lux.addField("Lux", luxValue); // em lux

	// display.clearDisplay();
	// display.setCursor(0, 0);
	delay(10); // Small delay to smooth readings

	//yield();
	// display.display();

	// Clear fields for reusing the point. Tags will remain the same as set above.
	// sensor.clearFields();
	// luz.clearFields();

	// Store measured value into point
	// Report RSSI of currently connected network
	// sensor.addField("rssi", WiFi.RSSI());

	// Print what are we exactly writing
	// Serial.print("Writing: ");
	// Serial.println(sensor.toLineProtocol());
	// Serial.println(luz.toLineProtocol());

	// Check WiFi connection and reconnect if needed
	// if (wifiMulti.run() != WL_CONNECTED)
	// {
	// 	Serial.println("Wifi connection lost");
	// }

	// Enviar dados para o InfluxDB
	//if (!client.writePoint(sensor) || !client.writePoint(luz) || !client.writePoint(temperatura) || !client.writePoint(humidade) || !client.writePoint(lux) || !client.writePoint(distancia))
	//	{
	//		Serial.print("InfluxDB write failed: ");
	//		Serial.println(client.getLastErrorMessage());
	//	}

	Serial.println("Waiting 1 second");
}
void checkTemperature(float temperature)
{
	if (temperature > 20) //Ta idiota de proposito para testar
	{ // Threshold for temperature detection
		digitalWrite(LED_GARAGEM, HIGH);
	}
	else
	{
		digitalWrite(LED_GARAGEM, LOW);
	}
}

void checkHumidity(float humidity)
{
  if (humidity > 50) // Threshold for humidity detection
  { // em %
    Serial.println("humidade treshhold atingido");
    //ligar buzzer
  }
  else
  {

  }
}

void checkDistance(float distance)
{
	if (distance < 2)
	{ // em cm
		Serial.println("Object detected");
		servoMotor.write(30); // Turn the servo to 30 degrees
		delay(1000);		  // Wait for the servo to reach the position
		servoMotor.detach();  // Detach the servo to save power
	}
	else
	{
		Serial.println("No object detected");
		servoMotor.write(0);  // Turn the servo to 0 degrees
		delay(1000);		  // Wait for the servo to reach the position
		servoMotor.detach();  // Detach the servo to save power
	}
}

void checkLight(float lux)
{
	if (lux < 1500)
	{ // Threshold for light detection
		digitalWrite(LED_SALA, HIGH);
		digitalWrite(LED_QUARTO_ESQUERDA, HIGH);
		digitalWrite(LED_QUARTO_DIREITA, HIGH);
		digitalWrite(LED_QUARTO_TRAS, HIGH);
	}
	else
	{
		digitalWrite(LED_SALA, LOW);
		digitalWrite(LED_QUARTO_ESQUERDA, LOW);
		digitalWrite(LED_QUARTO_DIREITA, LOW);
		digitalWrite(LED_QUARTO_TRAS, LOW);
	}
}

// Function to measure distance using HC-SR04
float getDistanceCM()
{
	// Send a 10us HIGH pulse on the trigger pin
	digitalWrite(TRIG_PIN, LOW);
	delayMicroseconds(2);
	digitalWrite(TRIG_PIN, HIGH);
	delayMicroseconds(10);
	digitalWrite(TRIG_PIN, LOW);

	// Measure the length of the echo pulse
	long duration = pulseIn(ECHO_PIN, HIGH, 30000); // Timeout at 30ms (~5 meters)

	if (duration == 0)
	{
		Serial.println("No pulse received (timeout).");
		return -1.0;
	}

	// Convert time into distance (speed of sound = 343 m/s)
	float distance = (duration * 0.0343) / 2;

	return distance;
}

// Updated DHT reading functions with retries and delays
float getTemperatureCelsius()
{
    float temp;
    for (int i = 0; i < 3; i++) // Retry up to 3 times
    {
        temp = dht.readTemperature(); // Reads in Celsius by default
        if (!isnan(temp))
        {
            return temp; // Return valid temperature
        }
        delay(1000); // Small delay before retrying
    }
    Serial.println("Failed to read temperature after retries!");
    return -999.0; // Error code
}

float getHumidityPercentage()
{
    float hum;
    for (int i = 0; i < 3; i++) // Retry up to 3 times
    {
        hum = dht.readHumidity();
        if (!isnan(hum))
        {
            return hum / 100; // Return valid humidity as a percentage
        }
        delay(1000); // Small delay before retrying
    }
    Serial.println("Failed to read humidity after retries!");
    return -999.0; // Error code
}

float LDRToLux(float LDR)
{
    return 10837*exp(-1.07*map(LDR,0,4095,0,3.3)); // Convert LDR voltage to lux
}