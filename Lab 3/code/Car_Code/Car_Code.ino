/*Libraires*/
#include <Wire.h>
#include <H61AEE_S02.h>
#include "Car_Template.h"

/*Global Variables*/
char Direction;                               /*Line Following Mode*/
int Speed, BaseSpeed = 64, Sum, Difference;   /*Line Following Mode*/

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
  Sum = BaseSpeed + Speed;

  if (Sum > 255)
    Sum = 255;

  Difference = BaseSpeed - Speed;
  if (Difference < 0)
    Difference = 0;

  if (Speed == 0)
    car.setSpeed(ALL, 0);

  else if (Speed < 64)
  {
    car.setDirection(ALL, forwards);
    car.setSpeed(ALL, BaseSpeed);
  }

  else
  {
    switch (Direction) {

      case 'R':
        car.setDirection(RIGHT, forwards);
        car.setDirection(LEFT, backwards);
        car.setSpeed(RIGHT, 96);
        car.setSpeed(LEFT, 96);
        break;

      case 'L':
        car.setDirection(RIGHT, backwards);
        car.setDirection(LEFT, forwards);
        car.setSpeed(RIGHT, 96);
        car.setSpeed(LEFT, 96);
        break;

      default:
        Serial.println("Line Following Error");
        break;
    }
    delay (10);
  }
}

void receiveEvent(int howMany) {
  Speed = Wire.read();
  int temp = Wire.read();

  if (temp == 0)
    Direction = 'R';
  
  else if (temp == 1)
    Direction = 'L';  

  Serial.print(" Speed = ");
  Serial.print(Speed);
  Serial.print("  temp = ");
  Serial.println(temp);

  delay(10);
}
