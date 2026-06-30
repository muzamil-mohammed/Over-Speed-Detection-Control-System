/*
  =====================================================================
  Over-Speed Detection & Control System
  ---------------------------------------------------------------------
  Description : Detects the speed of a moving object using two IR
                sensors placed a known distance apart, calculates
                speed in km/h, and automatically reduces DC motor
                speed (PWM) + triggers buzzer/LED alert if the
                measured speed exceeds a predefined limit.

  Hardware    : Arduino UNO/Nano, 2x IR Sensor Modules, L298N Motor
                Driver, DC Geared Motor, Buzzer, LED

  Author      : muzamil mohammed
  Repository  : https://github.com/muzamil-mohammed/-Over-Speed-Detection-Control-System
  License     : MIT
  =====================================================================
*/

// ---------------------- Pin Definitions ----------------------
const int IR_SENSOR_1_PIN = 2;   // First IR sensor (interrupt pin)
const int IR_SENSOR_2_PIN = 3;   // Second IR sensor (interrupt pin)
const int BUZZER_PIN      = 8;
const int LED_PIN         = 9;

const int MOTOR_IN1_PIN   = 5;   // L298N IN1
const int MOTOR_IN2_PIN   = 6;   // L298N IN2
const int MOTOR_ENA_PIN   = 10;  // L298N ENA (PWM speed control)

// ---------------------- Configuration --------------------------
const float SENSOR_DISTANCE = 0.20;   // Distance between IR sensors (meters)
const float SPEED_LIMIT     = 5.0;    // Speed limit threshold (km/h)
const int   NORMAL_PWM      = 255;    // Normal motor PWM (0-255)
const int   REDUCED_PWM     = 120;    // Reduced PWM when over-speeding
const unsigned long DEBOUNCE_MS = 50; // Sensor debounce time (ms)

// ---------------------- State Variables --------------------------
volatile unsigned long timeAtSensor1 = 0;
volatile unsigned long timeAtSensor2 = 0;
volatile bool sensor1Triggered = false;
volatile bool sensor2Triggered = false;

unsigned long lastSensor1Time = 0;
unsigned long lastSensor2Time = 0;

bool overSpeedState = false;
int currentPWM = NORMAL_PWM;

void setup() {
  Serial.begin(9600);

  pinMode(IR_SENSOR_1_PIN, INPUT);
  pinMode(IR_SENSOR_2_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(MOTOR_IN1_PIN, OUTPUT);
  pinMode(MOTOR_IN2_PIN, OUTPUT);
  pinMode(MOTOR_ENA_PIN, OUTPUT);

  // Attach interrupts for accurate timestamp capture
  attachInterrupt(digitalPinToInterrupt(IR_SENSOR_1_PIN), onSensor1Trigger, FALLING);
  attachInterrupt(digitalPinToInterrupt(IR_SENSOR_2_PIN), onSensor2Trigger, FALLING);

  // Start motor at normal speed, forward direction
  digitalWrite(MOTOR_IN1_PIN, HIGH);
  digitalWrite(MOTOR_IN2_PIN, LOW);
  setMotorSpeed(NORMAL_PWM);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  Serial.println(F("=== Over-Speed Detection & Control System ==="));
  Serial.print(F("Speed Limit: "));
  Serial.print(SPEED_LIMIT);
  Serial.println(F(" km/h"));
  Serial.println(F("System Initialized. Waiting for object..."));
}

void loop() {
  // When both sensors have been triggered in sequence, compute speed
  if (sensor1Triggered && sensor2Triggered) {
    noInterrupts();
    unsigned long t1 = timeAtSensor1;
    unsigned long t2 = timeAtSensor2;
    sensor1Triggered = false;
    sensor2Triggered = false;
    interrupts();

    if (t2 > t1) {
      float deltaTimeSec = (t2 - t1) / 1000.0;
      float speedMps = SENSOR_DISTANCE / deltaTimeSec;
      float speedKmph = speedMps * 3.6;

      Serial.print(F("Speed Detected: "));
      Serial.print(speedKmph);
      Serial.println(F(" km/h"));

      evaluateSpeed(speedKmph);
    }
  }
}

// ---------------------- Interrupt Service Routines --------------------------
void onSensor1Trigger() {
  unsigned long now = millis();
  if (now - lastSensor1Time > DEBOUNCE_MS) {
    timeAtSensor1 = now;
    sensor1Triggered = true;
    lastSensor1Time = now;
  }
}

void onSensor2Trigger() {
  unsigned long now = millis();
  if (now - lastSensor2Time > DEBOUNCE_MS) {
    timeAtSensor2 = now;
    sensor2Triggered = true;
    lastSensor2Time = now;
  }
}

// ---------------------- Core Logic --------------------------
void evaluateSpeed(float speedKmph) {
  if (speedKmph > SPEED_LIMIT) {
    if (!overSpeedState) {
      Serial.println(F(">>> OVER SPEED! Reducing motor speed and alerting <<<"));
    }
    overSpeedState = true;
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    setMotorSpeed(REDUCED_PWM);
  } else {
    if (overSpeedState) {
      Serial.println(F("Speed within limit. Resuming normal operation."));
    }
    overSpeedState = false;
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    setMotorSpeed(NORMAL_PWM);
  }
}

void setMotorSpeed(int pwmValue) {
  currentPWM = constrain(pwmValue, 0, 255);
  analogWrite(MOTOR_ENA_PIN, currentPWM);
}
