#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include <FirebaseESP32.h>
#include <ESP32Servo.h>

// _____________________________________________________________________________________________

Adafruit_TCS34725 tcs = Adafruit_TCS34725(
  TCS34725_INTEGRATIONTIME_50MS,
  TCS34725_GAIN_4X
);

// _____________________________________________________________________________________________

// water level sensor pin
#define WATER_PIN 34

// water pump control pin
#define Pump 17

// _____________________________________________________________________________________________

// IR Sensors
#define IR_LEFT   32
#define IR_CENTER 33
#define IR_RIGHT  25

// L298N Motor Pins
#define IN1 26
#define IN2 27
#define IN3 14
#define IN4 12
#define ENA 13
#define ENB 15

// Define base speed and turn speed
const int baseSpeed = 80;    // Base speed for straight movement
const int turnSpeed = 120;   // Speed for turning

// _____________________________________________________________________________________________

// Ultrasonic sensor pins
#define TRIG_PIN 4
#define ECHO_PIN 5

// Servo pin
#define SERVO_PIN 18

Servo myServo;
int Set = 15;
int distance_L, distance_F, distance_R;

// _____________________________________________________________________________________________

// WIFI details
#define WIFI_SSID "Adhi's iPhone (2)"
#define WIFI_PASSWORD "123456789"

// Firebase details
#define DATABASE_SECRET "fNp5yTefIKBzjXmf7O5sHFgJSuUL3AeMfNFx8gNG"
#define DATABASE_URL "https://soil-moisture-data-5fcea-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// _____________________________________________________________________________________________
void setup()
{
// ============== serial monitor ==============
    Serial.begin(115200);
// _____________________________________________________________________________________________
// ============== line following configuration ==============
    pinMode(IR_LEFT, INPUT);
    pinMode(IR_CENTER, INPUT);
    pinMode(IR_RIGHT, INPUT);

    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    pinMode(ENA, OUTPUT);
    pinMode(ENB, OUTPUT);

// _____________________________________________________________________________________________
// ============== obstacle avoidence configuration ==============
    pinMode(ECHO_PIN, INPUT);
    pinMode(TRIG_PIN, OUTPUT);

    myServo.attach(SERVO_PIN);

    for (int angle = 70; angle <= 140; angle += 5)  myServo.write(angle);
    for (int angle = 140; angle >= 0; angle -= 5)  myServo.write(angle);
    for (int angle = 0; angle <= 70; angle += 5)  myServo.write(angle);
    distance_F = Ultrasonic_read();

// _____________________________________________________________________________________________
// ============== water pump configuration ==============
    pinMode(Pump, OUTPUT);

// _____________________________________________________________________________________________
// ============== WIFI and Firebase configuration ==============
    // Connect to WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) 
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.println("Connected to WiFi");

    // Firebase configuration
    config.database_url = DATABASE_URL;
    config.signer.tokens.legacy_token = DATABASE_SECRET;

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
    Serial.println("Connected to Firebase");

// _____________________________________________________________________________________________
// ============== color sensor configuration ==============
    Wire.begin(21, 22); // SDA, SCL pins for ESP32

    if (tcs.begin()) 
    {
        Serial.println("TCS34725 found");
    } 
    else 
    {
        Serial.println("No TCS34725 found ... check your connections");
        while (1);
    }

// _____________________________________________________________________________________________


}

// _____________________________________________________________________________________________
void loop()
{
// _____________________________________________________________________________________________
// ============== IR Sensors ==============
    // ============== Reading IR Sensors ==============
    int left   = digitalRead(IR_LEFT);
    int center = digitalRead(IR_CENTER);
    int right  = digitalRead(IR_RIGHT);

    // ============== Print IR Sensor Values ==============
    Serial.print("Left: ");
    Serial.print(left);
    Serial.print("  Center: ");
    Serial.print(center);
    Serial.print("  Right: ");
    Serial.println(right);
// _____________________________________________________________________________________________
// ============== Reading Color Sensor ==============
    uint16_t r, g, b, c;
    tcs.getRawData(&r, &g, &b, &c);

    float rf = (float)r / c * 255.0;
    float gf = (float)g / c * 255.0;
    float bf = (float)b / c * 255.0;

    // ============== Print colors ==============
    if (rf > 193.00 && gf < 42.00 && bf < 37.00) 
    {
        Serial.println("Red Color Detected");
    } 
    else if (gf > 111.00 && rf < 87.00 && bf < 52.00) 
    {
        Serial.println("Green Color Detected");
    } 
    else if (bf > 114.00 && bf < 116.00 && rf < 66.00 && rf > 64.00 && gf < 114.00 && gf > 112.00) 
    {
        Serial.println("Blue Color Detected");
    }
    else if (rf > 133.00 && gf > 83.00 && bf < 34.00) 
    {
        Serial.println("Yellow Color Detected");
    }
    else if (rf > 170.00 && gf > 55.00 && gf < 56.00 && bf < 32.00) 
    {
        Serial.println("Orange Color Detected");
    }
    else 
    {
        Serial.println("UNKNOWN color");
    }
// _____________________________________________________________________________________________
// ============== water level ==============
    // ============== Read water level sensor ==============
    int waterValue = analogRead(WATER_PIN);

    // ============== Convert to percentage ==============
    int waterPercent = map(waterValue, 0, 1023, 0, 100);

    // ============== Print water level ==============
    if (waterPercent <= 0)
    {
        Firebase.RTDB.setInt(&fbdo, "/waterLevel", 0);
        Serial.println ("0%");
    }
    else if (waterPercent > 5 && waterPercent <= 30)
    {
        Firebase.RTDB.setInt(&fbdo, "/waterLevel", 25);
        Serial.println ("25%");
    }
    else if (waterPercent > 30 && waterPercent <= 55)
    {
        Firebase.RTDB.setInt(&fbdo, "/waterLevel", 50);
        Serial.println ("50%");
    }
    else if (waterPercent > 55 && waterPercent <= 80)
    {
        Firebase.RTDB.setInt(&fbdo, "/waterLevel",75);
        Serial.println ("75%");
    }
    else
    {
        Firebase.RTDB.setInt(&fbdo, "/waterLevel", 100);
        Serial.println ("100%");
    }
// _____________________________________________________________________________________________
// ============== manual control ==============
    if (Firebase.RTDB.getString(&fbdo, "/robot/mode")) 
    {
        String mode = fbdo.stringData();

        if (mode == "manual") 
        {

        // Read command
        if (Firebase.RTDB.getString(&fbdo, "/robot/manual_control/command")) 
        {
            String cmd = fbdo.stringData();

            if (cmd == "forward") 
            {
                forward();
            }
            else if (cmd == "backward") 
            {
                backward();
            }
            else if (cmd == "left") 
            {
                turnLeft();
            }
            else if (cmd == "right") 
            {
                turnRight();
            }
            else 
            {
                stopMotor();
            }
            
        }

        // Pump control
        if (Firebase.RTDB.getString(&fbdo, "/robot/pump/state")) 
        {
            String pumpState = fbdo.stringData();

            if (pumpState == "on") 
            {
                digitalWrite(Pump, LOW);
            }
            else 
            {
                digitalWrite(Pump, HIGH);
            }
        }
        }
    }
// _____________________________________________________________________________________________
// ============== auto control ==============
    else
    {
        // ============== Check the water level ==============
        if (waterPercent <= 20)
        {
            // Go to water tank and refill
            if (bf > 114.00 && bf < 116.00 && rf < 66.00 && rf > 64.00 && gf < 114.00 && gf > 112.00) 
            {
                stopMotor(); delay(500);
                turnRight(); delay(500);
                if (left == HIGH && center == HIGH && right == HIGH)
                {
                    stopMotor();

                }
                else
                {
                    // Black line = HIGH, White surface = LOW
                    if (center == HIGH && left == LOW && right == LOW) {
                        forward();
                    }
                    else if (left == HIGH && center == LOW) {
                        turnL();
                    }
                    else if (right == HIGH && center == LOW) {
                        turnR();
                    }
                    else if (left == HIGH && center == HIGH && right == LOW) {
                        turnL();
                    }
                    else if (right == HIGH && center == HIGH && left == LOW) {
                        turnR();
                    }
                    else {
                        stopMotor();
                    }
                }
            }
            else
            {
                // Black line = HIGH, White surface = LOW
                    if (center == HIGH && left == LOW && right == LOW) {
                        forward();
                    }
                    else if (left == HIGH && center == LOW) {
                        turnL();
                    }
                    else if (right == HIGH && center == LOW) {
                        turnR();
                    }
                    else if (left == HIGH && center == HIGH && right == LOW) {
                        turnL();
                    }
                    else if (right == HIGH && center == HIGH && left == LOW) {
                        turnR();
                    }
                    else {
                        stopMotor();
                    }
            }
        }
        // ============== Check the plant 1 ==============
        else if (Firebase.RTDB.getBool(&fbdo, "/soil/sensor1"))
        {
            s1 = fbdo.boolData();
            if (s1 == true)
            {
                if (rf > 193.00 && gf < 42.00 && bf < 37.00) 
                {
                    turnLeft(); delay(500);

                    if(rf > 193.00 && gf < 42.00 && bf < 37.00)
                    {
                        stopMotor();
                        if(s1 == true)
                        {
                            // activate water pump
                            digitalWrite(Pump, LOW);
                        }
                        else
                        {
                            digitalWrite(Pump, HIGH);
                        }

                    }
                    else
                    {
                        // Black line = HIGH, White surface = LOW
                        if (center == HIGH && left == LOW && right == LOW) {
                            forward();
                        }
                        else if (left == HIGH && center == LOW) {
                            turnLeft();
                        }
                        else if (right == HIGH && center == LOW) {
                            turnRight();
                        }
                        else if (left == HIGH && center == HIGH && right == LOW) {
                            turnLeft();
                        }
                        else if (right == HIGH && center == HIGH && left == LOW) {
                            turnRight();
                        }
                        else {
                            stopMotor();
                        }
                    }

                }
                else
                {
                    // Black line = HIGH, White surface = LOW
                    if (center == HIGH && left == LOW && right == LOW) {
                        forward();
                    }
                    else if (left == HIGH && center == LOW) {
                        turnLeft();
                    }
                    else if (right == HIGH && center == LOW) {
                        turnRight();
                    }
                    else if (left == HIGH && center == HIGH && right == LOW) {
                        turnLeft();
                    }
                    else if (right == HIGH && center == HIGH && left == LOW) {
                        turnRight();
                    }
                    else {
                        stopMotor();
                    }
                }
            }
        }
        // ============== Check the plant 2 ==============
        else if(Firebase.RTDB.getBool(&fbdo, "/soil/sensor2"))
        {
            s2 = fbdo.boolData();
            if (s2 == true)
            {
                if (gf > 111.00 && rf < 87.00 && bf < 52.00) 
                {
                    turnLeft(); delay(500);

                    if(gf > 111.00 && rf < 87.00 && bf < 52.00)
                    {
                        stopMotor();
                        if(s2 == true)
                        {
                            // activate water pump
                            digitalWrite(Pump, LOW);
                        }
                        else
                        {
                            digitalWrite(Pump, HIGH);
                        }

                    }
                    else
                    {
                        // Black line = HIGH, White surface = LOW
                        if (center == HIGH && left == LOW && right == LOW) {
                            forward();
                        }
                        else if (left == HIGH && center == LOW) {
                            turnLeft();
                        }
                        else if (right == HIGH && center == LOW) {
                            turnRight();
                        }
                        else if (left == HIGH && center == HIGH && right == LOW) {
                            turnLeft();
                        }
                        else if (right == HIGH && center == HIGH && left == LOW) {
                            turnRight();
                        }
                        else {
                            stopMotor();
                        }
                    }

                }
                else
                {
                    // Black line = HIGH, White surface = LOW
                    if (center == HIGH && left == LOW && right == LOW) {
                        forward();
                    }
                    else if (left == HIGH && center == LOW) {
                        turnLeft();
                    }
                    else if (right == HIGH && center == LOW) {
                        turnRight();
                    }
                    else if (left == HIGH && center == HIGH && right == LOW) {
                        turnLeft();
                    }
                    else if (right == HIGH && center == HIGH && left == LOW) {
                        turnRight();
                    }
                    else {
                        stopMotor();
                    }
                }
            }
        }
        // ============== Check the plant 3 ==============
        else if(Firebase.RTDB.getBool(&fbdo, "/soil/sensor3"))
        {
            s3 = fbdo.boolData();
            if (s3 == true)
            {
                if (rf > 133.00 && gf > 83.00 && bf < 34.00)
                {
                    turnLeft(); delay(500);

                    if(rf > 133.00 && gf > 83.00 && bf < 34.00)
                    {
                        stopMotor();
                        if(s3 == true)
                        {
                            // activate water pump
                            digitalWrite(Pump, LOW);
                        }
                        else
                        {
                            digitalWrite(Pump, HIGH);
                        }

                    }
                    else
                    {
                        // Black line = HIGH, White surface = LOW
                        if (center == HIGH && left == LOW && right == LOW) {
                            forward();
                        }
                        else if (left == HIGH && center == LOW) {
                            turnLeft();
                        }
                        else if (right == HIGH && center == LOW) {
                            turnRight();
                        }
                        else if (left == HIGH && center == HIGH && right == LOW) {
                            turnLeft();
                        }
                        else if (right == HIGH && center == HIGH && left == LOW) {
                            turnRight();
                        }
                        else {
                            stopMotor();
                        }
                    }

                }
                else
                {
                    // Black line = HIGH, White surface = LOW
                    if (center == HIGH && left == LOW && right == LOW) {
                        forward();
                    }
                    else if (left == HIGH && center == LOW) {
                        turnLeft();
                    }
                    else if (right == HIGH && center == LOW) {
                        turnRight();
                    }
                    else if (left == HIGH && center == HIGH && right == LOW) {
                        turnLeft();
                    }
                    else if (right == HIGH && center == HIGH && left == LOW) {
                        turnRight();
                    }
                    else {
                        stopMotor();
                    }
                }
            }
        }
        // ============== Check the plant 4 ==============
        else if(Firebase.RTDB.getBool(&fbdo, "/soil/sensor4"))
        {
            s4 = fbdo.boolData();
            if (s4 == true)
            {
                if (rf > 170.00 && gf > 55.00 && gf < 56.00 && bf < 32.00) 
                {
                    turnLeft(); delay(500);

                    if(rf > 170.00 && gf > 55.00 && gf < 56.00 && bf < 32.00)
                    {
                        stopMotor();
                        if(s4 == true)
                        {
                            // activate water pump
                            digitalWrite(Pump, LOW);
                        }
                        else
                        {
                            digitalWrite(Pump, HIGH);
                        }

                    }
                    else
                    {
                        // Black line = HIGH, White surface = LOW
                        if (center == HIGH && left == LOW && right == LOW) {
                            forward();
                        }
                        else if (left == HIGH && center == LOW) {
                            turnLeft();
                        }
                        else if (right == HIGH && center == LOW) {
                            turnRight();
                        }
                        else if (left == HIGH && center == HIGH && right == LOW) {
                            turnLeft();
                        }
                        else if (right == HIGH && center == HIGH && left == LOW) {
                            turnRight();
                        }
                        else {
                            stopMotor();
                        }
                    }

                }
                else
                {
                    // Black line = HIGH, White surface = LOW
                    if (center == HIGH && left == LOW && right == LOW) {
                        forward();
                    }
                    else if (left == HIGH && center == LOW) {
                        turnLeft();
                    }
                    else if (right == HIGH && center == LOW) {
                        turnRight();
                    }
                    else if (left == HIGH && center == HIGH && right == LOW) {
                        turnLeft();
                    }
                    else if (right == HIGH && center == HIGH && left == LOW) {
                        turnRight();
                    }
                    else {
                        stopMotor();
                    }
                }
            }
        }
        // ============== all plant wet ==============
        else
        {
            stopMotor();
        }

    }


}

// _____________________________________________________________________________________________
// ============== obstacle avoid ==============
    void obstacle_avoid()
    {
        distance_F = Ultrasonic_read();
    Serial.print(" F="); Serial.println(distance_F);
    
    
    if ((right == 0) && (center == 1) && (left == 0)) {
        if (distance_F > Set) {
        forward();
        } else {
        Check_side();
        }
    }
    // If right sensor is black and left is white, turn right
    else if ((right == 1) && (center == 1) && (left == 0)) {
        turnRight();
    }
    else if ((right == 1) && (center == 0) && (left == 0)) {
        turnRight();
    }
    // If left sensor is black and right is white, turn left
    else if ((right == 0) && (center == 1) && (left == 1)) {
        turnLeft();
    }
    else if ((right == 0) && (center == 0) && (left == 1)) {
        turnLeft();
    } 
    else if ((right == 1) && (center == 1) && (left == 1)) {
        stopMotor();
    }
    }

    long Ultrasonic_read() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    long duration = pulseIn(ECHO_PIN, HIGH, 30000);
    long distance = duration * 0.034 / 2;
    return distance;
    }

    void compareDistance() {
    if (distance_L > distance_R) {
        turnL(); delay(300);
        forward(); delay(300);
        turnR(); delay(300);
        forward(); delay(300);
        turnR(); delay(300);
    } else {
        turnR(); delay(300);
        forward(); delay(300);
        turnL(); delay(300);
        forward(); delay(300);
        turnL(); delay(300);
    }
    }

    void Check_side() {
    stopMotor(); delay(100);
    myServo.write(140); delay(300);
    distance_R = Ultrasonic_read();
    Serial.print("D R="); Serial.println(distance_R);
    delay(100);
    myServo.write(0); delay(500);
    distance_L = Ultrasonic_read();
    Serial.print("D L="); Serial.println(distance_L);
    delay(100);
    myServo.write(70); delay(300);
    compareDistance();
    }

// _____________________________________________________________________________________________
// ============== Motor control functions ==============
    // Motor Functions
    void forward() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    }

    void turnLeft() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    }

    void turnRight() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    }

    void stopMotor() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    }

    void turnL() {
    analogWrite(ENA, turnSpeed);
    analogWrite(ENB, baseSpeed);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    }

    void turnR() {
    analogWrite(ENA, baseSpeed);
    analogWrite(ENB, turnSpeed);
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    }