#include <Servo.h>

Servo myServo;

const int leftButtonPin = A1;     // Taster za levo
const int rightButtonPin = A2;   // Taster za desno

int angle = 0;                   // Trenutni ugao (0–180)
bool lastLeftState = HIGH;       // Prethodno stanje levog tastera
bool lastRightState = HIGH;      // Prethodno stanje desnog tastera

void setup() {
  Serial.begin(9600);
  myServo.attach(8);             // Servo na pinu 8

  pinMode(leftButtonPin, INPUT_PULLUP);
  pinMode(rightButtonPin, INPUT_PULLUP);

  myServo.write(angle);          // Postavi početni ugao
}

void loop() {
  bool leftState = digitalRead(leftButtonPin);
  bool rightState = digitalRead(rightButtonPin);

  // Levo pomeranje (smanjenje ugla)
  if (lastLeftState == HIGH && leftState == LOW) {
    angle -= 90;
    if (angle < 0) {
      angle = 0;
    }
    myServo.write(angle);
    Serial.print("LEVO");
    Serial.println(angle);
    delay(300);  // debounce
  }

  // Desno pomeranje (povećanje ugla)
  if (lastRightState == HIGH && rightState == LOW) {
    angle += 90;
    if (angle > 180) {
      angle = 180;
    }
    myServo.write(angle);
    Serial.print("DESNO");
    Serial.println(angle);
    delay(300);  // debounce
  }

  lastLeftState = leftState;
  lastRightState = rightState;
}
