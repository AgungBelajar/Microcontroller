#include <ModbusMaster.h>
#define MAX485_DE 3
#define MAX485_RE_NEG 2

ModbusMaster node;
void preTransmission()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}
void postTransmission()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}


void setup() {
  // put your setup code here, to run once:
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);

  pinMode(4,INPUT);
  pinMode(5,INPUT);
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
  
  Serial.begin(115200);
  node.begin(1,Serial);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
}

void loop() {
  // put your main code here, to run repeatedly:
  float value = analogRead(A0);
  node.writeSingleRegister(0x40000,value);
  
  int a= digitalRead(4);
  int b= digitalRead(5);
   if (a == 1)
  {
    node.writeSingleRegister(0x40001,1);
  }
  else
  {
    node.writeSingleRegister(0x40001,0);
  }
  if (b == 1)
  {
    node.writeSingleRegister(0x40002,1);
  }
  else
  {
    node.writeSingleRegister(0x40002,0);
  }
}
