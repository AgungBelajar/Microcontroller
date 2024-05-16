const int pinSwitch = 3; //Pin Reed
float StatoSwitch = 0 ;

void setup()
{
  pinMode(pinSwitch, INPUT);
  Serial.begin(9600);
}

float get_StatoSwitch()
{
a : if (digitalRead(pinSwitch) == LOW)
  {
    return 1.346;
  }
  delay(200);
  goto a;
}

void loop()
{
  get_StatoSwitch();
  StatoSwitch = StatoSwitch + get_StatoSwitch();
  Serial.print(StatoSwitch);
  Serial.println(" mm");
  delay(1000);
}
