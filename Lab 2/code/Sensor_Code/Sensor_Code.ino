/*Libraries*/
#include <SPI.h>
#include <Wire.h>
#include <MPU6050_tockn.h>
#include "RF24.h"
#include "Sensor_Template.h"

MPU6050 MyMPU6050 (Wire);
long timer = 0;
int GyroData[2];

/*Pin Declarations*/
const int LLLSensor = A0, LLSensor = A1, LSensor = A2, RSensor = A3, RRSensor = A6, RRRSensor = A7;
RF24 radio(9,10);

/*Global Variables*/
float SReading[6] = {0,0,0,0,0,0};          /*Sensor Readings Array*/
float SWeight[6] = {-60,-40,-15,15,35,60};  /*Weighting for each sensor in mm*/

float WeightedTotal = 0, SensorTotal = 0, PreviousError = 0, CurrentError; /*PID variables*/
float Proportional, Integral = 0, Differential, PIDoutput;

char Direction; /*PID output variables*/
int Speed;

int ButtonXYKpKiKdValues[6]; /*RF remote array*/
char XYDirection[2];

void setup() {
  radio.begin();                              /*Start the radio module*/
  radio.setPALevel(RF24_PA_LOW);              /*Set the transmission power to low*/
  radio.setDataRate(RF24_250KBPS);            /*Set the speed to 250kbps*/
  radio.setChannel(CHANNELNUMBER);            /*Set the channel*/
  radio.openWritingPipe(rfAddresses[0]);      /*Transmit to the remote address*/
  radio.openReadingPipe(1, rfAddresses[1]);   /*Recieve sensor addressed packets*/
  radio.setPayloadSize(PAYLOADSIZE);          /*Set the number of bytes for the data packet*/

  Serial.begin (9600);
  
  Wire.begin(SENSOR);   /*Join I2C as the sensor*/

  MyMPU6050.begin();                  /*Start the MPU sensor*/
  MyMPU6050.calcGyroOffsets(true);    /*Calculate the offsets*/
  delay(3000);                        /*3 second delay for offsets to be calculated*/
}

void loop() {
  radio.startListening();
  radio.read(&ButtonXYKpKiKdValues, sizeof(ButtonXYKpKiKdValues));
//  PrintAllIncomingInfo();
  
   if (ButtonXYKpKiKdValues[0] == 0) {
    ReadSensorValues();
    CalculateLFSpeedAndDirection();
//    PrintAllSensorAndSpeedInformation();
    SendLFSpeedAndDirectionToCar();
    
    PreviousError = CurrentError;   /*Save the current error as previous error for the next loop*/
    WeightedTotal = 0;              /*and reset the weighted total and sensor total back to 0*/
    SensorTotal = 0;
  }

  else if (ButtonXYKpKiKdValues[0] == 1) {
    CalculateRFSpeedAndDirection();
//    PrintAllJoyStickInfo();
    SendRFSpeedAndDirectionToCar();
    SendGyroDataToRemote();
    delay(75);
  }
}





void PrintAllIncomingInfo() {
  Serial.print("Button = ");
  Serial.print(ButtonXYKpKiKdValues[0]);
  Serial.print("   X Value = ");
  Serial.print(ButtonXYKpKiKdValues[1]);
  Serial.print("   Y Value = ");
  Serial.print(ButtonXYKpKiKdValues[2]);
  Serial.print("   Kp = ");
  Serial.print(ButtonXYKpKiKdValues[3]);
  Serial.print("   Ki = ");
  Serial.print(ButtonXYKpKiKdValues[4]);
  Serial.print("   Kd = ");
  Serial.println(ButtonXYKpKiKdValues[5]);
}

void ReadSensorValues() {
/*The analogue sensor is read, then constrained within a range,*/
/*then mapped (within that same range) to a value between 0 and 9*/
  SReading[0] = map(constrain(analogRead(LLLSensor), 120, 690), 120, 690, 0, 9); 
  SReading[1] = map(constrain(analogRead(LLSensor), 130, 640), 130, 640, 0, 9);
  SReading[2] = map(constrain(analogRead(LSensor), 160, 710), 160, 710, 0, 9);
  SReading[3] = map(constrain(analogRead(RSensor), 90, 690), 90, 690, 0, 9);
  SReading[4] = map(constrain(analogRead(RRSensor), 390, 710), 390, 710, 0, 9);
  SReading[5] = map(constrain(analogRead(RRRSensor), 50, 430), 50, 430, 0, 9);
}

void CalculateLFSpeedAndDirection() {
/*Calculate the current error value (weighted average)*/
  for (int i=0; i<=5; i++) {
    WeightedTotal += SWeight[i] * SReading[i];
    SensorTotal += SReading[i];
  }
  
  CurrentError = WeightedTotal/SensorTotal;
  
/*Calculate PID value*/
  Proportional = CurrentError;
  Integral += CurrentError;
  Differential = CurrentError - PreviousError;

/*                    Kp = ButtonXYKpKiKdValues[3]       Ki = ButtonXYKpKiKdValues[4]           Kd = ButtonXYKpKiKdValues[5]*/
  PIDoutput = Proportional*ButtonXYKpKiKdValues[3] + Integral*ButtonXYKpKiKdValues[4] + Differential*ButtonXYKpKiKdValues[5];

/*Work out direction*/
  if (PIDoutput > 15) {            /*Then line is to the left, so car needs to turn left*/                   
    Direction = 'L';
    Speed = PIDoutput;
    
  } else if (PIDoutput < -15) {    /*Then line is to the right, so car needs to turn right*/
    Direction = 'R';
    Speed = -PIDoutput;
    
  } else {                         /*Then line is inbetween sensors so no change*/
    Direction = 'N';
    Speed = 0;
  }
}

void PrintAllSensorAndSpeedInformation() {
  Serial.print("Button = ");
  Serial.print(ButtonXYKpKiKdValues[0]);
  Serial.print("\t");
  Serial.print(SReading[0]);
  Serial.print("\t");
  Serial.print(SReading[1]);
  Serial.print("\t");
  Serial.print(SReading[2]);
  Serial.print("\t");
  Serial.print(SReading[3]);
  Serial.print("\t");
  Serial.print(SReading[4]);
  Serial.print("\t");
  Serial.print(SReading[5]);
  Serial.print("\t");
  Serial.print("Direction = ");
  Serial.print(Direction);
  Serial.print("   Speed = ");
  Serial.println(Speed);
}

void SendLFSpeedAndDirectionToCar() {
  Wire.beginTransmission(CAR);
  Wire.write(ButtonXYKpKiKdValues[0]);
  Wire.write(Speed);
  Wire.write(Direction);
  Wire.endTransmission();
}

void CalculateRFSpeedAndDirection() {
  if (ButtonXYKpKiKdValues[1] > 0) 
        XYDirection[0] = 'R';
  
    else if (ButtonXYKpKiKdValues[1] < 0) {
        XYDirection[0] = 'L';
        ButtonXYKpKiKdValues[1] *= -1;
    
    } else
        XYDirection[0] = 'N';
    
  
    if (ButtonXYKpKiKdValues[2] > 0)
        XYDirection[1] = 'F';
    
    else if (ButtonXYKpKiKdValues[2] < 0) {
        XYDirection[1] = 'B';
        ButtonXYKpKiKdValues[2] *= -1;
    
    } else 
        XYDirection[1] = 'N';
}

void PrintAllJoyStickInfo () {
  Serial.print(XYDirection[0]);
  Serial.print(" X Value = ");
  Serial.print(ButtonXYKpKiKdValues[1]);
  Serial.print("\t");
  Serial.print(XYDirection[1]);
  Serial.print(" Y Value = ");
  Serial.println(ButtonXYKpKiKdValues[2]);
}


void SendRFSpeedAndDirectionToCar() {
  Wire.beginTransmission(CAR);
  Wire.write(ButtonXYKpKiKdValues[0]);
  Wire.write(ButtonXYKpKiKdValues[1]);
  Wire.write(ButtonXYKpKiKdValues[2]);
  Wire.write(XYDirection[0]);
  Wire.write(XYDirection[1]);
  Wire.endTransmission();
}

void SendGyroDataToRemote() {
  MyMPU6050.update();
  GyroData[0] = MyMPU6050.getAccX();
  GyroData[1] = MyMPU6050.getAngleY();

  radio.stopListening();
  radio.write(&GyroData, sizeof(GyroData));
}
