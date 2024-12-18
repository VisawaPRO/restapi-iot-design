#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Wi-Fi credentials
const char* ssid = "Jangnubburengnong";        // ชื่อ Wi-Fi
const char* password = "12345678";            // รหัสผ่าน Wi-Fi

// Configuration for API
const char* siteID = "KMb827eb3fe41f";        // Site ID
const int deviceID = 2;                       // Device ID
const char* BEARIOT_IP = "172.20.10.2";       // IP ของ BeaRiOT
const int BEARIOT_PORT = 3300;                // Port ของ BeaRiOT
String API_ENDPOINT = "http://" + String(BEARIOT_IP) + ":" + String(BEARIOT_PORT) + "/api/interfaces/update";

// LDR Pin
const int LDR_PIN = 4;  // ขา GPIO ที่อ่านค่าจาก LDR Module (Analog Output)

// Global variable for sensor value
int ldrValue = 0;       // เก็บค่า ADC ที่อ่านจากเซ็นเซอร์

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
String generatePayload(int value) {
  StaticJsonDocument<200> doc;

  // วันที่และเวลา (hard-coded)
  String isoDate = "2024-06-17T10:00:00Z";

  doc["siteID"] = siteID;
  doc["deviceID"] = deviceID;
  doc["date"] = isoDate;
  doc["offset"] = 0;
  doc["connection"] = "REST";

  JsonArray tagObj = doc.createNestedArray("tagObj");
  JsonObject obj = tagObj.createNestedObject();
  obj["status"] = true;
  obj["label"] = "ldr_sensor";   // ชื่อของเซ็นเซอร์
  obj["value"] = value;          // ใช้ค่า ADC ที่อ่านจาก LDR
  obj["record"] = true;
  obj["update"] = "All";

  String payload;
  serializeJson(doc, payload);
  return payload;
}

// Function to read LDR value
void readLDR() {
  ldrValue = analogRead(LDR_PIN);  // อ่านค่า ADC
  
  // แสดงค่า ADC ใน Serial Monitor
  Serial.print("LDR Value (Analog): ");
  Serial.println(ldrValue);
}

// Function to send HTTP POST request
void sendData(String payload) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(API_ENDPOINT);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(payload);
    if (httpResponseCode == 200) {
      Serial.println("Data sent successfully!");
    } else {
      Serial.print("Failed to send data. HTTP response code: ");
      Serial.println(httpResponseCode);
    }
    http.end(); // ปิดการเชื่อมต่อ HTTP
  } else {
    Serial.println("Error: Not connected to Wi-Fi");
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(LDR_PIN, INPUT); // ตั้งค่า LDR_PIN เป็น input
  connectWiFi();           // เชื่อมต่อ Wi-Fi

  // อ่านค่า LDR ครั้งแรกใน setup
  Serial.println("Initial LDR Reading...");
  readLDR();
}

void loop() {
  // อ่านค่า LDR และส่งข้อมูลใน loop
  readLDR();
  if (ldrValue >= 0 && ldrValue <= 4095) {    // ตรวจสอบค่าที่ได้ให้อยู่ในช่วงปกติ
    String payload = generatePayload(ldrValue); // สร้าง JSON payload
    sendData(payload);                        // ส่งข้อมูลไปยัง API
  } else {
    Serial.println("Error: Invalid LDR value");
  }

  delay(1000); // รอ 1 วินาทีก่อนวนลูปอีกครั้ง
}
