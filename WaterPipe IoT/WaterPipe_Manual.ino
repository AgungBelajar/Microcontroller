#define compressor 2
#define sol_buka 3
#define sol_tutup 4
#define button_buka 5
#define button_tutup 6
#define selektor_auto 7
#define selektor_manual 7

void setup() {
  Serial.begin(115200);
  pinMode(compressor, OUTPUT);
  pinMode(sol_buka, OUTPUT);
  pinMode(sol_tutup, OUTPUT);
  pinMode(button_buka, INPUT);
  pinMode(button_tutup, INPUT);
  pinMode(selektor_auto, INPUT);
  pinMode(selektor_manual, INPUT);

  digitalWrite(compressor, HIGH);
  digitalWrite(sol_buka, HIGH);
  digitalWrite(sol_tutup, HIGH);
}

void loop() {
  int val_buka = digitalRead(button_buka);
  int val_tutup = digitalRead(button_tutup);
  int val_sel_au = digitalRead(selektor_auto);
  int val_sel_man = digitalRead(selektor_manual);

  Serial.print("buka = ");
  Serial.print(val_buka);
  Serial.print("  ; tutup = ");
  Serial.print(val_tutup);
  if (val_sel_au == HIGH) {
    Serial.println("  ; auto");

  }

  else if (val_sel_man == HIGH) {
    Serial.println("  ; manual");
    if (val_buka == HIGH && val_tutup == LOW)
    {
      digitalWrite(compressor, LOW);
      delay(2000);
      digitalWrite(compressor, LOW);
      digitalWrite(sol_buka, LOW);
      Serial.println("BUKA");
      delay(20000);
      digitalWrite(sol_buka, HIGH);
      digitalWrite(compressor, HIGH);
    }

    else if (val_tutup == HIGH && val_buka == LOW)
    {
      digitalWrite(compressor, LOW);
      delay(2000);
      digitalWrite(compressor, LOW);
      digitalWrite(sol_tutup, LOW);
      Serial.println("TUTUP");
      delay(20000);
      digitalWrite(sol_tutup, HIGH);
      digitalWrite(compressor, HIGH);
    }
    else
    {
      digitalWrite(compressor, HIGH);
      digitalWrite(sol_tutup, HIGH);
      Serial.println("TIDAK BERGERAK");
    }
  }
  else {
    Serial.println("  ; OFF");
  }

  delay(1000);
}
