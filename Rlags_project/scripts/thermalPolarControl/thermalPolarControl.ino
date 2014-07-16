//#include <Time.h>
#include<OneWire.h>
#include<DallasTemperature.h>

#define DTS_PRECISION 12  // thermal sensor resolution (bits)

#define ONE_WIRE_BUS1 10  //     etalon DTS -> Arduino pin 10
#define ONE_WIRE_BUS2 11  //   powerBox DTS -> Arduino pin 11
#define ONE_WIRE_BUS3 12  //       sedi DTS -> Arduino pin 12
#define ONE_WIRE_BUS4 13  //    control DTS -> Arduino pin 13
#define ONE_WIRE_BUS5 15  //  attitude1 DTS -> Arduino pin 15 / A1

#define DIGITAL_SERVO 16  //   SEDI polarizer -> Arduino pin 16 / A2
#define CALIBR_ENABLE 17  // SEDI calibration -> Arduino pin 17 / A3

#define RELAY_CTRL_CH1 2  // thermalControl -> Arduino pin 2
#define RELAY_CTRL_CH2 3  // thermalControl -> Arduino pin 3
#define RELAY_CTRL_CH3 4  // thermalControl -> Arduino pin 4
#define RELAY_CTRL_CH4 5  // thermalControl -> Arduino pin 5
#define RELAY_CTRL_CH5 6  // thermalControl -> Arduino pin 6
#define RELAY_CTRL_CH6 7  // thermalControl -> Arduino pin 7
//#define RELAY_CTRL_CH7 8  // thermalControl -> Arduino pin 8
//#define RELAY_CTRL_CH8 9  // thermalControl -> Arduino pin 9

#define ERROR_TEMP -127.00
// set SEDI Temperature @ 20C, Hysteresis @ 1C
// set Camera Temperature @ 6C, Hysteresis @ 1C
#define LOWER_HYST_BOUND1 19.5
#define UPPER_HYST_BOUND1 20.5
#define LOWER_HYST_BOUND2 5.5
#define UPPER_HYST_BOUND2 6.5

// setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire_etalon(ONE_WIRE_BUS1);
OneWire oneWire_control(ONE_WIRE_BUS2);
OneWire oneWire_powerBox(ONE_WIRE_BUS3);
OneWire oneWire_odroidX2(ONE_WIRE_BUS4);
OneWire oneWire_attitude(ONE_WIRE_BUS5);

// pass oneWire reference to Dallas Temperature.
DallasTemperature sensors_etalon(&oneWire_etalon);
DallasTemperature sensors_control(&oneWire_control);
DallasTemperature sensors_powerBox(&oneWire_powerBox);
DallasTemperature sensors_odroidX2(&oneWire_odroidX2);
DallasTemperature sensors_attitude(&oneWire_attitude);

/*-----------------------------etalon DTS------------------------------*/
DeviceAddress DTS_1 = { 0x28, 0x03, 0x9A, 0x8E, 0x05, 0x00, 0x00, 0xB4 };
DeviceAddress DTS_2 = { 0x28, 0xB4, 0x97, 0x8F, 0x05, 0x00, 0x00, 0x55 };
DeviceAddress DTS_3 = { 0x28, 0xE9, 0x92, 0x8E, 0x05, 0x00, 0x00, 0x6C };
DeviceAddress DTS_4 = { 0x28, 0x62, 0xA2, 0x8F, 0x05, 0x00, 0x00, 0x75 };
/*---------------------------------------------------------------------*/

/*-----------------------------control DTS------------------------------*/
DeviceAddress DTS_5  = { 0x28, 0xC1, 0xF0, 0x8E, 0x05, 0x00, 0x00, 0xE9 };
DeviceAddress DTS_6  = { 0x28, 0xA6, 0xBE, 0x8E, 0x05, 0x00, 0x00, 0xE4 };
DeviceAddress DTS_7  = { 0x28, 0x79, 0xBC, 0x8F, 0x05, 0x00, 0x00, 0x08 };
DeviceAddress DTS_8  = { 0x28, 0x57, 0xB2, 0x8F, 0x05, 0x00, 0x00, 0x0F };
DeviceAddress DTS_9  = { 0x28, 0xAC, 0xDB, 0xE7, 0x05, 0x00, 0x00, 0xEB };
DeviceAddress DTS_10 = { 0x28, 0x3D, 0xF7, 0xE7, 0x05, 0x00, 0x00, 0xB4 };
/*----------------------------------------------------------------------*/

/*----------------------------powerBox DTS-----------------------------*/
DeviceAddress DTS_11 = { 0x28, 0x34, 0x6B, 0x8E, 0x05, 0x00, 0x00, 0xB7 };
DeviceAddress DTS_12 = { 0x28, 0xD2, 0xBF, 0x8F, 0x05, 0x00, 0x00, 0xE2 };
DeviceAddress DTS_13 = { 0x28, 0x20, 0x3C, 0x8E, 0x05, 0x00, 0x00, 0xF4 };
/*---------------------------------------------------------------------*/

/*-----------------------------odroidX2 DTS-----------------------------*/
DeviceAddress DTS_14 = { 0x28, 0x74, 0x8A, 0x8F, 0x05, 0x00, 0x00, 0x5A };
DeviceAddress DTS_15 = { 0x28, 0x0D, 0x82, 0x8E, 0x05, 0x00, 0x00, 0xE5 };
DeviceAddress DTS_16 = { 0x28, 0x1C, 0xD0, 0x8F, 0x05, 0x00, 0x00, 0x10 };
DeviceAddress DTS_17 = { 0x28, 0xE2, 0x89, 0x8F, 0x05, 0x00, 0x00, 0x17 };
/*----------------------------------------------------------------------*/

/*-----------------------------attitude DTS-----------------------------*/
DeviceAddress DTS_18 = { 0x28, 0xDE, 0xE8, 0xE7, 0x05, 0x00, 0x00, 0xD7 };
DeviceAddress DTS_19 = { 0x28, 0x17, 0x2C, 0xE8, 0x05, 0x00, 0x00, 0x79 };
DeviceAddress DTS_20 = { 0x28, 0xE1, 0xEC, 0xE8, 0x05, 0x00, 0x00, 0x9B };
DeviceAddress DTS_21 = { 0x28, 0x30, 0x79, 0xE8, 0x05, 0x00, 0x00, 0x0D };
DeviceAddress DTS_22 = { 0x28, 0x74, 0x90, 0xE8, 0x05, 0x00, 0x00, 0x8D };
DeviceAddress DTS_23 = { 0x28, 0x10, 0x07, 0xE8, 0x05, 0x00, 0x00, 0x74 };
/*----------------------------------------------------------------------*/

float degC_1[3] = {0, 0, 0};
float degC_2[3] = {0, 0, 0};
float degC_3[3] = {0, 0, 0};
float degC_4[3] = {0, 0, 0};
float degC_5[3] = {0, 0, 0};
float degC_6[3] = {0, 0, 0};
float degC_7[3] = {0, 0, 0};
float degC_8[3] = {0, 0, 0};
float degC_9[3] = {0, 0, 0};
float degC_10[3] = {0, 0, 0};
float degC_11[3] = {0, 0, 0};
float degC_12[3] = {0, 0, 0};
float degC_13[3] = {0, 0, 0};
float degC_14[3] = {0, 0, 0};
float degC_15[3] = {0, 0, 0};
float degC_16[3] = {0, 0, 0};
float degC_17[3] = {0, 0, 0};
float degC_18[3] = {0, 0, 0};
float degC_19[3] = {0, 0, 0};
float degC_20[3] = {0, 0, 0};
float degC_21[3] = {0, 0, 0};
float degC_22[3] = {0, 0, 0};
float degC_23[3] = {0, 0, 0};

int relayStateCh1, relayStateCh2, relayStateCh3, relayStateCh4,
    relayStateCh5, relayStateCh6; // relayStateCh7, relayStateCh8;

int runningChange = 0;
int lastAngle = 0;
bool waitAngleFlag = false;
int angleWaitCount = 0;


void setup() {

  // initialization
  relayStateCh1 = LOW;
  relayStateCh2 = LOW;
  relayStateCh3 = LOW;
  relayStateCh4 = LOW;
  relayStateCh5 = LOW;
  relayStateCh6 = LOW;
//  relayStateCh7 = LOW;
//  relayStateCh8 = LOW;

  pinMode(RELAY_CTRL_CH1, OUTPUT);
  pinMode(RELAY_CTRL_CH2, OUTPUT);
  pinMode(RELAY_CTRL_CH3, OUTPUT);
  pinMode(RELAY_CTRL_CH4, OUTPUT);
  pinMode(RELAY_CTRL_CH5, OUTPUT);
  pinMode(RELAY_CTRL_CH6, OUTPUT);
//  pinMode(RELAY_CTRL_CH7, OUTPUT);
//  pinMode(RELAY_CTRL_CH8, OUTPUT);

  pinMode(DIGITAL_SERVO, OUTPUT);
  pinMode(CALIBR_ENABLE, OUTPUT);

  Serial.begin(19200);  // start serial port

  sensors_etalon.begin();   // start up library for etalon sensors
  sensors_control.begin();  // start up library for active control sensors
  sensors_powerBox.begin(); // start up library for powerBox sensors
  sensors_odroidX2.begin(); // start up library for processor sensors
  sensors_attitude.begin(); // start up library for attitude sensors

  // set the resolution
  /*-------------------etalon DTS------------------*/
  sensors_etalon.setResolution(DTS_1, DTS_PRECISION);
  sensors_etalon.setResolution(DTS_2, DTS_PRECISION);
  sensors_etalon.setResolution(DTS_3, DTS_PRECISION);
  sensors_etalon.setResolution(DTS_4, DTS_PRECISION);
  /*-----------------------------------------------*/

  /*--------------------control DTS------------------*/
  sensors_control.setResolution(DTS_5, DTS_PRECISION);
  sensors_control.setResolution(DTS_6, DTS_PRECISION);
  sensors_control.setResolution(DTS_7, DTS_PRECISION);
  sensors_control.setResolution(DTS_8, DTS_PRECISION);
  sensors_control.setResolution(DTS_9, DTS_PRECISION);
  sensors_control.setResolution(DTS_10, DTS_PRECISION);
  /*-------------------------------------------------*/

  /*-------------------powerBox DTS-------------------*/
  sensors_powerBox.setResolution(DTS_11, DTS_PRECISION);
  sensors_powerBox.setResolution(DTS_12, DTS_PRECISION);
  sensors_powerBox.setResolution(DTS_13, DTS_PRECISION);
  /*--------------------------------------------------*/

  /*----------------odroidX2 DTS------------------*/
  sensors_odroidX2.setResolution(DTS_14, DTS_PRECISION);
  sensors_odroidX2.setResolution(DTS_15, DTS_PRECISION);
  sensors_odroidX2.setResolution(DTS_16, DTS_PRECISION);
  sensors_odroidX2.setResolution(DTS_17, DTS_PRECISION);
  /*----------------------------------------------*/

  /*----------------attitude DTS------------------*/
  sensors_attitude.setResolution(DTS_18, DTS_PRECISION);
  sensors_attitude.setResolution(DTS_19, DTS_PRECISION);
  sensors_attitude.setResolution(DTS_20, DTS_PRECISION);
  sensors_attitude.setResolution(DTS_21, DTS_PRECISION);
  sensors_attitude.setResolution(DTS_22, DTS_PRECISION);
  sensors_attitude.setResolution(DTS_23, DTS_PRECISION);
  /*----------------------------------------------*/

  Serial.print("\rAcquiring Data...:\r");

  Serial.print("KAH1\t");
  Serial.print("DTS5\t");

  Serial.print("KAH2\t");
  Serial.print("DTS6\t");

  Serial.print("KAH3\t");
  Serial.print("DTS7\t");

  Serial.print("KAH4\t");
  Serial.print("DTS8\t");

  Serial.print("KAH5s\t");
  Serial.print("DTS9\t");

  Serial.print("KAH8\t");
  Serial.print("DTS10\t");

  Serial.print("DTS1\t");
  Serial.print("DTS2\t");
  Serial.print("DTS3\t");
  Serial.print("DTS4\t");

  Serial.print("DTS11\t");
  Serial.print("DTS12\t");
  Serial.print("DTS13\t");

  Serial.print("DTS14\t");
  Serial.print("DTS15\t");
  Serial.print("DTS16\t");
  Serial.print("DTS17\t");

  Serial.print("DTS18\t");
  Serial.print("DTS19\t");
  Serial.print("DTS20\t");
  Serial.print("DTS21\t");
  Serial.print("DTS22\t");
  Serial.print("DTS23:\r");
}

int countCont = 0;

void loop() {
//  long startTime = millis();

  // relay control based on a thermal sensor
  thermalActive();

  if(countCont++ == 9)
  {
    countCont = 0;
    sensors_etalon.requestTemperatures();

    thermalInfo(sensors_etalon, DTS_1, degC_1);
    Serial.print("\t");
    thermalInfo(sensors_etalon, DTS_2, degC_2);
    Serial.print("\t");
    thermalInfo(sensors_etalon, DTS_3, degC_3);
    Serial.print("\t");
    thermalInfo(sensors_etalon, DTS_4, degC_4);
    Serial.print("\t");

    sensors_powerBox.requestTemperatures();
    thermalInfo(sensors_powerBox, DTS_11, degC_11);
    Serial.print("\t");
    thermalInfo(sensors_powerBox, DTS_12, degC_12);
    Serial.print("\t");
    thermalInfo(sensors_powerBox, DTS_13, degC_13);
    Serial.print("\t");

    sensors_odroidX2.requestTemperatures();
    thermalInfo(sensors_odroidX2, DTS_14, degC_14);
    Serial.print("\t");
    thermalInfo(sensors_odroidX2, DTS_15, degC_15);
    Serial.print("\t");
    thermalInfo(sensors_odroidX2, DTS_16, degC_16);
    Serial.print("\t");
    thermalInfo(sensors_odroidX2, DTS_17, degC_17);
    Serial.print("\t");

    sensors_attitude.requestTemperatures();
    thermalInfo(sensors_attitude, DTS_18, degC_18);
    Serial.print("\t");
    thermalInfo(sensors_attitude, DTS_19, degC_19);
    Serial.print("\t");
    thermalInfo(sensors_attitude, DTS_20, degC_20);
    Serial.print("\t");
    thermalInfo(sensors_attitude, DTS_21, degC_21);
    Serial.print("\t");
    thermalInfo(sensors_attitude, DTS_22, degC_22);
    Serial.print("\t");
    thermalInfo(sensors_attitude, DTS_23, degC_23);
  }

//  Serial.print((startTime-millis()));
//  Serial.print("\t");

  if(Serial.available())
  {
    byte servoAngle = Serial.read();
    if(servoAngle <= 180 && servoAngle >= 0)
    {
      if(waitAngleFlag)
      {
        angleWaitCount--;
        if(angleWaitCount <= 0)
        {
          waitAngleFlag = false;
        }
      }
      else
      {
        Serial.print("\t");
        runningChange = abs(servoAngle - lastAngle);
        if(runningChange > 170)
        {
          waitAngleFlag = true;
          angleWaitCount = 3;
          lastAngle = servoAngle;
          setServoAngle(servoAngle);
        }
        else
        {
          lastAngle = servoAngle;
          setServoAngle(servoAngle);
        }
        Serial.print(servoAngle);
      }
    }

    else if(servoAngle == 200)
    {
      Serial.print("\t");
      digitalWrite(CALIBR_ENABLE, HIGH);
      Serial.print("ON");
    }

    else if(servoAngle == 201)
    {
      Serial.print("\t");
      digitalWrite(CALIBR_ENABLE, LOW);
      Serial.print("OFF");
    }
  }

  Serial.print(":\r");
}

void thermalActive() {
  sensors_control.requestTemperatures();
  thermalCtrl(sensors_control, DTS_5, RELAY_CTRL_CH1, relayStateCh1, degC_5, LOWER_HYST_BOUND1, UPPER_HYST_BOUND1);
  thermalCtrl(sensors_control, DTS_6, RELAY_CTRL_CH2, relayStateCh2, degC_6, LOWER_HYST_BOUND1, UPPER_HYST_BOUND1);
  thermalCtrl(sensors_control, DTS_7, RELAY_CTRL_CH3, relayStateCh3, degC_7, LOWER_HYST_BOUND1, UPPER_HYST_BOUND1);
  thermalCtrl(sensors_control, DTS_8, RELAY_CTRL_CH4, relayStateCh4, degC_8, LOWER_HYST_BOUND1, UPPER_HYST_BOUND1);
  thermalCtrl(sensors_control, DTS_9, RELAY_CTRL_CH5, relayStateCh5, degC_9, LOWER_HYST_BOUND2, UPPER_HYST_BOUND2);
  thermalCtrl(sensors_control, DTS_10, RELAY_CTRL_CH6, relayStateCh6, degC_10, LOWER_HYST_BOUND2, UPPER_HYST_BOUND2);
}

void thermalInfo(DallasTemperature dataLine, DeviceAddress& dts, float* arr)
{
  float tempC = dataLine.getTempC(dts) + retrieveOffset(dts);
  addAverage(arr, tempC);
  tempC = getAverage(arr);

  if (tempC == ERROR_TEMP) {
    Serial.print("Error");
  }

  else {
    Serial.print(tempC);
  }
}

void thermalCtrl(DallasTemperature dataLine, DeviceAddress& dts, int pin, int relayStateCh, float* arr, float lower_hyst_bound, float upper_hyst_bound)
{
  float tempC = dataLine.getTempC(dts) + retrieveOffset(dts);

  if (tempC != ERROR_TEMP) {
    addAverage(arr, tempC);
    tempC = getAverage(arr);
    if (tempC <= lower_hyst_bound) {
      relayStateCh = HIGH;
      Serial.print("1\t");
    }
    else if (tempC >= upper_hyst_bound) {
      relayStateCh = LOW;
      Serial.print("0\t");
    }
    else if (tempC < upper_hyst_bound && tempC > lower_hyst_bound) {
      if (relayStateCh == LOW) {
        Serial.print("0\t");
      }
      else {
        Serial.print("1\t");
      }
    }
  }
  else {
    relayStateCh = LOW;
    Serial.print("0\t");
    Serial.print("Error\t");
  }
  digitalWrite(pin, relayStateCh);
  Serial.print(tempC);
  Serial.print("\t");
}

void addAverage(float* tArr, float var)
{
  tArr[2] = tArr[1];
  tArr[1] = tArr[0];
  tArr[0] = var;
}

float getAverage(float* tArr)
{
  return ((tArr[0] + tArr[1] + tArr[2])/3.0);
}

float retrieveOffset(DeviceAddress& dev)
{
  float retVal = 0;
  if (dev == DTS_1) {
    retVal = 0.0;
  }
  else if (dev == DTS_2) {
    retVal = 0.0;
  }
  else if (dev == DTS_3) {
    retVal = 0.0;
  }
  else if (dev == DTS_4) {
    retVal = 0.0;
  }
  else if (dev == DTS_5) {
    retVal = 0.0;
  }
  else if (dev == DTS_6) {
    retVal = 0.0;
  }
  else if (dev == DTS_7) {
    retVal = 0.0;
  }
  else if (dev == DTS_8) {
    retVal = 0.0;
  }
  else if (dev == DTS_9) {
    retVal = 0.0;
  }
  else if (dev == DTS_10) {
    retVal = 0.0;
  }
  else if (dev == DTS_11) {
    retVal = 0.0;
  }
  else if (dev == DTS_12) {
    retVal = 0.0;
  }
  else if (dev == DTS_13) {
    retVal = 0.0;
  }
  else if (dev == DTS_14) {
    retVal = 0.0;
  }
  else if (dev == DTS_15) {
    retVal = 0.0;
  }
  else if (dev == DTS_16) {
    retVal = 0.0;
  }
  else if (dev == DTS_17) {
    retVal = 0.0;
  }
  else if (dev == DTS_18) {
    retVal = 0.0;
  }
  else if (dev == DTS_19) {
    retVal = 0.0;
  }
  else if (dev == DTS_20) {
    retVal = 0.0;
  }
  else if (dev == DTS_21) {
    retVal = 0.0;
  }
  else if (dev == DTS_22) {
    retVal = 0.0;
  }
  else if (dev == DTS_23) {
    retVal = 0.0;
  }
  return retVal;
}

void setServoAngle(int servoAngle)
{
  const int SERVO_LOW = 900;
  const int SERVO_HIGH = 1980;

  // linrear mapping degree with the servo (value between 0 and 180)
  servoAngle = servoMap(servoAngle, 0, 180, SERVO_LOW, SERVO_HIGH);
  // sets servo position according to the mapped value
  setPWM(servoAngle);
}

float servoMap(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setPWM(long pwm)
{
  long start = micros();
  digitalWrite(DIGITAL_SERVO, HIGH);
  while (micros() - start < pwm);
  digitalWrite(DIGITAL_SERVO, LOW);
}
