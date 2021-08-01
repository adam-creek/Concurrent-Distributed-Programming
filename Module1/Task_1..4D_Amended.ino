/*
  Task 1.4D Multiple Interrupts & Timer
  
  Program has three inputs - motion sensor, button and slide switch input, as well as a timer
  Output for all 3 inputs is associated with LEDs: green (motion sensor), red (Button) and yellow (Slide Switch)
  Timer will constantly blink blue LED at approx 500ms intervals
*/

// Set variable & constants
const int  motion = 2;			// Assign motion sensor to DPin# 2
const int button = 3;			// Assign button to DPin# 3
const int slide_switch = 6;		// Assign button to DPin# 6
const int LED_green = 11;		// Assign LED Green to Pin# 11 - motion
const int LED_red = 10;			// Assign LED Red to Pin# 10 - button
const int LED_yellow = 12;		// Assign LED Yellow to Pin# 12 - temp
const int LED_blue = 13;		// Assign LED Blue to Pin# 13 - timer
volatile int motion_flag = 0;	// set motion Flag varaible as volatile
volatile int button_state = 0;	// set button State varaible as volatile
volatile int switch_state = 0;	// set switch State varaible as volatile

// Program setup code
void setup()
{
  pinMode(motion,INPUT_PULLUP); // initialise sensor as input
  pinMode(button,INPUT_PULLUP); // initialise button as input
  pinMode(slide_switch,INPUT_PULLUP);// initialise temp controller as input
  pinMode(LED_green, OUTPUT);	// initialise Green LED as output
  pinMode(LED_red, OUTPUT);		// initialise Red LED as output
  pinMode(LED_yellow, OUTPUT);	// initialise Yellow LED as output
  pinMode(LED_blue, OUTPUT);	// initialise Yellow LED as output
  
  PCICR |= B00000100;			// PCIE2 Activated
  PCMSK2 |= B01001100;			// D2, D3,D6 will trigger interrupt
   
  noInterrupts();
  TCCR1A = 0;					// Clear regs
  TCCR1B = 0;	
  TCNT1 = 0;	

  TCCR1B |= (1 << CS12);		// set to prescaler 256
  TCCR1B &= ~(1 << CS11);
  TCCR1B &= ~(1 << CS10);
  
  OCR1A = 31250;
  
  TIMSK1 = (1 << OCIE1A);		// Enable timer and compare interrupt  TIMSK1 = (1 << OCIE1A);		// Enable timer and compare interrupt
  TCCR1B |=(1<< WGM12);

  interrupts();
  Serial.begin(9600); 		// initialise serial at 9600bps
}

// Program Main 
void loop()
{
  Serial.println();
  Serial.print("No Input Detected");	// constant loop for monitoring
  delay(500);
}

//Interrupt function

ISR(PCINT2_vect)	// Digital Pins 0-7
{
  if(digitalRead(motion))
  {
    motion_flag = HIGH;
    digitalWrite(LED_green,motion_flag);
    Serial.println();
    Serial.print(motion_flag);
    Serial.print("\t");
    Serial.print("Motion Detected");
  }
  else
  {
    motion_flag = LOW;
    digitalWrite(LED_green,motion_flag);
  }
  
  if(digitalRead(button))
  {
    button_state = HIGH;
    digitalWrite(LED_red,button_state);
    Serial.println();
    Serial.print(button_state);
    Serial.print("\t");
    Serial.print("Button Pressed");
  }
  else
  {
    button_state = LOW;
    digitalWrite(LED_red,button_state);
  }
  
  if(digitalRead(slide_switch))
  {
    switch_state = HIGH;
    digitalWrite(LED_yellow,switch_state);
    Serial.println();
    Serial.print(switch_state);
    Serial.print("\t");
    Serial.print("Switch on");
  }
  else
  {
    switch_state = LOW;
    digitalWrite(LED_yellow,switch_state);
  }
}

ISR(TIMER1_COMPA_vect){
  digitalWrite(LED_blue,digitalRead(LED_blue)^1);
}
