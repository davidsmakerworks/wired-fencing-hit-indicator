#define RED_FENCER 2
#define GREEN_FENCER 4

#define RED_LED 12
#define GREEN_LED 13

#define BUZZER 10

#define STATE_READY 0
#define STATE_HIT 1
#define STATE_LOCKOUT 2
#define STATE_RESETTING 3

#define MIN_BREAK_TIME 14
#define LOCKOUT_TIME 300

unsigned long red_break_timestamp = 0, green_break_timestamp = 0;
unsigned long lockout_timestamp = 0;

unsigned int state = STATE_RESETTING;

void setup() {
  pinMode(RED_FENCER, INPUT_PULLUP);
  pinMode(GREEN_FENCER, INPUT_PULLUP);

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  pinMode(BUZZER, OUTPUT);
}

void loop() {
  switch(state) {
    case STATE_RESETTING:
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, HIGH);

      noTone(BUZZER);

      state = STATE_READY;
      break;
    case STATE_READY:
      if (digitalRead(RED_FENCER) == HIGH) {
        
      }
  }

}
