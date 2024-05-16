unsigned long interval=1000; // the time we need to wait
unsigned long previousMillis=0; // millis() returns an unsigned long.
extern volatile unsigned long timer0_millis;

bool ledState = false; // state variable for the LED
  
void setup() {
 Serial.begin(9600);
}
  
void loop() {
 unsigned long currentMillis = millis(); // grab current time
  
 // check if "interval" time has passed (1000 milliseconds)
 if ((unsigned long)(currentMillis - previousMillis) >= interval) {
   
//   ledState = !ledState; // "toggles" the state
   Serial.print("Millis : ");
   Serial.println(previousMillis); // sets the LED based on ledState
   // save the "current" time
   previousMillis = millis();

 if (previousMillis == 10000)
 {
  Serial.println("RESET");
  noInterrupts();
  timer0_millis=0;
  interrupts();
 }
 }
}
