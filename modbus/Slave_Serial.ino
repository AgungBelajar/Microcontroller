int incoming;
String message = "";
char incomingChar;
int data = 100;
void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
Serial3.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
if (Serial3.available())
{
  char incomingChar = Serial3.read();
    if (incomingChar != '\n'){
      message += String(incomingChar);
    }
    else{
      message = "";
    }
    Serial.println(message);  
  }
  if (message =="10"){
    Serial3.println(data);
    Serial.println("udah kekirim");
  }
}
