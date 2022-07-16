#include <Arduino.h>
#include <SdFat.h>
#include <TimeLib.h>
#include "Adafruit_10DOF.h"
#include "Adafruit_ADS1X15.h"
#include "FreqMeasureMulti.h"

//Accel
Adafruit_L3GD20_Unified gyro  = Adafruit_L3GD20_Unified(20);
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(30301);

//Shock Pot
//Adafruit_ADS1115 Ads1115s;
int16_t frontRight,frontLeft,rearRight,rearLeft;



#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = SS;
#else  // SDCARD_SS_PIN
// Assume built-in SD is used.
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN

#if HAS_SDIO_CLASS
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif  ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)
#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK)
#endif  // HAS_SDIO_CLASS
#define SD_FAT_TYPE 1
#if SD_FAT_TYPE == 0
SdFat sd;
typedef File file_t;
#elif SD_FAT_TYPE == 1
SdFat32 sd;
typedef File32 file_t;
#elif SD_FAT_TYPE == 2
SdExFat sd;
typedef ExFile file_t;
#elif SD_FAT_TYPE == 3
SdFs sd;
typedef FsFile file_t;
#else  // SD_FAT_TYPE
#error Invalid SD_FAT_TYPE
#endif  // SD_FAT_TYPE

#define FILE_BASE_NAME "Run-"
file_t myFile;
const uint8_t BASE_NAME_SIZE=sizeof(FILE_BASE_NAME)-1;
char fileName[] = FILE_BASE_NAME "00.txt";


void setup() {

Serial.begin(115200);
delay(100);
Serial.print("initialized");
/* RTC
if (timeStatus()!= timeSet) {
  Serial.println("Unable to sync with the RTC");
} 
else {
  Serial.println("RTC has set the system time");
}
setSyncProvider(Teensy3Clock.get);
*/
//Sensor Setup
if(!accel.begin())
  {
  Serial.println(F("Accel error detected"));
  while(1);
  }
if(!gyro.begin())
  {
  Serial.println(F("Gyro error detected"));
  while(1);
  }


//Sd Config 
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
  }

  while (sd.exists(fileName)) {
    if (fileName[BASE_NAME_SIZE + 1] != '9') {
      fileName[BASE_NAME_SIZE + 1]++;
    } 
    else if (fileName[BASE_NAME_SIZE] != '9') {
      fileName[BASE_NAME_SIZE + 1] = '0';
      fileName[BASE_NAME_SIZE]++;
    } 
    else {
      Serial.println(F("Can't create file name"));
    return;
    }
  }
  if (!myFile.open(fileName, FILE_WRITE)) {
    sd.errorHalt("opening file for write failed");
  }
  myFile.println(fileName);
  myFile.println("Time(Millis), aX, aY, aZ, gX, gY, gZ, Shock_FL, Shock_FR, Shock_RL, Shock_RR");

}
/*
String AccelLog(){
  //create event 
  sensors_event_t event;

  //get event
  accel.getEvent(&event);
  gyro.getEvent(&event);

  //Sensor Datapoints
  float AccelX = event.acceleration.x;
  float AccelY = event.acceleration.y;
  float AccelZ = event.acceleration.z;
  float GyroX = event.gyro.x;
  float GyroY = event.gyro.y;
  float GyroZ = event.gyro.z;
  char accel[6];
  accel[0] = AccelX;
  accel[1] = AccelY;
  accel[2] = AccelZ;
  accel[3] = GyroX;
  accel[4] = GyroY;
  accel[5] = GyroZ;

  return accel;
}

String ShockLog(){
  char Shock[4];
  Shock[0] = analogRead(A0);
  Shock[1] = analogRead(A1);
  Shock[2] = analogRead(A2);
  Shock[3] = analogRead(A3);
  return Shock;
}

*/

void loop() {
  sensors_event_t event;

  //Time
  int mills = millis();

  //get event
  accel.getEvent(&event);
  
  //Accel Datapoints
  float AccelX = event.acceleration.x;
  float AccelY = event.acceleration.y;
  float AccelZ = event.acceleration.z;

  //Gyro Datapoints
  gyro.getEvent(&event);

  float GyroX = event.gyro.x;
  float GyroY = event.gyro.y;
  float GyroZ = event.gyro.z;

  //shockpot datapointa
  int Shock_FL = analogRead(A0);
  int Shock_FR = analogRead(A1);
  int Shock_RL = analogRead(A2);
  int Shock_RR = analogRead(A3);

char buffer[80];
  int n;
  n=sprintf(buffer,"%i,%f,%f,%f,%f,%f,%f,%i,%i,%i,%i\n",mills,AccelX,AccelY,AccelZ,GyroX,GyroY,GyroZ,Shock_FL,Shock_FR,Shock_RL,Shock_RR);

myFile.open(fileName,FILE_WRITE);
myFile.write(buffer);
myFile.close();
Serial.print(buffer);
delay(50);
}