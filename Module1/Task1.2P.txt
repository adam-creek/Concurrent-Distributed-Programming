/*
  Task 1.2P Sense-Think-Act Board using Interrupts
*/

// Set variable &constants
int const motion = 2;		  // Assign motion sensor to Pin# 2
int const LED = 13;			  // Assign motion sensor to Pin# 13
volatile int motion_flag;	// set LED flag varaible as volatile. i.e. enables state change

// Program setup code
void setup()
{
  pinMode(motion,INPUT); 	// initialise sensor as input
  pinMode(LED, OUTPUT);		// initialise Green LED as output
  Serial.begin(9600); 		// initialise serial at 9600bps
  attachInterrupt(digitalPinToInterrupt(motion),ISR_LED,CHANGE);
}

// Program Main 
void loop()
{
}

//Interrupt function
void ISR_LED()
{
  motion_flag = digitalRead(motion); 	// Read input PIN #2
  digitalWrite(LED,motion_flag);		  
  if (motion_flag==1)                 // Serial Print motion flag and associated description based on motion flag status
  {
    Serial.println();
    Serial.print(motion_flag);
    Serial.print("\t");
    Serial.print("Motion Detected");
  }
   else
   {
     Serial.println();
     Serial.print(motion_flag);
     Serial.print("\t");
     Serial.print("NO Motion Detected");
   }
}
