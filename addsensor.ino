#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Wi-Fi credentials
const char* ssid = "Jangnubburengnong";   // ชื่อ Wi-Fi
const char* password = "12345678";        // รหัสผ่าน Wi-Fi

// Configuration for API
const char* siteID = "KMb827eb3fe41f";    // Site ID
const int deviceID = 2;                   // Device ID
const char* BEARIOT_IP = "172.20.10.2";   // IP ของ BeaRiOT
const int BEARIOT_PORT = 3300;            // Port ของ BeaRiOT
String API_ENDPOINT = "http://" + String(BEARIOT_IP) + ":" + String(BEARIOT_PORT) + "/api/interfaces/update";

// Function to connect to Wi-Fi
void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.println("Connecting to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// Function to generate JSON payload
String generatePayload(float value) {
  StaticJsonDocument<200> doc;

  // Date และเวลาแบบ mock (hard-coded)
  String isoDate = "2024-06-17T10:00:00Z";

  doc["siteID"] = siteID;
  doc["deviceID"] = deviceID;
  doc["date"] = isoDate;
  doc["offset"] = 0;
  doc["connection"] = "REST";

  JsonArray tagObj = doc.createNestedArray("tagObj");
  JsonObject obj = tagObj.createNestedObject();
  obj["status"] = true;
  obj["label"] = "ldr_sensor";   // เปลี่ยนเป็นชื่อของเซ็นเซอร์ LDR
  obj["value"] = value;          // ใช้ค่าจาก LDR
  obj["record"] = true;
  obj["update"] = "All";

  String payload;
  serializeJson(doc, payload);
  return payload;
}

// Function to read LDR value
float readLDR() {
  int analogValue = analogRead(25);  // อ่านค่าจากขา A0 (GPIO 25)
  float voltage = analogValue * (3.3 / 4095.0); // แปลงค่าจาก ADC เป็นแรงดัน (0-3.3V)
  Serial.print("LDR Voltage: ");
  Serial.println(voltage);
  return voltage; // ส่งค่าแรงดันกลับ
}

// Function to send HTTP POST request
void sendData(String payload) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(API_ENDPOINT);                     // ตั้งค่า URL API
    http.addHeader("Content-Type", "application/json");  // กำหนด header เป็น JSON

    int httpResponseCode = http.POST(payload);    // ส่งข้อมูลไปที่ server
    if (httpResponseCode == 200) {
      Serial.println("Data sent successfully!");
    } else {
      Serial.print("Failed to send data. HTTP response code: ");
      Serial.println(httpResponseCode);
    }
    http.end();  // ปิดการเชื่อมต่อ HTTP
  } else {
    Serial.println("Error: Not connected to Wi-Fi");
  }
}

void setup() {
  Serial.begin(9600);
  connectWiFi();  // เชื่อมต่อ Wi-Fi
  pinMode(25, INPUT);  // ตั้งขา 25 (A0) เป็น input สำหรับ LDR
}

void loop() {
  float ldrValue = readLDR();         // อ่านค่า LDR
  String payload = generatePayload(ldrValue); // สร้าง payload JSON
  sendData(payload);                            // ส่งข้อมูลไปยัง API

  delay(5000); // รอ 5 วินาทีก่อนส่งข้อมูลรอบถัดไป
}
