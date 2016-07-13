#include <Servo.h>

Servo myservo;

#define PIN_PISTOLA 13
#define TIEMPO_DISPARO 2500

int byteRead = 0; //Valor del byte leido del programa de python
int valServo; 

int minAngle = 60;
int maxAngle = 90;
char serialdata[10];

void setup() {
  myservo.attach(9);
  //myservo.write(75); // Start position, servo offset -15      
  //delay(1000);          

  Serial.begin(9600);
  pinMode(PIN_PISTOLA, OUTPUT);
  digitalWrite(PIN_PISTOLA, LOW);
}

void loop() {

   //Test Servo min-max positions
   //myservo.write(minAngle);  
   //delay(1000);       
   //myservo.write(maxAngle);     
   //delay(1000);          
   
  if( Serial.available() > 0 ) {
    if(Serial.readBytesUntil(10,serialdata,3) >0)
      if(serialdata[0]=='b'){
        //DISPARO
        //Serial.println('bang!');
        digitalWrite(PIN_PISTOLA, HIGH);
        delay(TIEMPO_DISPARO);
        digitalWrite(PIN_PISTOLA, LOW);
        //delay(TIEMPO_DISPARO);      
      }
      if(serialdata[0]=='s'){
        //servo
        valServo = byteRead;           
      //valServo = map(valServo, 0, 255, minAngle, maxAngle); //Mapeado desde arduino   
        myservo.write(valServo-15);  //valor ya mapeado desde python -15 offset
        delay(1000);
        
      }    
    }
}
