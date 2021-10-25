/*Libraries*/
#include <Wire.h>
#include <Servo.h>
#include <TM1637Display.h>
#include <MPU6050.h> 
#include "SR04.h"
#include "Sensor_Template.h"

/*Pin Declarations*/
const int TrigPin = 7;
const int EchoPin = 8;

const int ServoPin = 2; //Brown = GND    Red = 5V    Orange = D9

const int CLK = 4; //Set the CLK pin connection to the display
const int DIO = 3; //Set the DIO pin connection to the display

SR04 MySR04 = SR04(EchoPin, TrigPin);
Servo CameraServo;
TM1637Display DistanceDisplay (CLK, DIO);
MPU6050 MyMPU6050;


int Value = 2, PiDistance;


void setup() {
  Serial.begin (9600);
  Wire.begin(SENSOR);   /*Join I2C as the sensor*/
  Wire.onReceive(receiveEvent);
  
  CameraServo.attach(ServoPin);

  DistanceDisplay.setBrightness(0x0a); //set the diplay to maximum brightness

  MyMPU6050.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G);
}

void loop() {
  switch(Value)
  {
    case 0:
      Serial.println("Idle"); //turn off display
      break;


    case 1:
      Serial.println("Moving camera up");
      CameraServo.write(90); //turn off display
      break;


    case 2:
      Serial.println("Moving camera down");
      CameraServo.write(15); //turn off display 
      break;


    case 3:
      Serial.print("Measuring distance and displaying");
      PiDistance = MySR04.Distance();
      DistanceDisplay.showNumberDec(PiDistance);
      Serial.println(PiDistance);
      break;

    
    case 4: /*add delay so that car cabn get over incline and then can return to black line following */
      Serial.println("Measuring angle and displaying");  
      Vector normAccel = MyMPU6050.readNormalizeAccel();
      int Angle = (atan2(normAccel.XAxis, sqrt(normAccel.YAxis*normAccel.YAxis + normAccel.ZAxis*normAccel.ZAxis))*180.0)/M_PI;
      DistanceDisplay.showNumberDec(Angle);
      Serial.print("  Angle = ");
      Serial.print(Angle);
      break;

      
    default:
      Serial.println("Error in Value");  //turn off display
      break;
  }
  
  Value = 0;

  delay(10);
}

void receiveEvent(int howMany) {
  Value = Wire.read();
  int temp = Wire.read();

  delay(10);
}
