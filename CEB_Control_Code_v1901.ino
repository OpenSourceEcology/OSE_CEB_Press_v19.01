/* Open Source Ecology CEB Press v19.01 with Arduino Uno or Mega as controller of choice.
 CC-BY-SA, GPLv3, and OSE License for Distributive Economics
 Use Mega to keep consistency with OSE D3D 3D Printer to minimize GVCS part count.
 Switches FET's HIGH/LOW to control two hydraulic solenoids,
 measures piston motion time relative to pressure sensor trigger,
 and repeats cycle while auto calibrating timing from previous cycles and startup positions.

 Extension time is measured. Orientation of machine is such that cylinder retracts to the right.
 See sequence at http://bit.ly/2Hnuk6F

 Faults should be self-resolving based on pressing sequence: pressure trigger reverses direction
 at fault point and no block results. There is one likely place for faults to occur: soil load into chamber.

 Uses HiLetgo Relay Shield 5V 4 Channel for Arduino:

 https://www.amazon.com/HiLetgo-Relay-Shield-Channel-Arduino/dp/B07F7Y55Z7/ref=sr_1_2?ie=UTF8&qid=1547696929&sr=8-2&keywords=hiletgo+relay+shield

 Note pins must be trimmed flat under Relay 1 connector and must be insulated to prevent shorts.

 Relay 1 is controlled by digital pin 7
 Relay 2 is controlled by digital pin 6
 Relay 3 is controlled by digital pin 5
 Relay 4 is controlled by digital pin 4

 A high written to a pin turns the relay ON
 A low written to a pin turns the relay OFF

 Contributions by:
 Abe Anderson
 http://opensourceecology.org/wiki/AbeAnd_Log
 Marcin Jakubowski
 http://opensourceecology.org/wiki/Marcin_Log

 Unfamiliar with code structures? See https://www.arduino.cc/en/Reference/HomePage
 */

//defines to make it easier for non-coders to make adjustments for troubleshooting and custom changes

#define SOLENOID_UP 4         //Extension. See pin mapping above.
#define SOLENOID_DOWN 5       //swap these pin numbers for wire inversion
#define SOLENOID_LEFT 6       //Extension.
#define SOLENOID_RIGHT 7      //swap these pin numbers for wire inversion

#define PRESSURE_SENSOR 13    //Needs pins adjacent to get 8-pin dupont housing for both selector and sensor
//#define SELECTOR_RESET 12     //Second 8-pin Dupont housing for the solenoids
#define SELECTOR_QUARTER 11   //Reset is the shutdown/initialization procedure. All procedures are selected by
#define SELECTOR_HALF 10      //the WHILE function. QUARTER to FULL refers to brick thickness.
#define SELECTOR_3QUARTER 9   //Secondary cylinder timing is measured only.
#define SELECTOR_FULL 8       //Primary cylinder thickness setting is based on secondary cylinder motion.

#define PRESSURE_SENSOR_DEBOUNCE 20     //milliseconds to delay for pressure sensor debounce
#define DELAY 500                       // 1/2 sec extra to compress brick via main Cyl (default 500ms)
                                        //user defined function declarations tell the compiler what type parameters to expect for the function definitions at the bottom
bool lowPressure();                     //function to read pressure sensor
bool resetSelected();                     //
bool quarterSelected();                     //
bool halfSelected();                     //
bool threequarterSelected();                     //
bool fullSelected();                     //
                                        //Global variables
unsigned long drawerExtTime = 0;        //Time measurement for calibrating motion.
unsigned long previousMillis = 0;       //time measurement for expansion

void setup() {

  //initialize pin I/O Inputs and turn everything off to avoid startup glitches
  pinMode(SELECTOR_QUARTER, INPUT_PULLUP)
  pinMode(SELECTOR_HALF, INPUT_PULLUP)
  pinMode(SELECTOR_3QUARTER, INPUT_PULLUP)
  pinMode(SELECTOR_FULL, INPUT_PULLUP)
  pinMode(SOLENOID_RIGHT, OUTPUT);
  digitalWrite(SOLENOID_RIGHT, LOW);
  pinMode(SOLENOID_LEFT, OUTPUT);
  digitalWrite(SOLENOID_LEFT, LOW);
  pinMode(SOLENOID_DOWN, OUTPUT);
  digitalWrite(SOLENOID_DOWN, LOW);
  pinMode(SOLENOID_UP, OUTPUT);
  digitalWrite(SOLENOID_UP, LOW);
  pinMode(PRESSURE_SENSOR, INPUT);
  digitalWrite(PRESSURE_SENSOR, INPUT_PULLUP);
}

void loop() {

                                             //Step 0 Reset/Initialize - Brick pressing sequence - http://bit.ly/2Hnuk6F
while (resetSelected() == true) {
  while (lowPressure() == true) {            //Move drawer cylinder right
    digitalWrite(SOLENOID_RIGHT, HIGH);
  }
  digitalWrite(SOLENOID_RIGHT, LOW);

  while (lowPressure() == true) {            //Move main cylinder up
    digitalWrite(SOLENOID_UP, HIGH);
  }
  digitalWrite(SOLENOID_RIGHT, LOW);
}                                             //Step 1 Calibration + Soil Loading/Brick Ejection
if (resetSelected() == false) {             //Proceeds only if selector is not on 0 (reset) position
         previousMillis = millis();          //lowPressure() call adds a slight debounce delay to the millis diffirence
  while (lowPressure() == true) {            //Here we eject brick if this is a repeat cycle.
    digitalWrite(SOLENOID_LEFT, HIGH);
  }
  digitalWrite(SOLENOID_LEFT, LOW);
         drawerExtTime = millis() - previousMillis;
}
  if (quarterSelected() == true) {          //Goes through different selections.
    digitalWrite(SOLENOID_DOWN, HIGH);       //Selects brick thickness as in http://bit.ly/2RYpQYa
    delay(drawerExtTime*0.5);                //Thickness multiplier of 0.5 based on cylinder diameters
    digitalWrite(SOLENOID_DOWN, LOW);
  }

  if (halfSelected() == true) {
     digitalWrite(SOLENOID_DOWN, HIGH);
     delay(drawerExtTime*0.9);
     digitalWrite(SOLENOID_DOWN, LOW);
    }

  if (threequarterSelected() == true) {
     digitalWrite(SOLENOID_DOWN, HIGH);
     delay(drawerExtTime*1.3);
     digitalWrite(SOLENOID_DOWN, LOW);
    }

  if (fullSelected() == true) {
       while (lowPressure() == true) {
    digitalWrite(SOLENOID_DOWN, HIGH);
  }
  digitalWrite(SOLENOID_DOWN, LOW);
    }
                                                 //Step 2 Compression Chamber Closure
if (resetSelected() == false) {
  digitalWrite(SOLENOID_RIGHT, HIGH);
  delay(drawerExtTime*0.75*0.5);           //0.5 is the half-way point of drawer cylinder; 0.75 is the Drawer Multiplier
  digitalWrite(SOLENOID_RIGHT, LOW);       //Can modify the 0.5 to fine tune the drawer closure location
}

                                                 //Step 3 Compression by main cylinder with delay
if (resetSelected() == false) {
  while (lowPressure() == true) {
    digitalWrite(SOLENOID_UP, HIGH);
  }
  delay(DELAY);
  digitalWrite(SOLENOID_UP, LOW);
}
                                                 //Step 4 Ejection
if (resetSelected() == false) {
  while (lowPressure() == true) {
    digitalWrite(SOLENOID_RIGHT, HIGH);
  }
  digitalWrite(SOLENOID_RIGHT, LOW);

  while (lowPressure() == true) {
    digitalWrite(SOLENOID_UP, HIGH);
  }
  digitalWrite(SOLENOID_UP, LOW);
}
                                                 //Cycle through sequence from 1 (recalibration)



}                                                //end of loop
                                                 //user defined function definitions are code that can be called from almost anywhere (within scope).
                                                 //reads pressure sensor state HIGH is false and LOW is true
bool lowPressure() {
  if (digitalRead(PRESSURE_SENSOR) == HIGH) {
    delay(PRESSURE_SENSOR_DEBOUNCE);
    if (digitalRead(PRESSURE_SENSOR) == HIGH) {
      return false;
    }
    else {
      return true;
    }
  }
  else {
    return true;
  }}

  //reads selector  - HIGH is false, LOW is true- SELECTOR_RESET, SELECTOR_QUARTER, SELECTOR_HALF, SELECTOR_3QUARTERS, SELECTOR_FULL,
bool resetSelected() {
  if (threequarterSelected() == false && halfSelected() == false && quarterSelected() == false && fullSelected() == false) {
    return true;
  }
  else {
    return false;
  }}

  bool quarterSelected() {
  if (digitalRead(SELECTOR_QUARTER) == HIGH) {
    delay(PRESSURE_SENSOR_DEBOUNCE);
    if (digitalRead(SELECTOR_QUARTER) == HIGH) {
      return false;
    }
    else {
      return true;
    }
  }
  else {
    return true;
  }}

  bool halfSelected() {
  if (digitalRead(SELECTOR_HALF) == HIGH) {
    delay(PRESSURE_SENSOR_DEBOUNCE);
    if (digitalRead(SELECTOR_HALF) == HIGH) {
      return false;
    }
    else {
      return true;
    }
  }
  else {
    return true;
  }}

  bool threequarterSelected() {
  if (digitalRead(SELECTOR_3QUARTER) == HIGH) {
    delay(PRESSURE_SENSOR_DEBOUNCE);
    if (digitalRead(SELECTOR_3QUARTER) == HIGH) {
      return false;
    }
    else {
      return true;
    }
  }
  else {
    return true;
  }}

    bool fullSelected() {
  if (digitalRead(SELECTOR_FULL) == HIGH) {
    delay(PRESSURE_SENSOR_DEBOUNCE);
    if (digitalRead(SELECTOR_FULL) == HIGH) {
      return false;
    }
    else {
      return true;
    }
  }
  else {
    return true;
  }}
