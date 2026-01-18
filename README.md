🌱 Smart Plant Robot Car (IoT Based)
-

An IoT-enabled autonomous smart plant care robot designed to monitor soil moisture, refill its water tank, navigate using line-following, avoid obstacles, and water plants automatically. The system also supports manual remote control using Firebase Realtime Database.

---

🚀 Project Overview

The Smart Plant Robot Car is built using ESP32 and multiple sensors to automate plant watering and navigation.
It operates in two modes:

Auto Mode – Fully autonomous plant care

Manual Mode – User-controlled via Firebase

This project is ideal for smart agriculture, home gardens, and greenhouse automation.

---

🧠 System Features

🌿 Soil Moisture Monitoring – Detects dry plants using soil sensors

💧 Automatic Watering System – Activates water pump only when needed

🚰 Water Level Monitoring – Refills water tank when level is low

🛣️ Line Following Navigation – Uses IR sensors for guided movement

🚧 Obstacle Avoidance – Ultrasonic sensor + servo scanning

🎨 Color Detection – Identifies plant locations and water refill point

☁️ IoT Integration – Real-time data & control using Firebase

🎮 Manual Control Mode – Remote movement & pump control

---

⚙️ Hardware Components

ESP32 Development Board

Soil Moisture Sensors (4×)

TCS34725 Color Sensor

IR Line Sensors (Left, Center, Right)

Ultrasonic Sensor (HC-SR04)

Servo Motor

Water Level Sensor

Water Pump

L298N Motor Driver

DC Motors & Robot Chassis

---

🛠️ Software & Technologies

Arduino IDE

C / C++ (Arduino Programming)

Firebase Realtime Database

WiFi Communication (ESP32)

IoT Architecture

---

🔄 Working Principle

🔹 Auto Mode

Checks water tank level

Refills water if level is low

Reads soil moisture data from Firebase

Navigates to dry plants using line following

Confirms plant using color detection

Waters plant automatically

Avoids obstacles during movement

🔹 Manual Mode

User sends commands via Firebase:

Forward / Backward / Left / Right / Stop

Water pump ON / OFF

---

☁️ Firebase Data Structure

/soil/sensor1..4 → Soil moisture status

/waterLevel → Water tank level

/robot/mode → Manual / Auto

/robot/manual_control/command → Movement control

/robot/pump/state → Pump ON / OFF

---

📌 Project Applications

Smart Gardening

Agricultural Automation

Greenhouse Systems

Educational Robotics Projects

IoT-Based Smart Systems

---

🔮 Future Improvements

Mobile app with live dashboard

Camera-based plant detection

Solar-powered charging

AI-based plant health analysis

Multiple robot coordination
