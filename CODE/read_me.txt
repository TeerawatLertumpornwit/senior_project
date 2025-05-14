Readme File สำหรับอธิบายระบบ
	ระบบการวิเคราะห์การเคลื่อนไหวของมนุษย์ด้วยการเรียนรู้ของเครื่องบนอุปกรณ์ประมวลผลข้อมูล ใช้อุปกรณ์ ได้แก่ ไมโครคอนโทรเลอร์ Arduino Nano 33 BLE Sense, ไมโครคอนโทรเลอร์ Arduino Nano 33 IoT, แบตเตอรี่สำรองที่มีพอร์ตจ่ายไฟ USB อย่างน้อย 2 ช่อง, สายต่อจัมเปอร์ 3 เส้น, breadboard 170 holes 2 อัน, สายชาร์จ USB Micro B 2 เส้น และสามารถดาวน์โหลด์ไฟล์โปรแกรมได้จาก
https://drive.google.com/drive/folders/1Ec1rohDuirw-QXnumup4d0dW9QuCtK40?usp=sharing

1. ขั้นตอนการติดตั้งคลังโปรแกรม
1.	ติดตั้งคลังโปรแกรม Arduino ได้แก่ Arduino_LSM9DS1, WiFiNINA, SPI
โดยเลือกที่ Sketch > Include Library > Manage Libraries
2.	ติดตั้งคลังโปรแกรม Arduino .zip ที่ชื่อว่า ei-project_final_3-arduino-1.0.3.zip 
โดยเลือกที่ Sketch > Include Library > Add .ZIP Library
3.	ติดตั้งคลังโปรแกรม requests ในโปรแกรม Visual Studio Code
โดยใช้คำสั่ง pip install requests ใน Terminal

2. ขั้นตอนการใช้งานโปรแกรม
1.	ส่วนของการรวบรวมข้อมูล (Train)
-	นำไฟล์ชื่อว่า uart_ble_sense.ino ลงบอร์ด Arduino Nano 33 BLE Sense
-	นำไฟล์ชื่อว่า uart_iot.ino ลงบอร์ด Arduino Nano 33 IoT
-	แก้ไขไฟล์ arduino_secrets.h โดยกำหนด ssid และ password แล้วเชื่อมต่อ Network ที่กำหนดเข้ากับคอมพิวเตอร์ 
-	เปิดไฟล์ชื่อว่า csv_save_wifi_long_1.25.ipynb ใน Visual Studio Code เมื่อต้องการรวบรวมข้อมูลให้เลือกคำสั่ง Run All
-	หากต้องการข้อมูลแบบ Real Time สามารถเข้าถึงได้จาก http://192.168.4.1
2.	ส่วนของการทดสอบข้อมูล (Test)
-	นำไฟล์ชื่อว่า uart_ble_sense_test.ino ลงบอร์ด Arduino Nano 33 BLE Sense
-	นำไฟล์ชื่อว่า uart_iot_test.ino ลงบอร์ด Arduino Nano 33 IoT
-	แก้ไขไฟล์ arduino_secrets.h โดยกำหนด ssid และ password แล้วเชื่อมต่อ Network ที่กำหนดเข้ากับคอมพิวเตอร์ 
-	เปิดไฟล์ชื่อว่า csv_save_wifi_long_test_1.25.ipynb ใน Visual Studio Code เมื่อต้องการทดสอบข้อมูลให้เลือกคำสั่ง Run All
-	หากต้องการข้อมูลแบบ Real Time สามารถเข้าถึงได้จาก http://192.168.4.1
