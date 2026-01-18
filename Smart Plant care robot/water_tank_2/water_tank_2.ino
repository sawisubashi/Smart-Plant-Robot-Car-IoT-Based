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

// Ultrasonic pins
#define TRIG_PIN 33
#define ECHO_PIN 18

// Relay pin
#define RELAY_PIN 15
#define RELAY_ACTIVE_LOW true

// Thresholds
const int FIREBASE_LEVEL_THRESHOLD = 25;      // Firebase waterLevel < 25
const float SENSOR_DISTANCE_THRESHOLD = 10.0; // Ultrasonic confirm distance

bool pumpState = false;

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  setRelay(false);

  // Connect WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("Connected to Wi-Fi");

  // Connect Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
}

void loop() {
  if (Firebase.ready()) {
    // Read value from Firebase
    if (Firebase.RTDB.getInt(&firebaseData, "/waterLevel")) {
      if (firebaseData.dataType() == "int") {
        int waterLevel = firebaseData.intData();
        Serial.print("Firebase waterLevel = ");
        Serial.println(waterLevel);

        float dist = measureDistanceCM();
        Serial.print("Ultrasonic distance = ");
        Serial.println(dist);

        // ✅ Pump ON only if BOTH conditions are true
        if (waterLevel < FIREBASE_LEVEL_THRESHOLD && dist <= SENSOR_DISTANCE_THRESHOLD) {
          if (!pumpState) {
            setRelay(true);
            pumpState = true;
            Serial.println("Pump -> ON");
          }
        } else {
          if (pumpState) {
            setRelay(false);
            pumpState = false;
            Serial.println("Pump -> OFF");
          }
        }
      }
    } else {
      Serial.print("Firebase read failed: ");
      Serial.println(firebaseData.errorReason());
    }
  }

  delay(2000); // check every 2 seconds
}

float measureDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000UL);
  if (duration == 0) {
    return 1000.0; // timeout = no object
  }
  return (duration / 2.0) / 29.1;  // convert to cm
}

void setRelay(bool turnOn) {
  bool out;
  if (RELAY_ACTIVE_LOW) out = !turnOn;
  else out = turnOn;
  digitalWrite(RELAY_PIN, out ? HIGH : LOW);
}
