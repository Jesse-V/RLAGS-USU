#include <Time.h>

#include<OneWire.h>
#include<DallasTemperature.h>


#define DTS_PRECISION 12  // thermal sensor resolution (bits)

#define ONE_WIRE_BUS1 10  //   etalon DTS -> Arduino pin 10
#define ONE_WIRE_BUS2 11  // powerBox DTS -> Arduino pin 11
#define ONE_WIRE_BUS3 12  //     sedi DTS -> Arduino pin 12
#define ONE_WIRE_BUS4 13  //  control DTS -> Arduino pin 13
#define ONE_WIRE_BUS5 14  // attitude DTS -> Arduino pin 14

#define RELAY_CTRL_CH1 2 // thermalControl -> Arduino pin 2 
#define RELAY_CTRL_CH2 3 // thermalControl -> Arduino pin 3 
#define RELAY_CTRL_CH3 4 // thermalControl -> Arduino pin 4 
#define RELAY_CTRL_CH4 5 // thermalControl -> Arduino pin 5 
#define RELAY_CTRL_CH5 6 // thermalControl -> Arduino pin 6 
#define RELAY_CTRL_CH6 7 // thermalControl -> Arduino pin 7 
#define RELAY_CTRL_CH7 8 // thermalControl -> Arduino pin 8 
#define RELAY_CTRL_CH8 9 // thermalControl -> Arduino pin 9

#define ERROR_TEMP -127.00
#define LOWER_HIST_BOUND 20.5
#define UPPER_HIST_BOUND 21.5

// setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire_etalon(ONE_WIRE_BUS1);
//OneWire oneWire_power(ONE_WIRE_BUS2);
OneWire oneWire_sedi(ONE_WIRE_BUS3);
//OneWire oneWire_control(ONE_WIRE_BUS4);
//OneWire oneWire_attitude(ONE_WIRE_BUS5);

// pass oneWire reference to Dallas Temperature.
DallasTemperature sensors_etalon(&oneWire_etalon);
//DallasTemperature sensors_power(&oneWire_power);
DallasTemperature sensors_sedi(&oneWire_sedi);
//DallasTemperature sensors_control(&oneWire_control);
//DallasTemperature sensors_attitude(&oneWire_attitude);

/*-----------------------------etalon DTS------------------------------*/
DeviceAddress DTS_1 = { 0x28, 0x03, 0x9A, 0x8E, 0x05, 0x00, 0x00, 0xB4 };
DeviceAddress DTS_2 = { 0x28, 0xB4, 0x97, 0x8F, 0x05, 0x00, 0x00, 0x55 };
DeviceAddress DTS_3 = { 0x28, 0xE9, 0x92, 0x8E, 0x05, 0x00, 0x00, 0x6C };
DeviceAddress DTS_4 = { 0x28, 0x62, 0xA2, 0x8F, 0x05, 0x00, 0x00, 0x75 };
/*---------------------------------------------------------------------*/

/*----------------------------powerBox DTS-----------------------------*/
DeviceAddress DTS_5 = { 0x28, 0x34, 0x6B, 0x8E, 0x05, 0x00, 0x00, 0xB7 };
DeviceAddress DTS_6 = { 0x28, 0xD2, 0xBF, 0x8F, 0x05, 0x00, 0x00, 0xE2 };
DeviceAddress DTS_7 = { 0x28, 0x20, 0x3C, 0x8E, 0x05, 0x00, 0x00, 0xF4 };
/*---------------------------------------------------------------------*/

/*------------------------------sedi DTS--------------------------------*/
DeviceAddress DTS_8  = { 0x28, 0xC1, 0xF0, 0x8E, 0x05, 0x00, 0x00, 0xE9 };
DeviceAddress DTS_9  = { 0x28, 0xA6, 0xBE, 0x8E, 0x05, 0x00, 0x00, 0xE4 };
DeviceAddress DTS_10 = { 0x28, 0x79, 0xBC, 0x8F, 0x05, 0x00, 0x00, 0x08 };
DeviceAddress DTS_11 = { 0x28, 0x57, 0xB2, 0x8F, 0x05, 0x00, 0x00, 0x0F };
/*----------------------------------------------------------------------*/

/*-----------------------------control DTS------------------------------*/
DeviceAddress DTS_12 = { 0x28, 0x74, 0x8A, 0x8F, 0x05, 0x00, 0x00, 0x5A };
DeviceAddress DTS_13 = { 0x28, 0x0D, 0x82, 0x8E, 0x05, 0x00, 0x00, 0xE5 };
DeviceAddress DTS_14 = { 0x28, 0x1C, 0xD0, 0x8F, 0x05, 0x00, 0x00, 0x10 };
DeviceAddress DTS_15 = { 0x28, 0xE2, 0x89, 0x8F, 0x05, 0x00, 0x00, 0x17 };
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

/*-----------------------------attitude DTS-----------------------------*/
//DeviceAddress DTS_16 = {  };
//DeviceAddress DTS_17 = {  };
//DeviceAddress DTS_18 = {  };
//DeviceAddress DTS_19 = {  };
//DeviceAddress DTS_20 = {  };
//DeviceAddress DTS_21 = {  };
//DeviceAddress DTS_22 = {  };
//DeviceAddress DTS_23 = {  };
/*----------------------------------------------------------------------*/


int relayStateCh1, relayStateCh2, relayStateCh3, relayStateCh4,
    relayStateCh5, relayStateCh6, relayStateCh7, relayStateCh8;

void setup() {
  // initialization
  relayStateCh1 = LOW;
  relayStateCh2 = LOW;
  relayStateCh3 = LOW;
  relayStateCh4 = LOW;
  relayStateCh5 = LOW;
  relayStateCh6 = LOW;
  relayStateCh7 = LOW;
  relayStateCh8 = LOW;

  pinMode(RELAY_CTRL_CH1, OUTPUT);
  pinMode(RELAY_CTRL_CH2, OUTPUT);
  pinMode(RELAY_CTRL_CH3, OUTPUT);
  pinMode(RELAY_CTRL_CH4, OUTPUT);
  pinMode(RELAY_CTRL_CH5, OUTPUT);
  pinMode(RELAY_CTRL_CH6, OUTPUT);
  pinMode(RELAY_CTRL_CH7, OUTPUT);
  pinMode(RELAY_CTRL_CH8, OUTPUT);

  Serial.begin(19200);  // start serial port

  sensors_etalon.begin();   // start up library for etalon sensors
  //  sensors_power.begin();    // start up library for powerBox sensors
  sensors_sedi.begin();     // start up library for sedi sensors
  //  sensors_control.begin();  // start up library for control sensors
  //  sensors_attitude.begin(); // start up library for attitude sensors

  // set the resolution
  /*-------------------etalon DTS------------------*/
  sensors_etalon.setResolution(DTS_1, DTS_PRECISION);
  sensors_etalon.setResolution(DTS_2, DTS_PRECISION);
  sensors_etalon.setResolution(DTS_3, DTS_PRECISION);
  sensors_etalon.setResolution(DTS_4, DTS_PRECISION);
  /*-----------------------------------------------*/

  /*-----------------powerBox DTS------------------*/
  //  sensors_power.setResolution(DTS_5,  DTS_PRECISION);
  //  sensors_power.setResolution(DTS_6,  DTS_PRECISION);
  //  sensors_power.setResolution(DTS_7, DTS_PRECISION);
  /*-----------------------------------------------*/

  /*-------------------sedi DTS-------------------*/
  sensors_sedi.setResolution(DTS_8 , DTS_PRECISION);
  sensors_sedi.setResolution(DTS_9 , DTS_PRECISION);
  sensors_sedi.setResolution(DTS_10, DTS_PRECISION);
  sensors_sedi.setResolution(DTS_11, DTS_PRECISION);
  /*----------------------------------------------*/

  /*--------------------control DTS------------------*/
  //  sensors_control.setResolution(DTS_12, DTS_PRECISION);
  //  sensors_control.setResolution(DTS_13, DTS_PRECISION);
  //  sensors_control.setResolution(DTS_14, DTS_PRECISION);
  //  sensors_control.setResolution(DTS_15, DTS_PRECISION);
  /*-------------------------------------------------*/

  //  /*--------------------control DTS-------------------*/
  //  sensors_attitude.setResolution(DTS_16, DTS_PRECISION);
  //  sensors_attitude.setResolution(DTS_17, DTS_PRECISION);
  //  sensors_attitude.setResolution(DTS_18, DTS_PRECISION);
  //  sensors_attitude.setResolution(DTS_19, DTS_PRECISION);
  //  sensors_attitude.setResolution(DTS_20, DTS_PRECISION);
  //  sensors_attitude.setResolution(DTS_21, DTS_PRECISION);
  //  sensors_attitude.setResolution(DTS_22, DTS_PRECISION);
  //  sensors_attitude.setResolution(DTS_23, DTS_PRECISION);
  //  /*--------------------------------------------------*/

  Serial.print("\rAcquiring Data...\n\r");

  Serial.print("KAH1\t");
  Serial.print("DTS8\t");

  Serial.print("KAH2\t");
  Serial.print("DTS9\t");

  Serial.print("KAH3\t");
  Serial.print("DTS10\t");

  Serial.print("KAH4\t");
  Serial.print("DTS11\t");

  Serial.print("DTS1\t");
  Serial.print("DTS2\t");
  Serial.print("DTS3\t");
  Serial.print("DTS4\n\r");
}

void loop() {
  long startTime = millis();
  // relay control based on a thermal sensor
  tempControl_sedi();

  /*----------------etalon DTS---------------*/
  sensors_etalon.requestTemperatures();

  thermalInfo(sensors_etalon, DTS_1, degC_1);
  Serial.print("\t");  
  thermalInfo(sensors_etalon, DTS_2, degC_2);
  Serial.print("\t");  
  thermalInfo(sensors_etalon, DTS_3, degC_3);
  Serial.print("\t");  
  thermalInfo(sensors_etalon, DTS_4, degC_4);

//  thermalInfo(sensors_etalon, DTS_1);
//  Serial.print("\t");  
//  thermalInfo(sensors_etalon, DTS_2);
//  Serial.print("\t");  
//  thermalInfo(sensors_etalon, DTS_3);
//  Serial.print("\t");  
//  thermalInfo(sensors_etalon, DTS_4);
//
//  Serial.print((startTime-millis()));
//  Serial.print("\t");

  Serial.print("\n\r");
  /*-----------------------------------------*/

  //  /*-----------powerBox DTS-----------*/
  //  sensors_power.requestTemperatures();
  //  Serial.print("PowerBox temperature:");
  //  Serial.print("\n\r");
  //  Serial.print("Sensor #5: ");
  //  printTemp_power(DTS_5);
  //  Serial.print("\n\r");
  //  Serial.print("Sensor #6: ");
  //  printTemp_power(DTS_6);
  //  Serial.print("\n\r");
  //  Serial.print("Sensor #7: ");
  //  printTemp_power(DTS_7);
  //  Serial.print("\n\r\n\r");
  //  /*----------------------------------*/

  //  /*-------------control DTS------------*/
  //  sensors_control.requestTemperatures();
  //  Serial.print("ControlBox temperature:");
  //  Serial.print("\n\r");
  //  Serial.print("Sensor #12: ");
  //  printTemp_control(DTS_12);
  //  Serial.print("\n\r");
  //  Serial.print("Sensor #13: ");
  //  printTemp_control(DTS_14);
  //  Serial.print("\n\r");
  //  Serial.print("Sensor #14: ");
  //  printTemp_control(DTS_14);
  //  Serial.print("\n\r");
  //  Serial.print("Sensor #15: ");
  //  printTemp_control(DTS_15);
  //  Serial.print("\n\r\n\r");
  //  /*------------------------------------*/
}

//void printTemp_power(DeviceAddress deviceAddress) {
//  float tempC = sensors_power.getTempC(deviceAddress);
//  if (tempC == ERROR_TEMP) {
//    Serial.print("Error");
//  }
//
//  else {
//    Serial.print(tempC);
//  }
//}

//void printTemp_control(DeviceAddress deviceAddress) {
//  float tempC = sensors_control.getTempC(deviceAddress);
//  if (tempC == ERROR_TEMP) {
//    Serial.print("Error");
//  }
//
//  else {
//    Serial.print(tempC);
//  }
//}

//void printTemp_attitude(DeviceAddress deviceAddress) {
//  float tempC = sensors_attitude.getTempC(deviceAddress);
//  if (tempC == ERROR_TEMP) {
//    Serial.print("Error");
//  }
//
//  else {
//    Serial.print(tempC);
//  }
//}

void tempControl_sedi() {
  sensors_sedi.requestTemperatures();
//  thermalCtrl(sensors_sedi, DTS_8, RELAY_CTRL_CH1, relayStateCh1);
//  thermalCtrl(sensors_sedi, DTS_9, RELAY_CTRL_CH2, relayStateCh2);
//  thermalCtrl(sensors_sedi, DTS_10, RELAY_CTRL_CH3, relayStateCh3);
//  thermalCtrl(sensors_sedi, DTS_11, RELAY_CTRL_CH4, relayStateCh4);
  thermalCtrl(sensors_sedi, DTS_8, RELAY_CTRL_CH1, relayStateCh1, degC_8);
  thermalCtrl(sensors_sedi, DTS_9, RELAY_CTRL_CH2, relayStateCh2, degC_9);
  thermalCtrl(sensors_sedi, DTS_10, RELAY_CTRL_CH3, relayStateCh3, degC_10);
  thermalCtrl(sensors_sedi, DTS_11, RELAY_CTRL_CH4, relayStateCh4, degC_11);
}

//void thermalInfo(DallasTemperature dataLine, DeviceAddress& dts)
//{
//  float tempC = dataLine.getTempC(dts);
//  tempC += retrieveOffset(dts);
//  if (tempC == ERROR_TEMP) {
//    Serial.print("Error");
//  }
//
//  else {
//    Serial.print(tempC);
//  }
//}
//
//void thermalCtrl(DallasTemperature dataLine, DeviceAddress& dts, int pin, int relayStateCh)
//{
//  float tempC = dataLine.getTempC(dts);
//  tempC = tempC + retrieveOffset(dts);
//
//  if (tempC != ERROR_TEMP) {
//    if (tempC <= LOWER_HIST_BOUND) {
//      relayStateCh = HIGH;
//      Serial.print("1\t");
//    }
//    else if (tempC >= UPPER_HIST_BOUND) {
//      relayStateCh = LOW;
//      Serial.print("0\t");
//    }
//    else if (tempC < UPPER_HIST_BOUND && tempC > LOWER_HIST_BOUND) {
//      if (relayStateCh == LOW) {
//        Serial.print("0\t");
//      }
//      else {
//        Serial.print("1\t");
//      }
//    }
//  }
//  else {
//    relayStateCh = LOW;
//    Serial.print("0\t");
//    Serial.print("Error\t");
//  }
//  digitalWrite(pin, relayStateCh);
//  Serial.print(tempC);
//  Serial.print("\t");
//}

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

void thermalCtrl(DallasTemperature dataLine, DeviceAddress& dts, int pin, int relayStateCh, float* arr)
{
  float tempC = dataLine.getTempC(dts) + retrieveOffset(dts);
  
  if (tempC != ERROR_TEMP) {
    addAverage(arr, tempC);
    tempC = getAverage(arr);
    if (tempC <= LOWER_HIST_BOUND) {
      relayStateCh = HIGH;
      Serial.print("1\t");
    }
    else if (tempC >= UPPER_HIST_BOUND) {
      relayStateCh = LOW;
      Serial.print("0\t");
    }
    else if (tempC < UPPER_HIST_BOUND && tempC > LOWER_HIST_BOUND) {
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
  return retVal;
}

