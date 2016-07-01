
#define PIN_PISTOLA 9

#define TIEMPO_DISPARO 000

int byteRead = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(PIN_PISTOLA, OUTPUT);
  digitalWrite(PIN_PISTOLA, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  if( Serial.available() > 0 ) {
    byteRead = Serial.read();
    if ( byteRead == 'b' ) {
      Serial.println('hola');
      digitalWrite(PIN_PISTOLA, HIGH);
      delay(TIEMPO_DISPARO);
      digitalWrite(PIN_PISTOLA, LOW);
    }
  }
}
