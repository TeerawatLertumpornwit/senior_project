#include <SPI.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

int led = LED_BUILTIN;
int status = WL_IDLE_STATUS;
WiFiServer server(80);

// ตัวแปรเก็บค่า idle, move, risk แบบทศนิยม 5 ตำแหน่ง
float idle = 0.0, move = 0.0, risk = 0.0;
// ตัวแปรเก็บค่าสตริงที่ฟอร์แมต
char idleStr[12], moveStr[12], riskStr[12];
// ตัวแปรเก็บ class ที่ predict ได้
String predictedClass = "Unknown";

void setup() {
    pinMode(led, OUTPUT);
    Serial.begin(115200);
    Serial1.begin(115200);

    // เริ่มต้นค่าสตริงเป็น "0.00000"
    snprintf(idleStr, sizeof(idleStr), "%.5f", idle);
    snprintf(moveStr, sizeof(moveStr), "%.5f", move);
    snprintf(riskStr, sizeof(riskStr), "%.5f", risk);

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
            String values[4]; // เพิ่มเป็น 4 values เพื่อรับ class ด้วย

            // ใช้ strtok แยกข้อมูล
            char* ptr = strtok((char*)completeData.c_str(), ",");
            while (ptr != NULL && valueCount < 4) {
                values[valueCount] = String(ptr);
                valueCount++;
                ptr = strtok(NULL, ",");
            }

            // ตรวจสอบค่าที่ได้รับครบ 4 ค่า (idle, move, risk, class)
            if (valueCount >= 3) {
                idle = values[0].toFloat();
                move = values[1].toFloat();
                risk = values[2].toFloat();

                // แปลงค่าทศนิยม 5 ตำแหน่งและเก็บในตัวแปรสตริง
                snprintf(idleStr, sizeof(idleStr), "%.5f", idle);
                snprintf(moveStr, sizeof(moveStr), "%.5f", move);
                snprintf(riskStr, sizeof(riskStr), "%.5f", risk);
                
                // รับค่า class โดยตรงจากข้อมูลที่ส่งมา (ถ้ามี)
                if (valueCount >= 4) {
                    predictedClass = values[3];
                } else {
                    // ถ้าไม่มีข้อมูล class ให้ใช้ค่า "Unknown"
                    predictedClass = "Unknown";
                }

                Serial.print("Parsed: ");
                Serial.print(idleStr);
                Serial.print(",");
                Serial.print(moveStr);
                Serial.print(",");
                Serial.print(riskStr);
                Serial.print(" | Class: ");
                Serial.println(predictedClass);
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

                            // ส่งข้อมูล JSON โดยใช้ค่า idleStr, moveStr, riskStr และ predictedClass
                            char jsonBuffer[150];
                            snprintf(jsonBuffer, sizeof(jsonBuffer), 
                                 "{\"idle\":\"%s\",\"move\":\"%s\",\"risk\":\"%s\",\"class\":\"%s\"}", 
                                 idleStr, moveStr, riskStr, predictedClass.c_str());
                            client.println(jsonBuffer);
                            break;
                        }

                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-Type: text/html");
                        client.println("Connection: close");
                        client.println();

                        client.println("<html><head><title>Motion Status</title>");
                        client.println("<meta charset=\"UTF-8\">");
                        client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
                        client.println("<style>");
                        client.println("body { font-family: Arial, sans-serif; margin: 20px; }");
                        client.println("h1 { color: #333; }");
                        client.println(".status { margin: 10px 0; padding: 10px; border-radius: 5px; background-color: #f0f0f0; }");
                        client.println(".value { font-weight: bold; font-size: 1.2em; }");
                        client.println(".class-indicator { margin-top: 20px; padding: 15px; border-radius: 8px; font-size: 1.5em; text-align: center; font-weight: bold; }");
                        client.println(".Idle { background-color: #b3e6cc; color: #2e7d52; }");
                        client.println(".Moving { background-color: #ffcc80; color: #e65100; }");
                        client.println(".Risk { background-color: #ff8a80; color: #c62828; }");
                        client.println(".Unknown { background-color: #e0e0e0; color: #616161; }");
                        client.println("</style>");
                        client.println("<script>");
                        client.println("async function fetchData() {");
                        client.println("  try {");
                        client.println("    const response = await fetch('/data');");
                        client.println("    const data = await response.json();");
                        client.println("    document.getElementById('idle').innerText = data.idle;");
                        client.println("    document.getElementById('move').innerText = data.move;");
                        client.println("    document.getElementById('risk').innerText = data.risk;");
                        client.println("    const classElement = document.getElementById('predictedClass');");
                        client.println("    classElement.innerText = data.class;");
                        client.println("    classElement.className = 'class-indicator ' + data.class;");
                        client.println("  } catch (error) {");
                        client.println("    console.error('Error fetching data:', error);");
                        client.println("  }");
                        client.println("}"); 
                        client.println("setInterval(fetchData, 1000);");
                        client.println("</script></head><body>");
                        client.println("<h1>Motion Status</h1>");
                        client.println("<div class=\"status\">Idle: <span id='idle' class=\"value\">0.00000</span></div>");
                        client.println("<div class=\"status\">Move: <span id='move' class=\"value\">0.00000</span></div>");
                        client.println("<div class=\"status\">Risk: <span id='risk' class=\"value\">0.00000</span></div>");
                        client.println("<div id='predictedClass' class=\"class-indicator Unknown\">Unknown</div>");
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