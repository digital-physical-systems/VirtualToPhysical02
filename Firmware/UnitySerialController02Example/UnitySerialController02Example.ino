#include <elapsedMillis.h>

const int POT_PIN = A0;
const int BUTTON_PIN = 2;
const int GREEN_LED_PIN = 7;
const int RED_LED_PIN = 8;
const unsigned long SEND_INTERVAL_MS = 50;
const unsigned long DEFAULT_RED_PULSE_MS = 500;

elapsedMillis sendTimer;
bool flashlightState = false;
unsigned long redLedPulseEndTimeMs = 0;

const int SERIAL_BUFFER_SIZE = 32;
char serialBuffer[SERIAL_BUFFER_SIZE];
int serialBufferIndex = 0;

void triggerRedLedPulse(unsigned long durationMs) {
  unsigned long safeDuration = durationMs > 0 ? durationMs : 1;
  redLedPulseEndTimeMs = millis() + safeDuration;
}

bool isRedPulseActive() {
  return (long)(millis() - redLedPulseEndTimeMs) < 0;
}

void handleIncomingCommand(const char* command) {
  int stateValue = 0;
  unsigned long pulseDurationMs = 0;

  if (sscanf(command, "FLASHLIGHT,%d", &stateValue) == 1) {
    flashlightState = stateValue != 0;
  } else if (sscanf(command, "REDPULSE,%lu", &pulseDurationMs) == 1) {
    triggerRedLedPulse(pulseDurationMs);
  } else if (sscanf(command, "REDLED,%d", &stateValue) == 1) {
    // Backward compatibility: REDLED,1 triggers a default pulse.
    if (stateValue != 0) {
      triggerRedLedPulse(DEFAULT_RED_PULSE_MS);
    }
  }
}

void updateActuators() {
  digitalWrite(GREEN_LED_PIN, flashlightState ? HIGH : LOW);
  digitalWrite(RED_LED_PIN, isRedPulseActive() ? HIGH : LOW);
}

void processIncomingSerial() {
  while (Serial.available() > 0) {
    char incomingChar = Serial.read();

    if (incomingChar == '\r') {
      continue;
    }

    if (incomingChar == '\n') {
      serialBuffer[serialBufferIndex] = '\0';

      if (serialBufferIndex > 0) {
        handleIncomingCommand(serialBuffer);
      }

      serialBufferIndex = 0;
    } else if (serialBufferIndex < SERIAL_BUFFER_SIZE - 1) {
      serialBuffer[serialBufferIndex++] = incomingChar;
    } else {
      // If a line is too long, reset and wait for the next command.
      serialBufferIndex = 0;
    }
  }
}

void setup() {
  pinMode(POT_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);

  Serial.begin(115200);
}

void readSensors() {
  int potValue = analogRead(POT_PIN);
  bool buttonState = digitalRead(BUTTON_PIN) == LOW; // Active low button

  // Send one newline-terminated CSV message: potValue,button(0|1)
  Serial.print(potValue);
  Serial.print(',');
  Serial.println(buttonState ? 1 : 0);
} 

void loop() {
  processIncomingSerial();
  updateActuators();

  if (sendTimer >= SEND_INTERVAL_MS) {
    sendTimer = 0; // Reset the timer
    readSensors(); // Read and send sensor data
  }
}
