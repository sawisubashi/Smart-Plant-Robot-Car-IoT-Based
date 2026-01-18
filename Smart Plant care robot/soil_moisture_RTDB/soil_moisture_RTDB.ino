// ESP32 + Firebase + Soil Moisture Sensors
#include <WiFi.h>
#include <FirebaseESP32.h>

// ---------- WiFi ----------
#define WIFI_SSID "Adhi's iPhone (2)"
#define WIFI_PASSWORD "123456789"

// ---------- Firebase ----------
#define FIREBASE_HOST "soil-moisture-data-5fcea-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "fNp5yTefIKBzjXmf7O5sHFgJSuUL3AeMfNFx8gNG"   // Database secret

// Firebase object
FirebaseData firebaseData;

// ---------- Sensors ----------
#define SENSOR1_PIN 34
#define SENSOR2_PIN 35
#define SENSOR3_PIN 32
#define SENSOR4_PIN 33

// Threshold (calibrate per your sensor values)
#define MOISTURE_THRESHOLD 2700

void setup() {
  Serial.begin(115200);

  // Connect WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Connect Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
}

void loop() {
  int sensorValues[4];
  bool sensorStatus[4]; // true = Dry, false = Wet

  // Read sensor values
  sensorValues[0] = analogRead(SENSOR1_PIN);
  sensorValues[1] = analogRead(SENSOR2_PIN);
  sensorValues[2] = analogRead(SENSOR3_PIN);
  sensorValues[3] = analogRead(SENSOR4_PIN);

  for (int i = 0; i < 4; i++) {
    sensorStatus[i] = sensorValues[i] > MOISTURE_THRESHOLD;

    Serial.print("Sensor ");
    Serial.print(i + 1);
    Serial.print(" Value: ");
    Serial.print(sensorValues[i]);
    Serial.print(" -> ");
    Serial.println(sensorStatus[i] ? "Dry" : "Wet");

    // Firebase path
    String path = "/soil/sensor" + String(i + 1);

    // Update Firebase
    if (Firebase.setBool(firebaseData, path, sensorStatus[i])) {
      Serial.print("Firebase updated: ");
      Serial.println(path);
    } else {
      Serial.print("Firebase update failed: ");
      Serial.println(firebaseData.errorReason());
    }
  }

  delay(5000); // Update every 5 seconds
}
