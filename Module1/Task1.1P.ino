/*
  Task 1.1P Sense-Think-Act Board as Ardunio File
*/

// Set variable &constants
int sensor_data = 0;		// Initialise Sensor_data variable to zero
int const motion = 2;		// Assign motion sensor to Pin# 2
int const LED = 13;			// Assign motion sensor to Pin# 13


// Program setup code
void setup()
{
  pinMode(motion,INPUT); 	// initialise sensor as input
  pinMode(LED, OUTPUT);		// initialise LED as output
  Serial.begin(9600); 		// initialise serial at 9600bps
}

// Program Main 
void loop()
{
  sensor_data = digitalRead(motion); // Read Sensor
  
  if(sensor_data == HIGH)
  {
    Serial.println("Motion Detected");
    digitalWrite(LED, HIGH); // Turn LED on
    Serial.println("LED Activated");
  }
  else
  {
    Serial.println("No Motion Detected");
    digitalWrite(LED, LOW); // turn LED off
    Serial.println("LED NOT Activated");
  }
  delay(500); // Delay 500ms
}
