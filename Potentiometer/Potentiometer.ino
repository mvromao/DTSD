const int potPin = 33;  // Pin connected to the potentiometer
const int ledPin = 25;  // Pin connected to the LED

void setup() {
  Serial.begin(115200);  // Initialize serial communication
  pinMode(potPin, INPUT);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  int potValue = analogRead(potPin);
  int pwmValue = map(potValue, 0, 4095, 0, 255);  // Scale to 8-bit PWM (0-255)
  //Serial.println(pwmValue);  // Print potentiometer value to Serial Monitor  // Read the potentiometer value (0-4095)
  
  analogWrite(ledPin, pwmValue);  // Set the LED brightness
  delay(10);  // Small delay to smooth readings
}
