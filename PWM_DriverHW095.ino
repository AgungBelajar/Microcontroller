int ka1 = 2;
int ka2 = 3;
int PWM = 5;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(ka1, OUTPUT);
  pinMode(ka2, OUTPUT);
  pinMode(PWM,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(ka1, HIGH);
  digitalWrite(ka2, LOW);
  analogWrite(PWM,100);
  Serial.println("max kiri");
  delay(5000);

  for (int i = 0; i <= 255; i += 10)
  {
    digitalWrite(ka1, HIGH);
    digitalWrite(ka2, LOW);
    analogWrite(PWM,i);
    Serial.print(String(i) +"  ..  ");
    delay(500);
  }

  digitalWrite(ka1, LOW);
  digitalWrite(ka2, LOW);
  Serial.println("off");
  delay(1000);

  for (int i = 0; i <= 255; i += 10)
  {
    analogWrite(ka1, LOW);
    digitalWrite(ka2, HIGH);
    analogWrite(PWM,i);
    Serial.print(String(i) +"  ..  ");
    delay(500);
  }

  digitalWrite(ka1, LOW);
  digitalWrite(ka2, LOW);
  Serial.println("off");
  delay(1000);
  
  digitalWrite(ka1, LOW);
  digitalWrite(ka2, HIGH);
  analogWrite(PWM,100);
  Serial.println("max kanan");
  delay(3000);

  digitalWrite(ka1, LOW);
  digitalWrite(ka2, LOW);
  Serial.println("off");
  delay(1000);
}
