/*
 * Fencing Hit Indicator
 * Copyright (c) 2023 David Rice
 * 
 * This is a simple fencing hit indicator that implements the hit time and
 * lockout time as specified by current foil fencing rules.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define RED_FENCER 2
#define GREEN_FENCER 4

#define RED_LED 8
#define GREEN_LED 9

#define BUZZER 10

#define STATE_READY 0
#define STATE_LOCKOUT 1
#define STATE_RESETTING 2

#define MIN_BREAK_TIME 14
#define LOCKOUT_TIME 300

unsigned long red_break_timestamp = 0, green_break_timestamp = 0;
unsigned long red_valid_timestamp = 0, green_valid_timestamp = 0;
unsigned long lockout_timestamp = 0;

unsigned int state = STATE_RESETTING;

bool red_pending = false;
bool green_pending = false;

bool lockout_pending = false;

void setup() {
  pinMode(RED_FENCER, INPUT_PULLUP);
  pinMode(GREEN_FENCER, INPUT_PULLUP);

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  pinMode(BUZZER, OUTPUT);
}

void loop() {
  switch (state) {
    case STATE_RESETTING:
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, HIGH);

      digitalWrite(BUZZER, LOW);

      red_pending = false;
      green_pending = false;
      lockout_pending = false;

      state = STATE_READY;
      break;
    case STATE_READY:
      if (digitalRead(RED_FENCER) == HIGH) {
        if (red_pending) {
          if (millis() >= red_valid_timestamp) {
            if (!lockout_pending) {
              lockout_timestamp = millis() + LOCKOUT_TIME;
              lockout_pending = true;
            }
            digitalWrite(RED_LED, LOW);
            digitalWrite(BUZZER, HIGH);
          }
        } else {
          red_valid_timestamp = millis() + MIN_BREAK_TIME;
          red_pending = true;
        }
      } else {
        red_pending = false;
      }

      if (digitalRead(GREEN_FENCER) == HIGH) {
        if (green_pending) {

          if (millis() >= green_valid_timestamp) {
            if (!lockout_pending) {
              lockout_timestamp = millis() + LOCKOUT_TIME;
              lockout_pending = true;
            }
            digitalWrite(GREEN_LED, LOW);
            digitalWrite(BUZZER, HIGH);
          }
        } else {
          green_valid_timestamp = millis() + MIN_BREAK_TIME;
          green_pending = true;
        }
      } else {
        green_pending = false;
      }

      if (lockout_pending && (millis() >= lockout_timestamp)) {
        state = STATE_LOCKOUT;
      }
      break;
    case STATE_LOCKOUT:
      delay(2000);
      digitalWrite(BUZZER, LOW);
      delay(2000);

      state = STATE_RESETTING;
      break;
  }
}
