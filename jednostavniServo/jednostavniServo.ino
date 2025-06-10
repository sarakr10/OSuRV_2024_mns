#include <Servo.h>

Servo myServo;

const int buttonPin = 10;       // Taster je na pinu 10
int angle = 0;                  // Trenutni ugao
bool lastButtonState = HIGH;    // Prethodno stanje tastera

void setup() {
  Serial.begin(9600);           // Aktiviraj Serial Monitor
  myServo.attach(8);            // Servo na pinu 8
  pinMode(buttonPin, INPUT_PULLUP); // Koristi pull-up
  myServo.write(angle);         // Postavi početni ugao
}

void loop() {
  bool buttonState = digitalRead(buttonPin);

  // Ako je taster sada LOW, a bio je HIGH → klik
  if (lastButtonState == HIGH && buttonState == LOW) {
    angle += 90;
    if (angle > 180) {
      angle = 0;
    }
    myServo.write(angle);
    Serial.print("Ugao: ");
    Serial.println(angle);
    delay(300);  // Kraći debounce (brža reakcija)
  }

  lastButtonState = buttonState;
}
