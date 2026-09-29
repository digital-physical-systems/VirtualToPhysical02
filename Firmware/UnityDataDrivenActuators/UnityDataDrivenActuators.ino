#include <Servo.h>
#include <stdio.h>

// Unity Data Driven Actuators
// Receives newline-terminated command from Unity: CTRL,<servoAngle>,<lightState>
// Example: CTRL,120,1
// Also accepts compatibility formats: <servoAngle>,<lightState> and x,y,z,light

const int SERVO_PIN = 9;
const int LED_PIN = 8;
const bool RUN_STARTUP_SELF_TEST = true;

const int SERIAL_BUFFER_SIZE = 32;
char serialBuffer[SERIAL_BUFFER_SIZE];
int serialBufferIndex = 0;

int servoAngle = 90;
int lightState = 0;

Servo myServo;

const float UNITY_X_MIN = -10.0f;
const float UNITY_X_MAX = 10.0f;

int mapUnityXToServo(float xValue) {
  if (UNITY_X_MAX <= UNITY_X_MIN) {
    return 90;
  }

  float normalized = (xValue - UNITY_X_MIN) / (UNITY_X_MAX - UNITY_X_MIN);
  int mapped = (int)(normalized * 180.0f);
  return constrain(mapped, 0, 180);
}

bool parseControlCommand(const char* line, int& nextServoAngle, int& nextLightState) {
  int parsedServo = 0;
  int parsedLight = 0;
  float parsedX = 0.0f;
  float parsedY = 0.0f;
  float parsedZ = 0.0f;

  if (sscanf(line, "CTRL,%d,%d", &parsedServo, &parsedLight) == 2) {
    nextServoAngle = constrain(parsedServo, 0, 180);
    nextLightState = (parsedLight != 0) ? 1 : 0;
    return true;
  }

  if (sscanf(line, "%d,%d", &parsedServo, &parsedLight) == 2) {
    nextServoAngle = constrain(parsedServo, 0, 180);
    nextLightState = (parsedLight != 0) ? 1 : 0;
    return true;
  }

  if (sscanf(line, "%f,%f,%f,%d", &parsedX, &parsedY, &parsedZ, &parsedLight) == 4) {
    nextServoAngle = mapUnityXToServo(parsedX);
    nextLightState = (parsedLight != 0) ? 1 : 0;
    return true;
  }

  return false;
}

void handleIncomingUnityData(const char* line) {
  if (strcmp(line, "PING") == 0) {
    Serial.println("READY");
    return;
  }

  int nextServoAngle = 90;
  int nextLightState = 0;

  if (!parseControlCommand(line, nextServoAngle, nextLightState)) {
    return;
  }

  servoAngle = nextServoAngle;
  lightState = nextLightState;
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("OK");
  delay(5);
  digitalWrite(LED_BUILTIN, LOW);
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
  myServo.write(servoAngle);
  digitalWrite(LED_PIN, lightState ? HIGH : LOW);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  myServo.attach(SERVO_PIN);

  Serial.begin(115200);
  delay(1200);
  Serial.println("READY");

  if (RUN_STARTUP_SELF_TEST) {
    digitalWrite(LED_PIN, HIGH);
    myServo.write(0);
    delay(1000);
    myServo.write(180);
    delay(1000);
    myServo.write(90);
    digitalWrite(LED_PIN, LOW);
  }

  myServo.write(servoAngle);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  processIncomingSerial();
  updateActuators();
}
