/****************************************************
 * SMART HOME + AUTOMATIC CLOTHESLINE
 * ESP32 + Blynk + Rain + PIR + Light + Servo
 ****************************************************/

#define BLYNK_PRINT Serial

// ================================================== 
// BLYNK CONFIG
// ==================================================

#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "TMPL6pYsjtDEb"
#define BLYNK_TEMPLATE_NAME "Smart Home"
#define BLYNK_AUTH_TOKEN "Xp8NsrHPweH8RrEFnrkG4G0LVt_7bfRG"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>

// ===============================
// WiFi
// ===============================
char ssid[] = "POCO F6";
char pass[] = "00000000";

// ===============================
// กำหนดขาอุปกรณ์
// ===============================
#define RAIN_SENSOR   13
#define PIR_SENSOR    14
#define LED_PIN       12
#define BUZZER_PIN    26
#define LIGHT_SENSOR  27
#define SERVO_PIN     25

// ===============================
// Servo
// ===============================
Servo clothesServo;

#define SERVO_OPEN   90
#define SERVO_CLOSE  0

// ===============================
// Blynk Timer
// ===============================
BlynkTimer timer;

// ===============================
// ตัวแปรโหมด
//
// true  = AUTO
// false = MANUAL
// ===============================
bool clotheslineAuto = true;
bool buzzerAuto = true;
bool lightAuto = true;

// ===============================
// ค่าควบคุม MANUAL
// ===============================
int clotheslineManual = 0;
int buzzerManual = 0;
int ledManual = 0;

// ===============================
// สถานะอุปกรณ์
// ===============================
int servoStatus = 0;
int buzzerStatus = 0;
int ledStatus = 0;

// ===============================
// ค่าสถานะ Sensor
// ===============================
int rainValue = HIGH;
int pirValue = LOW;
int lightValue = LOW;


// =====================================================
// อ่าน Sensor
// =====================================================
void readSensors() {

  rainValue = digitalRead(RAIN_SENSOR);
  pirValue = digitalRead(PIR_SENSOR);
  lightValue = digitalRead(LIGHT_SENSOR);
}


// =====================================================
// ควบคุม Servo / ราวตากผ้า
// =====================================================
void controlClothesline() {

  // ถ้าเป็น MANUAL
  if (!clotheslineAuto) {

    if (clotheslineManual == 1) {

      // เก็บราว
      clothesServo.write(SERVO_CLOSE);
      servoStatus = 1;

    } else {

      // กางราว
      clothesServo.write(SERVO_OPEN);
      servoStatus = 0;
    }

    return;
  }


  // ==========================================
  // AUTO
  // ==========================================

  // ฝนตก = LOW
  if (rainValue == LOW) {

    clothesServo.write(SERVO_CLOSE);
    servoStatus = 1;

  } else {

    clothesServo.write(SERVO_OPEN);
    servoStatus = 0;
  }
}


// =====================================================
// ควบคุม Buzzer
// =====================================================
void controlBuzzer() {

  // ==========================================
  // AUTO
  // ==========================================
  if (buzzerAuto) {

    if (pirValue == HIGH) {

      digitalWrite(BUZZER_PIN, HIGH);
      buzzerStatus = 1;

    } else {

      digitalWrite(BUZZER_PIN, LOW);
      buzzerStatus = 0;
    }

  }

  // ==========================================
  // MANUAL
  // ==========================================
  else {

    if (buzzerManual == 1) {

      digitalWrite(BUZZER_PIN, HIGH);
      buzzerStatus = 1;

    } else {

      digitalWrite(BUZZER_PIN, LOW);
      buzzerStatus = 0;
    }
  }
}


// =====================================================
// ควบคุม LED
// =====================================================
void controlLED() {

  // ==========================================
  // AUTO
  // ==========================================
  if (lightAuto) {

    // จากการทดสอบของคุณ
    // HIGH = มืด
    if (lightValue == HIGH) {

      digitalWrite(LED_PIN, HIGH);
      ledStatus = 1;

    } else {

      digitalWrite(LED_PIN, LOW);
      ledStatus = 0;
    }
  }

  // ==========================================
  // MANUAL
  // ==========================================
  else {

    if (ledManual == 1) {

      digitalWrite(LED_PIN, HIGH);
      ledStatus = 1;

    } else {

      digitalWrite(LED_PIN, LOW);
      ledStatus = 0;
    }
  }
}


// =====================================================
// ส่งข้อมูล Sensor และสถานะไป Blynk
// =====================================================
void sendBlynkData() {

  readSensors();

  // -------------------------------
  // PIR
  // V4
  // -------------------------------
  if (pirValue == HIGH) {
    Blynk.virtualWrite(V4, 1);
  } else {
    Blynk.virtualWrite(V4, 0);
  }


  // -------------------------------
  // Rain
  // V7
  // LOW = ฝนตก
  // -------------------------------
  if (rainValue == LOW) {
    Blynk.virtualWrite(V7, 1);
  } else {
    Blynk.virtualWrite(V7, 0);
  }


  // -------------------------------
  // Light
  // V8
  // HIGH = มืด
  // -------------------------------
  if (lightValue == HIGH) {
    Blynk.virtualWrite(V8, 1);
  } else {
    Blynk.virtualWrite(V8, 0);
  }


  // -------------------------------
  // Servo Status
  // V9
  // -------------------------------
  Blynk.virtualWrite(V9, servoStatus);


  // -------------------------------
  // LED Status
  // V10
  // -------------------------------
  Blynk.virtualWrite(V10, ledStatus);


  // -------------------------------
  // Buzzer Status
  // V11
  // -------------------------------
  Blynk.virtualWrite(V11, buzzerStatus);
}


// =====================================================
// อัปเดตระบบทั้งหมด
// =====================================================
void updateSystem() {

  readSensors();

  controlClothesline();
  controlBuzzer();
  controlLED();

  sendBlynkData();
}


// =====================================================
// V0 = Clothesline Mode
//
// ON  = AUTO
// OFF = MANUAL
// =====================================================
BLYNK_WRITE(V0) {

  int value = param.asInt();

  if (value == 1) {

    // ON = AUTO
    clotheslineAuto = true;

  } else {

    // OFF = MANUAL
    clotheslineAuto = false;
  }

  controlClothesline();

  Blynk.virtualWrite(V9, servoStatus);
}


// =====================================================
// V1 = Clothesline Control
//
// ON  = เก็บ
// OFF = กาง
// =====================================================
BLYNK_WRITE(V1) {

  clotheslineManual = param.asInt();

  // ทำงานเฉพาะ MANUAL
  if (!clotheslineAuto) {

    controlClothesline();

    Blynk.virtualWrite(V9, servoStatus);
  }
}


// =====================================================
// V2 = Buzzer Mode
//
// ON  = AUTO
// OFF = MANUAL
// =====================================================
BLYNK_WRITE(V2) {

  int value = param.asInt();

  if (value == 1) {

    // ON = AUTO
    buzzerAuto = true;

  } else {

    // OFF = MANUAL
    buzzerAuto = false;
  }

  controlBuzzer();

  Blynk.virtualWrite(V11, buzzerStatus);
}


// =====================================================
// V3 = Buzzer Control
//
// ON  = เปิด
// OFF = ปิด
// =====================================================
BLYNK_WRITE(V3) {

  buzzerManual = param.asInt();

  // ทำงานเฉพาะ MANUAL
  if (!buzzerAuto) {

    controlBuzzer();

    Blynk.virtualWrite(V11, buzzerStatus);
  }
}


// =====================================================
// V5 = Light Mode
//
// ON  = AUTO
// OFF = MANUAL
// =====================================================
BLYNK_WRITE(V5) {

  int value = param.asInt();

  if (value == 1) {

    // ON = AUTO
    lightAuto = true;

  } else {

    // OFF = MANUAL
    lightAuto = false;
  }

  controlLED();

  Blynk.virtualWrite(V10, ledStatus);
}


// =====================================================
// V6 = LED Control
//
// ON  = เปิด
// OFF = ปิด
// =====================================================
BLYNK_WRITE(V6) {

  ledManual = param.asInt();

  // ทำงานเฉพาะ MANUAL
  if (!lightAuto) {

    controlLED();

    Blynk.virtualWrite(V10, ledStatus);
  }
}


// =====================================================
// เมื่อเชื่อมต่อ Blynk
// =====================================================
BLYNK_CONNECTED() {

  // ดึงค่าจาก Blynk
  Blynk.syncVirtual(V0);
  Blynk.syncVirtual(V1);

  Blynk.syncVirtual(V2);
  Blynk.syncVirtual(V3);

  Blynk.syncVirtual(V5);
  Blynk.syncVirtual(V6);
}


// =====================================================
// Setup
// =====================================================
void setup() {

  Serial.begin(115200);


  // -------------------------------
  // Sensor
  // -------------------------------
  pinMode(RAIN_SENSOR, INPUT);
  pinMode(PIR_SENSOR, INPUT);
  pinMode(LIGHT_SENSOR, INPUT);


  // -------------------------------
  // Output
  // -------------------------------
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);


  // ปิด LED และ Buzzer ตอนเริ่มต้น
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);


  // -------------------------------
  // Servo
  // -------------------------------
  clothesServo.attach(SERVO_PIN);

  // เริ่มต้น = กางราว
  clothesServo.write(SERVO_OPEN);

  servoStatus = 0;


  // -------------------------------
  // อ่าน Sensor ครั้งแรก
  // -------------------------------
  readSensors();


  // -------------------------------
  // เชื่อมต่อ Blynk
  // -------------------------------
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);


  // -------------------------------
  // Timer
  // -------------------------------
  timer.setInterval(500L, updateSystem);
}


// =====================================================
// Loop
// =====================================================
void loop() {

  Blynk.run();

  timer.run();
}