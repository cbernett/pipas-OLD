#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

/* This driver reads raw data from the BNO055

   Connections
   ===========
   Connect SCL to analog 5
   Connect SDA to analog 4
   Connect VDD to 3.3V DC
   Connect GROUND to common ground

   History
   =======
   2015/MAR/03  - First release (KTOWN)
*/



Adafruit_BNO055 bno = Adafruit_BNO055();

unsigned long lastSendTime = 0;
unsigned long lastSampleTime = 0;

//DESIRED sample rate (in milliseconds)
//At minimum (only printing timestamp/lin accel/euler) sample will
//go down to ~40ms since Serial.print is a slow method
#define SAMPLE_DELAY 100


//smoothing variables
const int numReadings = 2;    // <----- NUMBER OF SAMPLES TO AVERAGE

float Xreadings[numReadings];      // the readings from the input
int XreadIndex = 0;              // the index of the current reading
float Xtotal = 0;                  // the running total
float Xaverage = 0;                // the average

float Yreadings[numReadings];      // the readings from the input
int YreadIndex = 0;              // the index of the current reading
float Ytotal = 0;                  // the running total
float Yaverage = 0;                // the average

float Zreadings[numReadings];      // the readings from the input
int ZreadIndex = 0;              // the index of the current reading
float Ztotal = 0;                  // the running total
float Zaverage = 0;                // the average


/**************************************************************************/
/*
    Arduino setup function (automatically called at startup)
*/
/**************************************************************************/
void setup(void)
{
  Serial.begin(9600);
  Serial.println("Orientation Sensor Raw Data Test"); Serial.println("");

  /* Initialise the sensor */
  if (!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while (1);
  }

  delay(500);

  /* Display the current temperature */
  int8_t temp = bno.getTemp();
  Serial.print("Current Temperature: ");
  Serial.print(temp);
  Serial.println(" C");
  Serial.println("");

  bno.setExtCrystalUse(true);

  Serial.println("Calibration status values: 0=uncalibrated, 3=fully calibrated");

  Serial.println("time\teul_z\teul_y\teul_x\tlinA_x\tlinA_y\tlinA_z\tcal_sys\tcal_gyr\tcal_acc\tcal_mag");

  //initialize smoothing arrays
  for (int i = 0; i < numReadings; i++) {
    Xreadings[i] = 0;
    Yreadings[i] = 0;
    Zreadings[i] = 0;
  }

}

/**************************************************************************/
/*
    Arduino loop function, called once 'setup' is complete (your own code
    should go here)
*/
/**************************************************************************/
void loop(void)
{


  if (millis() - lastSampleTime > SAMPLE_DELAY) {


    //print the sample time
    Serial.print(millis() - lastSendTime);
    Serial.print('\t');

    lastSendTime = millis();

    imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
    imu::Vector<3> linearAcceleration = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);

    //print Euler data
    for (int i = 0; i < 3; ++i) {
      Serial.print(euler[i]);
      Serial.print('\t');
    }



    //------------------------------SMOOTHING OF LINEAR ACCEL DATA------------------------------

    //----------X DIRECTION----------

    // subtract the last reading:
    Xtotal = Xtotal - Xreadings[XreadIndex];
    // read from the sensor:
    Xreadings[XreadIndex] = linearAcceleration[0];
    // add the reading to the total:
    Xtotal = Xtotal + Xreadings[XreadIndex];
    // advance to the next position in the array:
    XreadIndex = XreadIndex + 1;

    // if we're at the end of the array...
    if (XreadIndex >= numReadings) {
      // ...wrap around to the beginning:
      XreadIndex = 0;
    }

    // calculate the average:
    Xaverage = Xtotal / numReadings;

    //----------Y DIRECTION----------

    // subtract the last reading:
    Ytotal = Ytotal - Yreadings[YreadIndex];
    // read from the sensor:
    Yreadings[YreadIndex] = linearAcceleration[1];
    // add the reading to the total:
    Ytotal = Ytotal + Yreadings[YreadIndex];
    // advance to the next position in the array:
    YreadIndex = YreadIndex + 1;

    // if we're at the end of the array...
    if (YreadIndex >= numReadings) {
      // ...wrap around to the beginning:
      YreadIndex = 0;
    }

    // calculate the average:
    Yaverage = Ytotal / numReadings;


    //----------Z DIRECTION----------

    // subtract the last reading:
    Ztotal = Ztotal - Zreadings[ZreadIndex];
    // read from the sensor:
    Zreadings[ZreadIndex] = linearAcceleration[2];
    // add the reading to the total:
    Ztotal = Ztotal + Zreadings[ZreadIndex];
    // advance to the next position in the array:
    ZreadIndex = ZreadIndex + 1;

    // if we're at the end of the array...
    if (ZreadIndex >= numReadings) {
      // ...wrap around to the beginning:
      ZreadIndex = 0;
    }

    // calculate the average:
    Zaverage = Ztotal / numReadings;



    //print SMOOTHED linear Acceleration data
    Serial.print(Xaverage);
    Serial.print('\t');
    Serial.print(Yaverage);   
    Serial.print('\t');
    Serial.print(Zaverage);
    Serial.print('\t');


    //print RAW linAccel data
    //    for (int i = 0; i < 3; ++i) {
    //      Serial.print(linearAcceleration[i]);
    //      Serial.print('\t');
    //    }




//    /* Display calibration status for each sensor. */
//    uint8_t system, gyro, accel, mag = 0;
//    bno.getCalibration(&system, &gyro, &accel, &mag);
//    //  Serial.print("CALIBRATION: Sys=");
//    Serial.print(system, DEC);
//    Serial.print('\t');
//    //  Serial.print(" Gyro=");
//    Serial.print(gyro, DEC);
//    Serial.print('\t');
//    //  Serial.print(" Accel=");
//    Serial.print(accel, DEC);
//    Serial.print('\t');
//    //  Serial.print(" Mag=");
//    Serial.print(mag, DEC);

    Serial.println();

    //record time
    lastSampleTime = millis();

  }

}
