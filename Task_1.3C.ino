/*
  Task 1.3C Multiple Interrupts
*/

// Set variable &constants
int const motion = 2;		// Assign motion sensor to Pin# 2
int const button = 3;		// Assign button to Pin# 3
int const LED_green = 13;		// Assign LED Green to Pin# 13
int const LED_red = 10;			// Assign LED Red to Pin# 10
volatile int motion_flag = 0;	// set motion Flag varaible as volatile to 0
volatile int button_state = 0;	// set button State varaible as volatile to 0 

// Program setup code
void setup()
{
  pinMode(motion,INPUT); 		// initialise sensor as input
  pinMode(button,INPUT); 		// initialise button as input
  pinMode(LED_green, OUTPUT);	// initialise Green LED as output
  pinMode(LED_red, OUTPUT);		// initialise Red LED as output
  
  attachInterrupt(digitalPinToInterrupt(motion),ISR_LED_green,CHANGE);
  attachInterrupt(digitalPinToInterrupt(button),ISR_LED_red,CHANGE);
	Serial.begin(9600); 		// initialise serial at 9600bps
}

// Program Main 
void loop()
{
}

//Interrupt function
void ISR_LED_green()
{
  motion_flag = digitalRead(motion); 	// Read input PIN #2=
  digitalWrite(LED_green,motion_flag);
  if (motion_flag==1)                 // If motion flag is High, print output that motion has been detected, otherwise print it has not 
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

void ISR_LED_red()
{
  button_state = digitalRead(button); 	// Read input PIN #3
  digitalWrite(LED_red,button_state);
  if (button_state==1)                  // If button state is High, print output that it has been pressed, otherwise do nothing
  {
    Serial.println();
    Serial.print(button_state);
    Serial.print("\t");
    Serial.print("Button Pressed");
  }
   else
   {
   }
}
