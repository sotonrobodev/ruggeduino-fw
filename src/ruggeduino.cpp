#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// NB: If you make any changes to this file, then the sample source for
// teams to extend in srweb (resources/kit/ruggeduino-fw.ino) will also
// need changing similarly (though there are some intentional differences)

// Arduino default baud is 9600, but the SR kit uses 115200
// If you change this, you'll also need to change the value in the
// sr-robot API layer (in ruggeduino.py)
#define SERIAL_BAUD 115200

// Determined through experimentation.
#define SERVO_TICKS_MIN 150
#define SERVO_TICKS_MAX 600

void setup() {
  Serial.begin(SERIAL_BAUD);

  pwm.begin();
  pwm.setPWMFreq(60);
}

int read_pin() {
  while (!Serial.available());
  int pin = Serial.read();
  return (int)(pin - 'a');
}

void command_read() {
  int pin = read_pin();
  // Read from the expected pin.
  int level = digitalRead(pin);
  // Send back the result indicator.
  if (level == HIGH) {
    Serial.write('h');
  } else {
    Serial.write('l');
  }
}

void command_analogue_read() {
  int pin = read_pin();
  int value = analogRead(pin);
  Serial.print(value);
}

void command_write(int level) {
  int pin = read_pin();
  digitalWrite(pin, level);
}

void command_mode(int mode) {
  int pin = read_pin();
  pinMode(pin, mode);
}

void loop() {
  int channel, pulseTicks;

  // Fetch all commands that are in the buffer
  while (Serial.available()) {
    int selected_command = Serial.read();
    // Do something different based on what we got:
    switch (selected_command) {
      case 'a':
        command_analogue_read();
        break;
      case 'r':
        command_read();
        break;
      case 'l':
        command_write(LOW);
        break;
      case 'h':
        command_write(HIGH);
        break;
      case 'i':
        command_mode(INPUT);
        break;
      case 'o':
        command_mode(OUTPUT);
        break;
      case 'p':
        command_mode(INPUT_PULLUP);
        break;
      case 'v':
        Serial.print("SRDduino:");
        Serial.print(FW_VER);
        break;
      case 's':
        // TODO: improve this protocol.
        // All other commands use a pure ASCII protocol, but is
        // this restriction really necessary?
        while (!Serial.available()) {}
        channel = Serial.read() - 'a';
        while (!Serial.available()) {}
        pulseTicks = Serial.read() << 8;
        while (!Serial.available()) {}
        pulseTicks |= Serial.read();
        if (channel >= 0 && channel <= 15) {
          pulseTicks = constrain(pulseTicks, SERVO_TICKS_MIN, SERVO_TICKS_MAX);
          // pulseTicks - the number of 'ticks' that the output should be high for
          // 1 tick is 1/4096 of a PWM period
          // PWM frequency is 60 Hz
          pwm.setPWM(channel, 0, pulseTicks);
        }
        break;
      default:
        // A problem here: we do not know how to handle the command!
        // Just ignore this for now.
        break;
    }
    Serial.print("\n");
  }
}
