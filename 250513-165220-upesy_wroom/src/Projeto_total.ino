// Variávels de DEBUG ====================
#define WIFI_ENABLED 1
#define INFLUXDB_ENABLED 1
#define OLED_ENABLED 1
// =======================================

#include <SPI.h>
#include <Wire.h>
#include <math.h>

#include "arduino_secrets.h"
#include <ESP32Servo.h>
#include <DHTesp.h>

// Initialize Wifi if enabled
#if WIFI_ENABLED
	#include <WiFi.h>
	void startWiFi() {
		WiFi.mode(WIFI_STA);
		WiFi.begin(SECRET_SSID, SECRET_OPTIONAL_PASS);
		while (WiFi.status() != WL_CONNECTED) {
			delay(1000);
			Serial.print(".");
		}
		Serial.println("Connected to WiFi");
		setenv("TZ", TZ_INFO, 1);
		tzset();
		configTime(0, 0, "pool.ntp.org", "time.nis.gov");
	}
#endif

#define DEVICE "ESP32"

#if OLED_ENABLED
	#include <Adafruit_GFX.h>
	#include <Adafruit_SH110X.h>
	Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);
	void startOLED() {
	// Initialize the OLED display
	if(OLED_ENABLED == 1) {	
		delay(300);
		display.begin(0x3C, true); // Address 0x3C default
		display.setRotation(1);
		display.setTextSize(1);
		display.setTextColor(SH110X_WHITE);
		display.setCursor(0, 0);
		display.println("Starting...");
		display.display();
	}
	}
	void writeToOLED(String text)
	{
		display.clearDisplay();
		display.setCursor(0, 0);
		display.println(text);
		display.display();
	}
#endif

// Declare Data points if InfluxDB is enabled
#if INFLUXDB_ENABLED 
	#include <InfluxDbClient.h> // Influx DB Libraries
	#include <InfluxDbCloud.h>	// Influx DB Libraries
	
	Point sensors("Sensores");
	Point luzes("Luzes");
	Point garagem("Garagem");	

	InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);

	void startInfluxDB()
	{
		if (client.validateConnection())
		{
			Serial.print("Connected to InfluxDB: ");
			Serial.println(client.getServerUrl());
		}
		else
		{
			Serial.print("InfluxDB connection failed: ");
			Serial.println(client.getLastErrorMessage());
		}
	}
#endif


// Pinos dos leds a interagir com o IoT Cloud
#define LED_SALA 25
#define LED_QUARTO_ESQUERDA 32
#define LED_QUARTO_DIREITA 33
#define LED_QUARTO_TRAS 26
#define LED_GARAGEM 27

#define LDR_PIN 36
#define DHT_PIN 16
#define TRIG_PIN 17
#define ECHO_PIN 18
#define POT_PIN 19
#define BUZZ_PIN 14
#define SERVO_PIN 5

DHTesp dht;  // Create DHT sensor object
Servo servoMotor; // Create a Servo object

float durationValue, distanceValue, luxValue, temperatureValue, humidityValue;
int potValue, tempValue, ldrValue, pwmValue, portaState, dhtIndex = 0;

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
	
	pinMode(LDR_PIN, INPUT);
}


void startSerial()
{
	Serial.begin(9600);
	delay(500);
}

void setup(){
	dht.setup(DHT_PIN, DHTesp::DHT22); // Connect DHT sensor to GPIO 17
	startSerial();
	startLEDs();
	if(OLED_ENABLED == 1)
		startOLED();
	Serial.println("Starting...");

	servoMotor.attach(SERVO_PIN); // Attach the servo to the defined pin
    servoMotor.write(0);          // Set the initial position of the servo to 0 degrees

	delay(1000);
	if(WIFI_ENABLED == 1)
		startWiFi();

	if(INFLUXDB_ENABLED == 1) {
		startInfluxDB();
		sensors.addTag("device", DEVICE);
		
	}

	Serial.println("Finished setup");
}

void loop()
{
	dhtIndex++;
	// Read potentiometer value
	//potValue = analogRead(POT_PIN);
	//pwmValue = map(potValue, 0, 4095, 0, 255); // Scale to 8-bit PWM (0-255)

	// Read LDR value
	ldrValue = analogRead(LDR_PIN);

	// Read LDR resistance and lux value
	luxValue = LDRToLux(ldrValue);
	Serial.print("LDR Value: ");
	Serial.print(ldrValue);
	// Serial.print("Lux Value: ");
	// Serial.println(luxValue);

	// DHT sensor
	temperatureValue = getTemperatureCelsius();
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
	checkDistance(distanceValue);
	checkLight(ldrValue);
	Serial.print("Porta: ");
	Serial.println(portaState? "Aberta" : "Fechada");
	
	if (dhtIndex == 4) {
		checkTemperature(temperatureValue);
		checkHumidity(humidityValue);
		dhtIndex = 0;
	}
	// Send values to InfluxDB
	if(INFLUXDB_ENABLED == 1)
	{
		//sensors.addField("pot", potValue);
		sensors.addField("LDR", ldrValue);
		sensors.addField("lux", luxValue);
		sensors.addField("humidade", humidityValue);
		sensors.addField("temperatura", temperatureValue);
		sensors.addField("distancia", distanceValue);
		
		sensors.addField("rssi", WiFi.RSSI());

		luzes.addField("Sala", digitalRead(LED_SALA));
		luzes.addField("Quarto_Esquerda", digitalRead(LED_QUARTO_ESQUERDA));
		luzes.addField("Quarto_Direita", digitalRead(LED_QUARTO_DIREITA));
		luzes.addField("Quarto_Tras", digitalRead(LED_QUARTO_TRAS));
		luzes.addField("Garagem", digitalRead(LED_GARAGEM));

		garagem.addField("Estado da Porta", portaState);
		
		if(WiFi.status() != WL_CONNECTED)
		{
			Serial.println("WiFi connection lost");
			startWiFi();
		}

		// Send data to InfluxDB
		if (!client.writePoint(sensors) || !client.writePoint(luzes) || !client.writePoint(garagem))
		 {
		 Serial.print("InfluxDB write failed: ");
		 Serial.println(client.getLastErrorMessage());
		 }
	}

	delay(500);
	Serial.println("Waiting 500 milli");
}


//	===========================================
//			HOUSE UPDATING FUNCTIONS
//	===========================================

void checkTemperature(float temperature)
{
	if (temperature >= 20)
	{ // Threshold for temperature detection
		digitalWrite(LED_GARAGEM, HIGH);
	}
	if(temperature < 20 && temperature > 0)
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
	if(!(distance == -1))
		if (distance < 4 && distance > 0)
		{ 
			Serial.println("Object detected");
			servoMotor.write(0); 
			portaState = 1; // Open the door
		}
		else
		{
			Serial.println("No object detected");
			servoMotor.write(130);  // Turn the servo to 0 degrees
			portaState = 0; // Close the door
		}
}
void checkLight(float lux)
{
	if (lux > 1500)
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

//	===========================================
//			DATA ACQUIRING FUNCTIONS
//	===========================================

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
float getTemperatureCelsius()
{
    float temp;
    for (int i = 0; i < 3; i++) // Retry up to 3 times
    {
        temp = dht.getTemperature(); // Reads in Celsius by default
        if (!isnan(temp))
        {
            return temp; // Return valid temperature
        }
        delay(1000); // Small delay before retrying
    }
    Serial.println("Failed to read temperature after retries!");
	Serial.println(temp);
    return -999.0; // Error code
}
float getHumidityPercentage()
{
    float hum;
    for (int i = 0; i < 3; i++) // Retry up to 3 times
    {
        hum = dht.getHumidity();
        if (!isnan(hum))
        {
			
            return hum; // Return valid humidity as a percentage
        }
        delay(1000); // Small delay before retrying
    }
    Serial.println("Failed to read humidity after retries!");
	Serial.println(hum);
    return -999.0; // Error code
}
float LDRToLux(float LDR)
{
    return 10837*exp(-1.07*map(LDR,0,4095,0,3.3)); // Convert LDR voltage to lux
}