#include <Wire.h>
#include <sensor_fusion.h>
#include <printf.h>

#define PWR_MGMT_1 0x6B
#define GYRO_CONFIG 0x1B
#define CONFIG 0x1A
#define INT_STATUS 0x3A
#define ACCEL_XOUT_H 0x3B
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40
#define GYRO_XOUT_H 0x43
#define GYRO_XOUT_L 0x44
#define GYRO_YOUT_H 0x45
#define GYRO_YOUT_L 0x46
#define GYRO_ZOUT_H 0x47
#define GYRO_ZOUT_L 0x48

uint8_t buf;
uint8_t temp, temp0;
int16_t a_x, a_y, a_z;
int16_t g_x, g_y, g_z;
int16_t g;
int16_t avg_a_x = 0, avg_a_y = 0, avg_a_z = 0;
int32_t avg_g_x = 0, avg_g_y = 0, avg_g_z = 0;
int i = 0;
float angle, angle_of_rot, alpha = 0.3;
unsigned long currTime, prevTime;
long dt;
vector accel;
vector norm_accel;
vector gyro;
vector norm_gyro;
vector trueUp;
vector filter, filter1, filter2;
quaternion rotate;

void setup() {
  Serial.begin(115200);

  //Part B. Set up PWR, GYRO, CONFIG
  //Serial.println("PWR");
  readReg(PWR_MGMT_1, &buf, 1);
  buf = buf & B00000000;
  writeReg(PWR_MGMT_1, &buf, 1);
  //Serial.println(buf);

  //Serial.println("GYRO");
  readReg(GYRO_CONFIG, &buf, 1);
  buf = buf | B00011000;
  writeReg(GYRO_CONFIG, &buf, 1);
  //Serial.println(buf);

  //Serial.println("CONFIG");
  readReg(CONFIG, &buf, 1);
  buf = buf & B11111000;
  writeReg(CONFIG, &buf, 1);
  //Serial.println(buf);

  find_bias(100);
  trueUp.x = 0;
  trueUp.y = 0;
  trueUp.z = 1;
  prevTime = millis();
}

void loop() {
  readReg(INT_STATUS, &buf, 1);
  if ((buf & B00000001) == 1)
  {
    readAccel();
    readGyro();

    currTime = millis();
    dt = (currTime - prevTime);
    prevTime = currTime;

    a_x -= avg_a_x;
    a_y -= avg_a_y;
    a_z -= avg_a_z;

    accel.x = a_x;
    accel.y = a_y;
    accel.z = a_z;

    g_x -= avg_g_x;
    g_y -= avg_g_y;
    g_z -= avg_g_z;

    gyro.x = g_x;
    gyro.y = g_y;
    gyro.z = g_z;

    vector_normalize(&accel, &norm_accel);
    angle = vector_normalize(&gyro, &norm_gyro);
    //Serial.println(angle, 15);
    angle_of_rot = dt * (-1) * PI * angle / 180.0 / 16.4 / 1000.0;
    quaternion_create(&norm_gyro, angle_of_rot, &rotate);
    quaternion_rotate(&trueUp, &rotate, &trueUp);
    
    vector_multiply(&norm_accel, alpha, &filter1);
    vector_multiply(&trueUp, 1-alpha, &filter2);
    vector_add(&filter1, &filter2, &filter);
    vector_normalize(&filter, &filter);
    
    Serial.print(norm_accel.x);
    Serial.print(" ");
    Serial.print(norm_accel.y);
    Serial.print(" ");
    Serial.print(norm_accel.z);
    Serial.print(" ");
    Serial.print(trueUp.x);
    Serial.print(" ");
    Serial.print(trueUp.y);
    Serial.print(" ");
    Serial.print(trueUp.z);
    Serial.print(" ");
    Serial.print(filter.x);
    Serial.print(" ");
    Serial.print(filter.y);
    Serial.print(" ");
    Serial.println(filter.z);
  }
}

void find_bias(int i)
{
  while (1) {
    readReg(INT_STATUS, &buf, 1);
    if ((buf & B00000001) == 1)
    {
      readAccel();
      readGyro();

      avg_a_x += a_x;
      avg_a_y += a_y;
      avg_a_z += (a_z - 16384);

      avg_g_x += g_x;
      avg_g_y += g_y;
      avg_g_z += g_z;
      i--;
    }
    if (i <= 0)
    {
      avg_a_x /= 100;
      avg_a_y /= 100;
      avg_a_z /= 100;

      avg_g_x /= 100;
      avg_g_y /= 100;
      avg_g_z /= 100;
      break;
    }
  }
}
void readAccel()
{
  readReg(ACCEL_XOUT_H, &temp0, 1);
  readReg(ACCEL_XOUT_L, &temp, 1);
  a_x = temp0;
  a_x = a_x << 8 | temp;
  readReg(ACCEL_YOUT_H, &temp0, 1);
  readReg(ACCEL_YOUT_L, &temp, 1);
  a_y = temp0;
  a_y = a_y << 8 | temp;
  readReg(ACCEL_ZOUT_H, &temp0, 1);
  readReg(ACCEL_ZOUT_L, &temp, 1);
  a_z = temp0;
  a_z = a_z << 8 | temp;
}
void readGyro()
{
  readReg(GYRO_XOUT_H, &temp0, 1);
  readReg(GYRO_XOUT_L, &temp, 1);
  g_x = temp0;
  g_x = g_x << 8 | temp;
  readReg(GYRO_YOUT_H, &temp0, 1);
  readReg(GYRO_YOUT_L, &temp, 1);
  g_y = temp0;
  g_y = g_y << 8 | temp;
  readReg(GYRO_ZOUT_H, &temp0, 1);
  readReg(GYRO_ZOUT_L, &temp, 1);
  g_z = temp0;
  g_z = g_z << 8 | temp;
}

