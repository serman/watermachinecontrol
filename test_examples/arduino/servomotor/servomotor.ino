#include <Servo.h> 
int i;
bool up;
Servo myservo;  // create servo object to control a servo 
void setup() {
  // put your setup code here, to run once:
  myservo.attach(9);
  up=false;
  i=0;
}

void loop() {  
  // put your main code here, to run repeatedly:
  if(up==false)
    myservo.write(40);
  else
    myservo.write(90);

  if(up==true) 
    { i++;
      if(i>10) up=false;
    }
   else{
      i--; 
      if(i<0) up=true;    
   }
   
  delay(100);
}
