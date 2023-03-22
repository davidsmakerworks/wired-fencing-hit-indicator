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

// Pin numbers where the fencers' foils are connected
#define RED_FENCER 2
#define GREEN_FENCER 4

// Pin numbers where LED is connected with current-limiting resistor
#define RED_LED 8
#define GREEN_LED 9

// Pin number where active buzzer is connected
#define BUZZER 10

#define STATE_READY 0 // Monitoring for hits by one or both fencers
#define STATE_LOCKOUT 1 // Hit has been recorded and lockout time expired
#define STATE_RESETTING 2 // Resetting to prepare for next hit

// Per USA Fencing 2023 rules, the tip of the foil must be depressed for 14 ms
// to register a touch. 300 ms after the first touch is registered, the other
// fencer is no longer able to register a touch.
#define MIN_HIT_TIME 14 
#define LOCKOUT_TIME 300

// The time stamps (in ms) at which the red fencer's hit will be considered valid
// i.e., MIN_HIT_TIME ms after the hit was first detected
unsigned long red_valid_timestamp;
unsigned long green_valid_timestamp;

// The time stamp (in ms) at which point no further hits will be registered
// i.e., LOCKOUT_TIME ms after the first hit was registered
unsigned long lockout_timestamp;

// State variable for finite state machine in main loop
unsigned int state;

// Indicates if the tip is currently depressed (i.e., a hit will be registered
// if the tip remains depressed for MIN_HIT_TIME ms) 
bool red_pending;
bool green_pending;

// Indicated that a hit has been registered by one fencer, and now the other fencer
// has LOCKOUT_TIME ms to also register a hit
bool lockout_pending;

void setup() {
  // Foil circuit is normally closed, so the tip is connected to ground
  // and when the tip is depressed, the corresponding pin will be pulled high
  pinMode(RED_FENCER, INPUT_PULLUP);
  pinMode(GREEN_FENCER, INPUT_PULLUP);

  // These pins will sink current (active low)
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  // This pin will source current (active high)
  pinMode(BUZZER, OUTPUT);
}

void loop() {
  switch (state) {
    case STATE_RESETTING:
      // Turn off LEDs
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, HIGH);

      // Turn off buzzer
      digitalWrite(BUZZER, LOW);

      red_valid_timestamp = 0;
      green_valid_timestamp = 0;
      lockout_timestamp = 0;

      red_pending = false;
      green_pending = false;
      lockout_pending = false;

      state = STATE_READY;
      break;
    case STATE_READY:
      if (digitalRead(RED_FENCER) == HIGH) {
        // If tip is depressed...
        if (red_pending) { 
          // ...and if it has been depressed for MIN_HIT_TIME ms...
          if (millis() >= red_valid_timestamp) { 
            // ...then if this is the first hit recorded in the sequence...
            if (!lockout_pending) {
              // ...set the lockout timer to allow time for the other fencer to hit
              lockout_timestamp = millis() + LOCKOUT_TIME;
              lockout_pending = true;
            }
            digitalWrite(RED_LED, LOW); // Light up red LED
            digitalWrite(BUZZER, HIGH); // Sound buzzer
          }
        } else {
          // If this is the first moment that the tip has been depressed,
          // calculate time at which hit will be valid if tip remains depressed
          red_valid_timestamp = millis() + MIN_HIT_TIME;
          red_pending = true;
        }
      } else {
         // If tip is not depressed, or is released after less
         // than MIN_HIT_TIME ms, cancel pending hit
        red_pending = false;
      }

      if (digitalRead(GREEN_FENCER) == HIGH) {
        // If tip is depressed...
        if (green_pending) {
          // ...and if it has been depressed for MIN_HIT_TIME ms...
          if (millis() >= green_valid_timestamp) { 
            // ...then if this is the first hit recorded in the sequence...
            if (!lockout_pending) { 
              // ...set the lockout timer to allow time for the other fencer to hit
              lockout_timestamp = millis() + LOCKOUT_TIME; 
              lockout_pending = true;
            }
            digitalWrite(GREEN_LED, LOW); // Light up green LED
            digitalWrite(BUZZER, HIGH); // Sound buzzer
          }
        } else {
          // If this is the first moment that the tip has been depressed,
          // calculate time at which hit will be valid if tip remains depressed
          green_valid_timestamp = millis() + MIN_HIT_TIME;
          green_pending = true;
        }
      } else {
         // If tip is not depressed, or is released after less
         // than MIN_HIT_TIME ms, cancel pending hit
        green_pending = false;
      }

      // Continue to check for a hit scored by the other fencer until
      // lockout time has expired
      if (lockout_pending && (millis() >= lockout_timestamp)) {
        state = STATE_LOCKOUT; // Go into lockout state after LOCKOUT_TIME ms
      }
      break;
    case STATE_LOCKOUT:
      // Allow buzzer to sound for 2 seconds, keep LEDs on for
      // additional 2 seconds after that
      // TODO: Make these delay values defined constants instead of magic numbers
      delay(2000);
      digitalWrite(BUZZER, LOW);
      delay(2000);

      state = STATE_RESETTING; // Reset for next hit
      break;
  }
}
