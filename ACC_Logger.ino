#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <ADXL345.h>
#include <Adafruit_Sensor.h>
ADXL345 adxl; //variable adxl is an instance of the ADXL345 library

// globals
#define cardSelect 4

File logfile;
int writeLED=9; //status LED

//variables for processing measurements
unsigned long start, finished, elapsed;

// blink out an error code
void error(uint8_t errno) {
  while(1) {
    uint8_t i;
    for (i=0; i<errno; i++) {
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
      delay(100);
    }
    for (i=errno; i<10; i++) {
      delay(200);
    }
  }
}


void setup() {
  // connect at 115200 to monitor device
  // also spit it out
  Serial.begin(115200);
  //while (!Serial) ;
  Serial.println("\r\nAnalog logger test");
  
  //switch on accelerometer
  adxl.powerOn();
  //set range to +-16
  adxl.setRangeSetting(16);
  //switch on self test
  adxl.setSelfTestBit(0);
  adxl.setFullResBit(1);
  pinMode(13, OUTPUT);
  pinMode(writeLED, OUTPUT);

  // see if the card is present
  if (!SD.begin(cardSelect)) {
    Serial.println("Card init. failed!");
    error(2);
  }
  char filename[15];
  strcpy(filename, "ANALOG00.CSV");
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = '0' + i/10;
    filename[7] = '0' + i%10;
    // create if does not exist, do not open existing, write, sync after write
    if (! SD.exists(filename)) {
      break;
    }
  }

  logfile = SD.open(filename, FILE_WRITE);
  if( ! logfile ) {
    Serial.print("Couldnt create "); 
    Serial.println(filename);
    error(3);
  }
  
  Serial.print("Writing to "); 
  Serial.println(filename);

  pinMode(13, OUTPUT);
  Serial.println("Ready!");

  Serial.println("x,y,z"); //print column names
  logfile.println("x,y,z");
}

uint8_t i=0;
void loop() {
  //measure processing time
   start=millis();
  //get the last filename
   char filename[15];
  strcpy(filename, "ANALOG00.CSV");
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = '0' + i/10;
    filename[7] = '0' + i%10;
   if (!SD.exists(filename)) {
    filename[6] = '0' + (i-1)/10;
    filename[7] = '0' + (i-1)%10;
      break;
    }
  }

  //open the logfile
  File logfile = SD.open(filename, FILE_WRITE);
  
  if(logfile){
  //read the accelerometer values and store them in variables  x,y,z
  int x,y,z;  
  adxl.readXYZ(&x, &y, &z);
  
  //make variables for data
  double xyz[3];
  double ax,ay,az;
  
  //calculate g forces
  adxl.getAcceleration(xyz);
  ax = xyz[0];
  ay = xyz[1];
  az = xyz[2];

  //write data to serial
  Serial.print(ax);
  Serial.print(",");
  Serial.print(ay);
  Serial.print(",");
  Serial.println(az);
  
  //write data to file
  logfile.print(ax);
  logfile.print(",");
  logfile.print(ay);
  logfile.print(",");
  logfile.println(az);
  
  //flash writeLED when writing
  digitalWrite(writeLED, HIGH);
  
  //intervall of writing
  delay(10);
  
  //close the logfile
  logfile.close();
  //switch of writeLED when finished writing
  digitalWrite(writeLED, LOW);
  Serial.print("Processing Time:");
  finished=millis();
  elapsed=finished-start;
  Serial.println(elapsed);
  
  }
}
