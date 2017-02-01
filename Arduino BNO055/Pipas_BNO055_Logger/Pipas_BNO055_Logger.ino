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

/* Set the delay between fresh samples */
#define BNO055_SAMPLERATE_DELAY_MS (100)

Adafruit_BNO055 bno = Adafruit_BNO055();

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
  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }

  delay(1000);

  /* Display the current temperature */
  int8_t temp = bno.getTemp();
  Serial.print("Current Temperature: ");
  Serial.print(temp);
  Serial.println(" C");
  Serial.println("");

  bno.setExtCrystalUse(true);

  Serial.println("Calibration status values: 0=uncalibrated, 3=fully calibrated");

  Serial.println("accelerometer_x\taccelerometer_y\taccelerometer_z\tmagnetometer_x\tmagnetometer_y\tmagnetometer_z\teuler_x\teuler_y\teuler_z\tlinearAcceleration_x\tlinearAcceleration_y\tlinearAcceleration_z\tgravity_x\tgravity_y\tgravity_z\tquaternion_w\tquaternion_y\tquaternion_x\tquaternion_z\tcalibration_system\tcalibration_gyro\tcalibration_accel\tcalibration_mag");
}

/**************************************************************************/
/*
    Arduino loop function, called once 'setup' is complete (your own code
    should go here)
*/
/**************************************************************************/
void loop(void)
{
  // Possible vector values can be:
  // - VECTOR_ACCELEROMETER - m/s^2
  // - VECTOR_MAGNETOMETER  - uT
  // - VECTOR_GYROSCOPE     - rad/s
  // - VECTOR_EULER         - degrees
  // - VECTOR_LINEARACCEL   - m/s^2
  // - VECTOR_GRAVITY       - m/s^2
  imu::Vector<3> accelerometer = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
  imu::Vector<3> magnetometer = bno.getVector(Adafruit_BNO055::VECTOR_MAGNETOMETER);
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  imu::Vector<3> linearAcceleration = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
  imu::Vector<3> gravity = bno.getVector(Adafruit_BNO055::VECTOR_GRAVITY);

  imu::Vector<3> vecs[] = { accelerometer, magnetometer, euler, linearAcceleration, gravity };
  for(int j = 0; j < 5; ++j) {
    imu::Vector<3>& v = vecs[j];
    for(int i = 0; i < 3; ++i) {
      Serial.print(v[i]);
      Serial.print('\t');
    }
  }

  // Quaternion data
  imu::Quaternion quat = bno.getQuat();
  Serial.print(quat.w(), 4);
  Serial.print('\t');
  Serial.print(quat.y(), 4);
  Serial.print('\t');
  Serial.print(quat.x(), 4);
  Serial.print('\t');
  Serial.print(quat.z(), 4);
  Serial.print('\t');

  /* Display calibration status for each sensor. */
  uint8_t system, gyro, accel, mag = 0;
  bno.getCalibration(&system, &gyro, &accel, &mag);
//  Serial.print("CALIBRATION: Sys=");
  Serial.print(system, DEC);
  Serial.print('\t');
//  Serial.print(" Gyro=");
  Serial.print(gyro, DEC);
  Serial.print('\t');
//  Serial.print(" Accel=");
  Serial.print(accel, DEC);
  Serial.print('\t');
//  Serial.print(" Mag=");
  Serial.print(mag, DEC);

  Serial.println();

  delay(BNO055_SAMPLERATE_DELAY_MS);
}
