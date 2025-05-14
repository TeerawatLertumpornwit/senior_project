#include <Arduino_LSM9DS1.h>

// ค่าขอบเขตสำหรับ normalize
#define ACC_MIN -4.0f
#define ACC_MAX 4.0f
#define GYRO_MIN -2000.0f
#define GYRO_MAX 2000.0f

// พอร์ต UART สำหรับการสื่อสารกับอีกบอร์ด
#define UART_TX 1  // กำหนดขาที่ใช้สำหรับ TX
#define UART_RX 0  // กำหนดขาที่ใช้สำหรับ RX
#define UART_BAUD 115200

float normalize(float value, float min, float max) {
  return 2.0f * (value - min) / (max - min) - 1.0f;
}

void setup() {
  // เริ่มต้น Serial สำหรับ Debug
  Serial.begin(115200);
  
  // เริ่มต้น UART สำหรับส่งข้อมูล
  Serial1.begin(UART_BAUD);
  
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }
}

void loop() {
  float ax, ay, az, gx, gy, gz;
  
  // อ่านค่า IMU
  if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
    IMU.readAcceleration(ax, ay, az);
    IMU.readGyroscope(gx, gy, gz);
    
    // Normalize ค่า
    float ax_norm = normalize(ax, ACC_MIN, ACC_MAX);
    float ay_norm = normalize(ay, ACC_MIN, ACC_MAX);
    float az_norm = normalize(az, ACC_MIN, ACC_MAX);
    float gx_norm = normalize(gx, GYRO_MIN, GYRO_MAX);
    float gy_norm = normalize(gy, GYRO_MIN, GYRO_MAX);
    float gz_norm = normalize(gz, GYRO_MIN, GYRO_MAX);
    
    // สร้างข้อความสำหรับส่ง
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "%.5f,%.5f,%.5f,%.5f,%.5f,%.5f\n",
             ax_norm, ay_norm, az_norm, gx_norm, gy_norm, gz_norm);
             
    // ส่งผ่าน UART
    Serial1.print(buffer);
    
    // แสดงบน Serial Monitor สำหรับ Debug
    Serial.print(buffer);
  }
  
  delay(125);
}