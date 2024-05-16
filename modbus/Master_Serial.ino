int incoming;
String message = "";
int incomingString;
int data = 10;
void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
Serial3.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
Serial3.write(data);
delay(1);
//delay(3000);
while (Serial3.available())
{
    incoming = Serial3.read();
      message += incoming;
    Serial.println(incoming);
    delay(100);  
}
delay (3000);
}
