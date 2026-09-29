#include <Servo.h>
#include <stdlib.h>
#include <string.h>

// Unity Data Driven Actuators
// Receives newline-terminated CSV from Unity: x,y,z,light

const int SERVO_PIN = 9;
const int LED_PIN = 8;

const int SERIAL_BUFFER_SIZE = 96;
char serialBuffer[SERIAL_BUFFER_SIZE];
int serialBufferIndex = 0;

// Tune these to the expected Unity x range in your scene.
const float UNITY_X_MIN = -10.0f;
const float UNITY_X_MAX = 10.0f;

float posX = 0.0f;
float posY = 0.0f;
float posZ = 0.0f;
int lightState = 0;

Servo myServo;

int mapUnityXToServo(float xValue) {
  if (UNITY_X_MAX <= UNITY_X_MIN) {
    return 90;
  }

  float normalized = (xValue - UNITY_X_MIN) / (UNITY_X_MAX - UNITY_X_MIN);
  int servoAngle = (int)(normalized * 180.0f);
  return constrain(servoAngle, 0, 180);
}

bool parseUnityCsvLine(const char* line, float& x, float& y, float& z, int& light) {
  char buffer[SERIAL_BUFFER_SIZE];
  strncpy(buffer, line, sizeof(buffer) - 1);
  buffer[sizeof(buffer) - 1] = '\0';

  char* token = strtok(buffer, ",");
  if (token == NULL) return false;
  x = atof(token);

  token = strtok(NULL, ",");
  if (token == NULL) return false;
  y = atof(token);

  token = strtok(NULL, ",");
  if (token == NULL) return false;
  z = atof(token);

  token = strtok(NULL, ",");
  if (token == NULL) return false;
  light = atoi(token);

  return true;
}

void handleIncomingUnityData(const char* line) {
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
  int light = 0;

  if (!parseUnityCsvLine(line, x, y, z, light)) {
    return;
  }

  posX = x;
  posY = y;
  posZ = z;
  lightState = (light != 0) ? 1 : 0;
}

void processIncomingSerial() {
  while (Serial.available() > 0) {
    char incomingChar = (char)Serial.read();

    if (incomingChar == '\r') {
      continue;
    }

    if (incomingChar == '\n') {
      serialBuffer[serialBufferIndex] = '\0';

      if (serialBufferIndex > 0) {
        handleIncomingUnityData(serialBuffer);
      }

      serialBufferIndex = 0;
    } else if (serialBufferIndex < SERIAL_BUFFER_SIZE - 1) {
      serialBuffer[serialBufferIndex++] = incomingChar;
    } else {
      // If a line is too long, reset and wait for the next message.
      serialBufferIndex = 0;
    }
  }
}

void updateActuators() {
  int servoAngle = mapUnityXToServo(posX);
  myServo.write(servoAngle);
  digitalWrite(LED_PIN, lightState ? HIGH : LOW);
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  myServo.attach(SERVO_PIN);

  Serial.begin(115200);
  Serial.setTimeout(25);
  delay(1000);

  // Startup test sequence.
  digitalWrite(LED_PIN, HIGH);
  myServo.write(0);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  myServo.write(90);
}

void loop() {
  processIncomingSerial();
  updateActuators();
}
