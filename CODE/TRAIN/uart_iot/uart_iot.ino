#include <SPI.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

int led = LED_BUILTIN;
int status = WL_IDLE_STATUS;
WiFiServer server(80);

// เปลี่ยนตัวแปรเป็นค่าแกน accelerometer และ gyroscope
float ax = 0.0, ay = 0.0, az = 0.0, gx = 0.0, gy = 0.0, gz = 0.0;
// ตัวแปรเก็บค่าสตริงที่ฟอร์แมต (เพิ่มขนาดเป็น 12 เพื่อรองรับทศนิยม 5 ตำแหน่ง)
char axStr[12], ayStr[12], azStr[12], gxStr[12], gyStr[12], gzStr[12];

void setup() {
    pinMode(led, OUTPUT);
    Serial.begin(115200);
    Serial1.begin(115200);

    // เริ่มต้นค่าสตริงเป็น "0.00000" (ทศนิยม 5 ตำแหน่ง)
    snprintf(axStr, sizeof(axStr), "%.5f", ax);
    snprintf(ayStr, sizeof(ayStr), "%.5f", ay);
    snprintf(azStr, sizeof(azStr), "%.5f", az);
    snprintf(gxStr, sizeof(gxStr), "%.5f", gx);
    snprintf(gyStr, sizeof(gyStr), "%.5f", gy);
    snprintf(gzStr, sizeof(gzStr), "%.5f", gz);

    if (WiFi.status() == WL_NO_MODULE) {
        while (true) {
            digitalWrite(led, HIGH);
            delay(500);
            digitalWrite(led, LOW);
            delay(500);
        }
    }

    status = WiFi.beginAP(ssid, pass);
    if (status != WL_AP_LISTENING) {
        while (true) {
            digitalWrite(led, HIGH);
            delay(100);
            digitalWrite(led, LOW);
            delay(100);
        }
    }

    delay(10000);
    server.begin();
}

void readUartData() {
    if (Serial1.available()) {
        String completeData = "";
        
        // อ่านข้อมูลจาก Serial1
        while (Serial1.available() > 0) {
            char inChar = (char)Serial1.read();
            completeData += inChar;
            delay(2);
        }

        // ตรวจสอบข้อมูลที่ได้รับ
        if (completeData.length() > 0) {
            Serial.println("Received: " + completeData);
            completeData.trim();

            int valueCount = 0;
            String values[6]; // เปลี่ยนเป็น 6 ค่า

            // ใช้ strtok แยกข้อมูล
            char* ptr = strtok((char*)completeData.c_str(), ",");
            while (ptr != NULL && valueCount < 6) { // เปลี่ยนเป็น 6 ค่า
                values[valueCount] = String(ptr);
                valueCount++;
                ptr = strtok(NULL, ",");
            }

            // ตรวจสอบค่าที่ได้รับครบ 6 ค่า
            if (valueCount == 6) { // เปลี่ยนเป็น 6 ค่า
                ax = values[0].toFloat();
                ay = values[1].toFloat();
                az = values[2].toFloat();
                gx = values[3].toFloat();
                gy = values[4].toFloat();
                gz = values[5].toFloat();

                // แปลงค่าทศนิยม 5 ตำแหน่งและเก็บในตัวแปรสตริง
                snprintf(axStr, sizeof(axStr), "%.5f", ax);
                snprintf(ayStr, sizeof(ayStr), "%.5f", ay);
                snprintf(azStr, sizeof(azStr), "%.5f", az);
                snprintf(gxStr, sizeof(gxStr), "%.5f", gx);
                snprintf(gyStr, sizeof(gyStr), "%.5f", gy);
                snprintf(gzStr, sizeof(gzStr), "%.5f", gz);

                Serial.print("Parsed: ");
                Serial.print(axStr);
                Serial.print(",");
                Serial.print(ayStr);
                Serial.print(",");
                Serial.print(azStr);
                Serial.print(",");
                Serial.print(gxStr);
                Serial.print(",");
                Serial.print(gyStr);
                Serial.print(",");
                Serial.println(gzStr);
            } else {
                Serial.println("Error: Data format incorrect!");
            }
        }

        // ล้าง buffer
        Serial1.flush();
    }
}

void loop() {
    WiFiClient client = server.available();
    readUartData();

    if (client) {
        String currentLine = "";
        bool isDataRequest = false;

        while (client.connected()) {
            delayMicroseconds(10);

            if (client.available()) {
                char c = client.read();
                if (c == '\n') {
                    if (currentLine.length() == 0) {
                        if (isDataRequest) {
                            client.println("HTTP/1.1 200 OK");
                            client.println("Content-Type: application/json");
                            client.println("Connection: close");
                            client.println();
                            // ส่งข้อมูล JSON โดยใช้ค่าที่มีทศนิยม 5 ตำแหน่ง
                            char jsonBuffer[250]; // เพิ่มขนาด buffer เนื่องจากทศนิยมเพิ่มขึ้น
                            snprintf(jsonBuffer, sizeof(jsonBuffer), 
                                 "{\"ax\":\"%s\",\"ay\":\"%s\",\"az\":\"%s\",\"gx\":\"%s\",\"gy\":\"%s\",\"gz\":\"%s\"}", 
                                 axStr, ayStr, azStr, gxStr, gyStr, gzStr);
                            client.println(jsonBuffer);
                            break;
                        }

                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-Type: text/html");
                        client.println("Connection: close");
                        client.println();

                        client.println("<html><head><title>IMU Data</title>");
                        client.println("<meta charset=\"UTF-8\">");
                        client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
                        client.println("<style>");
                        client.println("body { font-family: Arial, sans-serif; margin: 20px; }");
                        client.println("h1 { color: #333; }");
                        client.println(".status { margin: 10px 0; padding: 10px; border-radius: 5px; background-color: #f0f0f0; }");
                        client.println(".value { font-weight: bold; font-size: 1.2em; }");
                        client.println("</style>");
                        client.println("<script>");
                        client.println("async function fetchData() {");
                        client.println("  try {");
                        client.println("    const response = await fetch('/data');");
                        client.println("    const data = await response.json();");
                        client.println("    document.getElementById('ax').innerText = data.ax;");
                        client.println("    document.getElementById('ay').innerText = data.ay;");
                        client.println("    document.getElementById('az').innerText = data.az;");
                        client.println("    document.getElementById('gx').innerText = data.gx;");
                        client.println("    document.getElementById('gy').innerText = data.gy;");
                        client.println("    document.getElementById('gz').innerText = data.gz;");
                        client.println("  } catch (error) {");
                        client.println("    console.error('Error fetching data:', error);");
                        client.println("  }");
                        client.println("}"); 
                        client.println("setInterval(fetchData, 100);");
                        client.println("</script></head><body>");
                        client.println("<h1>IMU Data</h1>");
                        client.println("<h2>Accelerometer</h2>");
                        client.println("<div class=\"status\">X: <span id='ax' class=\"value\">0.00000</span></div>");
                        client.println("<div class=\"status\">Y: <span id='ay' class=\"value\">0.00000</span></div>");
                        client.println("<div class=\"status\">Z: <span id='az' class=\"value\">0.00000</span></div>");
                        client.println("<h2>Gyroscope</h2>");
                        client.println("<div class=\"status\">X: <span id='gx' class=\"value\">0.00000</span></div>");
                        client.println("<div class=\"status\">Y: <span id='gy' class=\"value\">0.00000</span></div>");
                        client.println("<div class=\"status\">Z: <span id='gz' class=\"value\">0.00000</span></div>");
                        client.println("</body></html>");
                        break;
                    } else {
                        currentLine = "";
                    }
                } else if (c != '\r') {
                    currentLine += c;
                }
                if (currentLine.endsWith("GET /data")) {
                    isDataRequest = true;
                }
            }
        }
        client.stop();
    }
}