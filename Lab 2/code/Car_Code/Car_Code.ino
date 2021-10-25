/*Libraires*/
#include <Wire.h>
#include <H61AEE_S02.h>
#include "Car_Template.h"

/*Global Variables*/
int ButtonValue;

char Direction;
int Speed;

int XValue, YValue;
char XDirection, YDirection;

int BaseSpeed = 48, Sum, Difference;

void setup() {
  car.setupVehicle();
  car.enableMotors(true);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  Wire.begin(CAR);
  Wire.onReceive(receiveEvent);
  Serial.begin(9600);
}

void loop() {
  if (ButtonValue == 1) {
//    PrintAllJoyStickInfo();
    RFMotorControl();
  } 
  
  else {
//    PrintAllLineFollowingInfo();
    PIDMotorControl();
  }
  delay (1);
}





void receiveEvent(int howMany) {
  ButtonValue = Wire.read();
  
  if (ButtonValue == 1) {
  XValue = Wire.read();
  YValue = Wire.read();
  XDirection = Wire.read();
  YDirection = Wire.read();
  
  } else {
  Speed = Wire.read();
  Direction = Wire.read();
  }
}

void PrintAllJoyStickInfo () {
  Serial.print(XDirection);
  Serial.print(" X Value = ");
  Serial.print(XValue);
  Serial.print("\t");
  Serial.print(YDirection);
  Serial.print(" Y Value = ");
  Serial.println(YValue);
}

void RFMotorControl() {
  Difference = YValue - XValue;
  
  if (Difference < 0)
        Difference = 0;
    
  switch (YDirection) {
    case 'N':
      car.setSpeed(ALL, 0);
      break;

    case 'F':
      switch (XDirection) {
        case 'N':
          car.setDirection(ALL, forwards);
          car.setSpeed(ALL, YValue);
          break;

       case 'R':
          car.setDirection(ALL, forwards);
          car.setSpeed(LEFT, YValue);
          car.setSpeed(RIGHT, Difference);
          break;

        case 'L':
          car.setDirection(ALL, forwards);
          car.setSpeed(LEFT, Difference);
          car.setSpeed(RIGHT, YValue);
         break;

        default:
          Serial.println("RF Remote X Error");
          break;
      }
      break;

    case 'B':
       switch (XDirection) {
        case 'N':
          car.setDirection(ALL, backwards);
          car.setSpeed(ALL, YValue);
          break;

       case 'R':
          car.setDirection(ALL, backwards);
          car.setSpeed(LEFT, YValue);
          car.setSpeed(RIGHT, Difference);
          break;

        case 'L':
          car.setDirection(ALL, backwards);
          car.setSpeed(LEFT, Difference);
          car.setSpeed(RIGHT, YValue);
          break;

        default:
          Serial.println("RF Remote X Error");
          break;
      }
      break;
      
    default:
        Serial.println("RF Remote Y Error");
        break;  
  }
}

void PrintAllLineFollowingInfo () {
  Serial.print("Direction = ");
  Serial.print(Direction);
  Serial.print(" Speed = ");
  Serial.println(Speed);
}

void PIDMotorControl() {
  Sum = BaseSpeed + Speed;
  Difference = BaseSpeed - Speed;
  
  if (Sum > 255)
        Sum = 255;

  if (Difference < 0)
        Difference = 0;
  
  switch(Direction) {
    case 'N':
        car.setDirection(ALL, forwards);  
        car.setSpeed(RIGHT, BaseSpeed);
        car.setSpeed(LEFT, BaseSpeed);
        break;
    
    case 'R':
        car.setDirection(RIGHT, backwards);  
        car.setDirection(LEFT, forwards);  
        car.setSpeed(RIGHT, Sum);
        car.setSpeed(LEFT, Sum);
        break;  

    case 'L':
        car.setDirection(RIGHT, forwards);  
        car.setDirection(LEFT, backwards);
        car.setSpeed(RIGHT, Sum);
        car.setSpeed(LEFT, Sum);
        break;

    default:
        Serial.println("Line Following Error");
        break;
  }
}
