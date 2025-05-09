#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

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
  delay(1000); // wait for the OLED to power up
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

  display.setRotation(2);
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
  int pwmValue = map(potValue, 0, 4095, 0, 255);  
  //Serial.println(pwmValue);  // Print potentiometer value to Serial Monitor  // Read the potentiometer value (0-4095)
  display.clearDisplay();
  display.setCursor(0,0);
  //analogWrite(ledPin, pwmValue);  // Set the LED brightness
  //display.println("Value read: ");
  //display.print(pwmValue);
  delay(10);  // Small delay to smooth readings
 
  if(digitalRead(buttonC)==LOW){
    display.println("Button value: PRESSED");
    Serial.println("Button read");
    digitalWrite(ledPin, HIGH); // Turn LED ON
    } else {
    display.println("Button value: NOT PRESSED");  
    Serial.println("Button not read");
    
    digitalWrite(ledPin, LOW);  // Turn LED OFF
  }
  yield();
  display.display();
}
