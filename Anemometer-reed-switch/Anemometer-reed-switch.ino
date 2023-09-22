const int reedSwitchPin = 2;
volatile unsigned long lastTime = 0;
volatile unsigned long elapsedTime = 0;
volatile unsigned long rotationCount = 0;

void setup() {
  pinMode(reedSwitchPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(reedSwitchPin), countRotation, FALLING);
  Serial.begin(9600);
}

void loop() {
  // Controleer of er minstens één rotatie heeft plaatsgevonden voordat de windsnelheid wordt berekend
  if (rotationCount > 0) {
    // Bereken de windsnelheid in m/s op basis van het aantal rotaties en de tijd
    float windSpeed = (rotationCount / 2.0) / (elapsedTime / 1000.0); // in meter per seconde

    // Toon de windsnelheid op de seriële monitor
    Serial.print("Windsnelheid: ");
    Serial.print(windSpeed);
    Serial.println(" m/s");
  } 

  // Reset de rotatieteller en tijd
  rotationCount = 0;
  elapsedTime = 0;

  delay(1000); // Wacht 1 seconde voordat je opnieuw meet
}

void countRotation() {
  unsigned long currentTime = millis();
  elapsedTime = currentTime - lastTime;
  lastTime = currentTime;
  rotationCount++;
}
