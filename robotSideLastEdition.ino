#include <Arduino.h>

#define txPin 12
#define rxPin 13

#define espSerial Serial2


#define vSpeed 100       // MAX 255
#define turn_speed 100  // MAX 255 
#define turn_delay 0
#define enablePinR 4
#define enablePinL 5

#define RLedPin 33
#define GLedPin 0
#define BLedPin 16
#define YLedPin 2
  
//L293 Connection   
#define motorRp  18   //1
#define motorRn 21   //2

#define motorLp  14   //3
#define motorLn  15   //4 


//Sensor Connection
#define right_sensor_pin 23
#define left_sensor_pin 22

#define trigger 26
#define echo 25
// servo
#define servo 19

  int left_sensor_state;
  int right_sensor_state;
  int distance_F,distance_L,distance_R;
  int set =20;
  int numOfStops=2;

  bool isStop = false; 

  bool receivedCharStat1  = false;
  bool receivedLed1  = false;
  bool img1 = false ;

  bool receivedCharStat2  = false;
  bool receivedLed2  = false;
  bool img2 = false ;
  bool readyToGo=false;
  
void setup() {
    espSerial.begin(115200,SERIAL_8N1,rxPin,txPin); // Initialize serial communication (replace with baud rate)
      Serial.begin(115200);

  pinMode(trigger, OUTPUT);
  pinMode(echo, INPUT);

  pinMode(motorRp, OUTPUT);
  pinMode(motorRn, OUTPUT);
  pinMode(motorLp, OUTPUT);
  pinMode(motorLn, OUTPUT);

  pinMode(enablePinR, OUTPUT);
  pinMode(enablePinL, OUTPUT);

  pinMode(RLedPin,OUTPUT);
  pinMode(GLedPin,OUTPUT);
  pinMode(BLedPin,OUTPUT);
  pinMode(YLedPin,OUTPUT);



  digitalWrite (motorRp,LOW);
  digitalWrite (motorRn,LOW);
  digitalWrite (motorLp,LOW);
  digitalWrite (motorLn,LOW);

 
    


  pinMode(left_sensor_pin,INPUT);
  pinMode(right_sensor_pin,INPUT);
  
  pinMode(servo, OUTPUT); 

  rotateCamCenter();

  delay(2000);
  
}

void loop() 
{
  
  while(!readyToGo){
    receiveStartSignal();
        digitalWrite(RLedPin,HIGH);
        // digitalWrite(GLedPin,HIGH);
        // digitalWrite(BLedPin,HIGH);
        // digitalWrite(YLedPin,HIGH);
  }
        digitalWrite(RLedPin,LOW);
        digitalWrite(GLedPin,LOW);
        digitalWrite(BLedPin,LOW);
        digitalWrite(YLedPin,LOW);

        digitalWrite(GLedPin,HIGH);

    if(Ultrasonic_read()>10){
      if(!isStop){
    left_sensor_state = digitalRead(left_sensor_pin);
    right_sensor_state = digitalRead(right_sensor_pin);

    if(right_sensor_state == HIGH && left_sensor_state == LOW) goRight();

    else if(right_sensor_state == LOW && left_sensor_state == HIGH) goLeft();

    else if(right_sensor_state == LOW && left_sensor_state == LOW) goForword();

    else if(right_sensor_state == HIGH && left_sensor_state == HIGH)  stop();

}
else 
Serial.println("waitiong for response");
    }
    else{
      finalStop();
    }

}

void finalStop(){
  digitalWrite (motorRp,LOW);
  digitalWrite (motorLp,LOW);

  digitalWrite(motorRn,LOW);  
  digitalWrite (motorLn,LOW);

  analogWrite(enablePinR, 0);

  analogWrite(enablePinL, 0);
  Serial.println("final stop...");

}
void goLeft(){
  Serial.println("turning left");

  digitalWrite (motorLp,HIGH);
  digitalWrite (motorLn,LOW);
  
  digitalWrite (motorRp,LOW);
  digitalWrite (motorRn,LOW);//0000

  analogWrite(enablePinR, 0);

  analogWrite(enablePinL, vSpeed+20);

  delay(turn_delay);
}

void goRight(){
  Serial.println("turning right");

  digitalWrite (motorRp,HIGH);
  digitalWrite (motorRn,LOW);

  digitalWrite (motorLp,LOW);
  digitalWrite (motorLn,LOW);//00


  analogWrite(enablePinR, vSpeed);
  analogWrite(enablePinL, 0);

  delay(turn_delay);
}

void goForword(){

  Serial.println("going forward");

  digitalWrite(motorRp,HIGH);  
  digitalWrite (motorLp,HIGH);
  
  digitalWrite(motorRn,LOW);  
  digitalWrite (motorLn,LOW);
  
  analogWrite(enablePinR, vSpeed);

  analogWrite(enablePinL, vSpeed);
  
  
}

void stop(){
  Serial.println("stop");
   
  digitalWrite (motorRp,LOW);
  digitalWrite (motorLp,LOW);

  digitalWrite(motorRn,LOW);  
  digitalWrite (motorLn,LOW);

  analogWrite(enablePinR, 0);

  analogWrite(enablePinL, 0);
  isStop=true;
  rotateCamRigh();

  delay(100);
  sendChar('0');
  receiveResponse1();
  numOfStops--;
  
}

long Ultrasonic_read(){
  digitalWrite(trigger, LOW);
  delayMicroseconds(2);
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  long time = pulseIn (echo, HIGH);
  return time / 29 / 2;
}


void servoPulse (int pin, int angle){
  int pwm = (angle*11) + 500;      // Convert angle to microseconds
 digitalWrite(pin, HIGH);
 delayMicroseconds(pwm);
 digitalWrite(pin, LOW);
 delay(50); // Refresh cycle of servo
}
void rotateCamLeft(){
  for (int angle = 180; angle >= 0; angle -= 5)  {
   servoPulse(servo, angle);
     }
    Serial.println("Left pic=");

}
void rotateCamRigh(){
   for (int angle = 90; angle <= 180; angle += 5)  {
   servoPulse(servo, angle);  
   }
    Serial.println("Righ pic");
   
}
void rotateCamCenter(){
   for (int angle = 0; angle <= 90; angle += 5)  {
   servoPulse(servo, angle); 
    }
    Serial.println("cam Center");

}
void sendChar(char msgChar){
    if (espSerial.availableForWrite()) {  // Check if ready to write
      espSerial.write(msgChar);             // Send data (e.g., 'x') on button press
      Serial.println("Sent data: "+ char(msgChar) );
    } else {
      Serial.println("Serial not ready");
    }
}
void receiveResponse1(){
  while(!receivedLed1 || !receivedCharStat1 ){
 if(espSerial.available()){
    char data = char(espSerial.read());
    Serial.print("Received data: ");
        Serial.println(data);

   if(data=='8'){
        Serial.println("pic done");
        receivedCharStat1 =true;
    }
    else if(data=='1'){
      Serial.println("led1");
      receivedLed1=true;
    }
    else if(data=='2'){
      Serial.println("led2");
      // digitalWrite(GLedPin,HIGH);
      receivedLed1=true;
    }
    else if(data=='3'){
      Serial.println("led3");
    //  digitalWrite(BLedPin,HIGH);
      receivedLed1=true;
    }
    else if(data=='4'){
      Serial.println("led4");
      // digitalWrite(YLedPin,HIGH);

      receivedLed1=true;
    }
    else if(data=='5'){
      Serial.println("led5");
          // digitalWrite(RLedPin,HIGH);
      receivedLed1=true;
    }
      else if(data=='6'){
      Serial.println("led6");
          // digitalWrite(RLedPin,HIGH);
      receivedLed1=true;
    }
    else{
         digitalWrite(RLedPin,HIGH);
              receivedLed1=true;

    }


    delay(2000);
      Serial.println("all led off");
        digitalWrite(RLedPin,LOW);
        digitalWrite(GLedPin,LOW);
        digitalWrite(BLedPin,LOW);
        digitalWrite(YLedPin,LOW);

 }
  }
  receivedCharStat1=false;
  receivedLed1=false;

  rotateCamLeft();
  delay(100);
  sendChar('0');
  receiveResponse2();

}
void receiveResponse2(){
  while(!receivedLed2 || !receivedCharStat2 ){
    if(espSerial.available()){
      char data = espSerial.read();
    Serial.print("Received data: ");
        Serial.println(data);

    if(data=='8'){
        Serial.println("pic done");
        receivedCharStat2 =true;
    }
    else if(data=='1'){
      Serial.println("led1");
      receivedLed2=true;
    }
    else if(data=='2'){
      Serial.println("led2");
        // digitalWrite(GLedPin,HIGH);
      receivedLed2=true;
    }
    else if(data=='3'){
      Serial.println("led3");
          // digitalWrite(BLedPin,HIGH);
      receivedLed2=true;
    }
    else if(data=='4'){
      Serial.println("led4");
        // digitalWrite(YLedPin,HIGH);

      receivedLed2=true;
    }
    else if(data=='5'){
      Serial.println("led5");
        // digitalWrite(RLedPin,HIGH);
      receivedLed2=true;
    }
     else if(data=='6'){
      Serial.println("led6");
        // digitalWrite(RLedPin,HIGH);
      receivedLed2=true;
    }
    
    else{
         digitalWrite(RLedPin,HIGH);
      receivedLed2=true;

    }
    delay(2000);
      Serial.println("all led off");
        digitalWrite(RLedPin,LOW);
        digitalWrite(GLedPin,LOW);
        digitalWrite(BLedPin,LOW);
        digitalWrite(YLedPin,LOW);
    }

  }
  receivedCharStat2=false;
  receivedLed2=false;
  rotateCamCenter();
  delay(100);
  goForword();
  delay(500);
  isStop=false;
  
}
void receiveStartSignal(){
  
   char data = char(espSerial.read());
    Serial.print("Received data: ");
        Serial.println(data);

  if(data=='9')
  readyToGo=true;
}