#include <Servo.h>
#include "HX711.h" // Include the HX711 library

Servo servo1;

const int trigPin = 12;
const int echoPin = 11;
const int potPin = A0; // Input pin
const int DOUT = 3; // Pin connected to HX711 DOUT
const int CLK = 2;  // Pin connected to HX711 CLK

long duration;
int distance = 0;
int soil = 0;
int fsoil = 0;
float weight = 0.0;

HX711 scale; // Create an instance of the HX711 class
float calibration_factor = -196404; // Initial calibration factor, adjust as needed

void setup() 
{
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT); 
  servo1.attach(8);

  // Initialize the HX711
  scale.begin(DOUT, CLK);
  scale.set_scale();
  scale.tare(); // Reset the scale to zero

  Serial.println("HX711 calibration sketch");
  Serial.println("Remove all weight from scale");
  Serial.println("After readings begin, place known weight on scale");
  Serial.println("Press + or a to increase calibration factor");
  Serial.println("Press - or z to decrease calibration factor");

  long zero_factor = scale.read_average(); // Get a baseline reading
  Serial.print("Zero factor: "); // This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor);
}

void loop() 
{
  // Calibration routine
  if (Serial.available()) {
    char temp = Serial.read();
    if (temp == '+' || temp == 'a')
      calibration_factor += 10;
    else if (temp == '-' || temp == 'z')
      calibration_factor -= 10;
  }

  scale.set_scale(calibration_factor); // Adjust to this calibration factor

  Serial.print("Reading: ");
  Serial.print(scale.get_units(), 1);
  Serial.print(" lbs"); // Change this to kg and re-adjust the calibration factor if you follow SI units
  Serial.print(" calibration_factor: ");
  Serial.print(calibration_factor);
  Serial.println();

  // Read ultrasonic sensor
  distance = 0;
  for (int i = 0; i < 2; i++) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distance += duration * 0.034 / 2;
    delay(10);
  }
  distance /= 2;
  Serial.print("Distance: ");
  Serial.println(distance);

  if (distance < 15 && distance > 1) {
    delay(1000);

    // Read soil moisture sensor
    fsoil = 0;
    for (int i = 0; i < 3; i++) {
      soil = analogRead(potPin);
      soil = constrain(soil, 485, 1023);
      fsoil += map(soil, 485, 1023, 100, 0);
      delay(75);
    }
    fsoil /= 3;
    Serial.print("Soil Moisture: ");
    Serial.print(fsoil);
    Serial.println("%");

    if (fsoil > 3) {
      Serial.println("WET");
      servo1.write(0);
    } else {
      Serial.println("DRY");
      servo1.write(180);
    }

    delay(3000);
    servo1.write(90);

    // Read weight from the load cell
    weight = scale.get_units(10); // Read the average of 10 readings
    Serial.print("Weight: ");
    Serial.print(weight);
    Serial.println(" kg");
  }

  delay(1000);
}
