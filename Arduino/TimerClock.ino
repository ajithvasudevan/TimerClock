#include <Wire.h>
#include <EEPROM.h>
#include <AceButton.h>
#include "RTClib.h"
RTC_DS3231 rtc;
using namespace ace_button;

int modePin = A0;
int resetPin = A1;
int colonPin = A2;
int setPin = A3;
int alarmOut = 12;
int SET_BUTTON_STATE = 0;
int DISPLAY_MODE = 0;
int TIMER_MODE = 0;
int hr, mt;
int SET_DELAY = 50;
int timeChanged = 0;
int baseDelay = 1000;
int bias = 300;
int setDelayCtr = 0;

int CLOCK_SEC_PREV = 0;
int CLOCK_SEC = 0;
int CLOCK_MIN = 0;
int CLOCK_HR = 0;
int CLOCK_SEC_AL = 0;
int CLOCK_SEC_AL_ADDR = 0;

int CLOCK_MIN_AL_ADDR = 1;
int CLOCK_MIN_AL = EEPROM.read(CLOCK_MIN_AL_ADDR);

int CLOCK_HR_AL_ADDR = 2;
int CLOCK_HR_AL = EEPROM.read(CLOCK_HR_AL_ADDR);

int TIME_MIN = 0;
int TIME_SEC = 0;

int TIME_MIN_AL_ADDR = 3;
int TIME_MIN_AL = EEPROM.read(TIME_MIN_AL_ADDR);

int TIME_SEC_AL_ADDR = 4;
int TIME_SEC_AL = EEPROM.read(TIME_SEC_AL_ADDR);

int disp_left = 0;
int disp_right = 0;

int prev_second = 0;
int msec = 0;
long millisAtZero = 0;
bool zeroPad = true;
int colonState = 0;

AceButton mode_button(modePin, HIGH, 0);
AceButton set_button(setPin, HIGH, 1);
AceButton reset_button(resetPin, HIGH, 2);

void handleModeEvent(AceButton*, uint8_t, uint8_t);

// bits representing segments A through G (and decimal point) for numerals 0-9
const int numeral[10] = {
  //ABCDEFG
  B1111110, // 0
  B0110000, // 1
  B1101101, // 2
  B1111001, // 3
  B0110011, // 4
  B1011011, // 5
  B1011111, // 6
  B1110000, // 7
  B1111111, // 8
  B1111011, // 9
};
// pins for decimal point and each segment
// G,F,E,D,C,B,A
const int segmentPins[] = { 13, 11, 10, 9, 8, 7, 6 };
const int nbrDigits = 4; // the number of digits in the LED display
//dig 0 1 2 3
const int digitPins[nbrDigits] = { 2, 3, 4, 5 };


void handleEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  uint8_t id = button->getId();

  switch (eventType) {

    case AceButton::kEventClicked:

      if (id == mode_button.getId()) {
        DISPLAY_MODE++;
        if (DISPLAY_MODE > 2) DISPLAY_MODE = 0;
        
        if(DISPLAY_MODE == 1 || DISPLAY_MODE == 2) {
          if(TIMER_MODE == 0) {
            disp_left = CLOCK_HR;
            disp_right = CLOCK_MIN;
          }
        }
      }

      if (id == reset_button.getId()) {
        TIME_SEC = 0;
        TIME_MIN = 0;
//        digitalWrite(alarmOut, LOW);
//        delay(100);
//        digitalWrite(alarmOut, HIGH);        
      }
      
      if (id == set_button.getId()) {
        if(TIMER_MODE == 0) {
          if(DISPLAY_MODE == 1) { disp_right++; if(disp_right > 59) disp_right = 0; }
          if(DISPLAY_MODE == 2) { disp_left++; if(disp_left > 23) disp_left = 0; }
          rtc.adjust(DateTime(2025, 1, 1, disp_left, disp_right, 0));
          zeroPad = true;
        }
        if(TIMER_MODE == 1) {
          if(DISPLAY_MODE == 1) { TIME_SEC_AL++; if(TIME_SEC_AL > 59) TIME_SEC_AL = 0; }
          if(DISPLAY_MODE == 2) { TIME_MIN_AL++; if(TIME_MIN_AL > 99) TIME_MIN_AL = 0; }
          EEPROM.write(TIME_SEC_AL_ADDR, TIME_SEC_AL);
          EEPROM.write(TIME_MIN_AL_ADDR, TIME_MIN_AL);           
        }
        if(TIMER_MODE == 2) {
          if(DISPLAY_MODE == 1) { CLOCK_MIN_AL++; if(CLOCK_MIN_AL > 59) CLOCK_MIN_AL = 0; }
          if(DISPLAY_MODE == 2) { CLOCK_HR_AL++; if(CLOCK_HR_AL > 23) CLOCK_HR_AL = 0; }
          EEPROM.write(CLOCK_MIN_AL_ADDR, CLOCK_MIN_AL);
          EEPROM.write(CLOCK_HR_AL_ADDR, CLOCK_HR_AL);
        }                 
      }
      break;
    case AceButton::kEventLongPressed:
      if (id == mode_button.getId()) {
        if(DISPLAY_MODE == 0) {
          if(TIMER_MODE == 0) { TIMER_MODE = 1; }
          else if(TIMER_MODE == 1) { TIMER_MODE = 0; }
        }
        if(TIMER_MODE == 1 && (DISPLAY_MODE == 1 || DISPLAY_MODE == 2)) {
          TIME_SEC_AL = 0;
          TIME_MIN_AL = 0;
          EEPROM.write(TIME_SEC_AL_ADDR, TIME_SEC_AL);
          EEPROM.write(TIME_MIN_AL_ADDR, TIME_MIN_AL);           
        }        
        if(TIMER_MODE == 2) { TIMER_MODE = 0; DISPLAY_MODE = 0; }

      }
      if (id == set_button.getId()) {
        SET_BUTTON_STATE = 2;
        if((TIMER_MODE == 0) && DISPLAY_MODE == 0) { TIMER_MODE = 2; SET_BUTTON_STATE = 0; }
      }
      break;
    case AceButton::kEventReleased:
      if (id == set_button.getId()) {
        SET_BUTTON_STATE = 0;
      }
      break;
  }
}





void setup()
{
  //Serial.begin(115200);

  if (! rtc.begin()) {
//    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    rtc.adjust(DateTime(2020, 6, 1, 0, 0, 0));
  }
  
   for(int i=0; i < 7; i++) {
      pinMode(segmentPins[i], OUTPUT); 
   }
   for(int i=0; i < nbrDigits; i++) {
      pinMode(digitPins[i], OUTPUT);
   }
   pinMode(colonPin, OUTPUT);
   pinMode(alarmOut, OUTPUT);
   digitalWrite(alarmOut, HIGH);
   pinMode(modePin, INPUT_PULLUP);
   pinMode(setPin, INPUT_PULLUP);
   pinMode(resetPin, INPUT_PULLUP);

  ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
} 


void calcClock() {
  DateTime now = rtc.now();
  CLOCK_SEC = now.second();
  CLOCK_MIN = now.minute();
  CLOCK_HR = now.hour();
  if(CLOCK_SEC != prev_second) { prev_second = CLOCK_SEC; millisAtZero = millis(); }
  msec = millis() - millisAtZero;
}

void calcTime() {
  if(CLOCK_SEC != CLOCK_SEC_PREV) { CLOCK_SEC_PREV = CLOCK_SEC; TIME_SEC++; if(TIME_SEC > 59) { TIME_SEC = 0; TIME_MIN++; if(TIME_MIN > 99) TIME_MIN = 0; }}
}

int C_ALM_FLG = 0;
int T_ALM_FLG = 0;
long c_alarm_start = 0;
long t_alarm_start = 0;

void checkClockAlarm() {
  if((millis() - c_alarm_start) > 5000) {
    if(CLOCK_SEC == 0 && CLOCK_MIN == CLOCK_MIN_AL && CLOCK_HR == CLOCK_HR_AL) {
      if(C_ALM_FLG == 0) {
        C_ALM_FLG = 1;
        c_alarm_start = millis();
        digitalWrite(alarmOut, LOW);
        delay(100);
        digitalWrite(alarmOut, HIGH);
      }
    }
  } else {
    C_ALM_FLG = 0;
  }
}


void checkTimerAlarm() {
  if(TIME_MIN_AL == 0 && TIME_SEC_AL == 0) return;
  
  if((millis() - t_alarm_start) > 5000) {
    if(TIME_MIN == TIME_MIN_AL && TIME_SEC == TIME_SEC_AL) {
      if(T_ALM_FLG == 0) {
        T_ALM_FLG = 1;
        t_alarm_start = millis();
        digitalWrite(alarmOut, LOW);
        delay(100);
        digitalWrite(alarmOut, HIGH);
      }
    }
  } else {
    T_ALM_FLG = 0;
  }
}

void colon() {
  if(colonState == 2) {
    if(msec < 500) digitalWrite(colonPin, LOW);
    else digitalWrite(colonPin, HIGH);
  } else if(colonState == 1) digitalWrite(colonPin, LOW);
  else digitalWrite(colonPin, HIGH);
}

void render() {
  int disp_num = 100 * disp_left + disp_right;
  showNumber(disp_num, zeroPad);
}


void showClockIfReqd() {
  if(DISPLAY_MODE == 0 && TIMER_MODE == 0) {
    disp_left = CLOCK_HR;

   if(disp_left > 12) disp_left = disp_left - 12;
   if(disp_left == 0) disp_left = 12;
    
    disp_right = CLOCK_MIN;
    zeroPad = false;
    colonState = 2;
  }
}


void showTimerIfReqd() {
  if(DISPLAY_MODE == 0 && TIMER_MODE == 1) {
    disp_left = TIME_MIN;
    disp_right = TIME_SEC;
    zeroPad = false;
    colonState = 1;
  }
}


void showTimerAlarmIfReqd() {
  if(DISPLAY_MODE != 0 && TIMER_MODE == 1) {
    disp_left = TIME_MIN_AL;
    disp_right = TIME_SEC_AL;
    zeroPad = true;
    colonState = 1;
  }
}


void showClockAlarmIfReqd() {
  if(TIMER_MODE == 2) {
    disp_left = CLOCK_HR_AL;
    disp_right = CLOCK_MIN_AL;
    zeroPad = true;
    colonState = 1;
  }
}


void checkSetButtonState() {
  if(setDelayCtr == 1) {
    if(SET_BUTTON_STATE == 2) {
      if(TIMER_MODE == 0) {
        if(DISPLAY_MODE == 1) { disp_right--; if(disp_right < 0 ) disp_right = 59; }
        if(DISPLAY_MODE == 2) { disp_left--; if(disp_left < 0) disp_left = 23; }
        rtc.adjust(DateTime(2025, 1, 1, disp_left, disp_right, 0));
        zeroPad = true;
      }
      if(TIMER_MODE == 1) {
        if(DISPLAY_MODE == 1) { TIME_SEC_AL--; if(TIME_SEC_AL < 0) TIME_SEC_AL = 59; }
        if(DISPLAY_MODE == 2) { TIME_MIN_AL--; if(TIME_MIN_AL < 0) TIME_MIN_AL = 99; }
        EEPROM.write(TIME_SEC_AL_ADDR, TIME_SEC_AL);
        EEPROM.write(TIME_MIN_AL_ADDR, TIME_MIN_AL);        
        
      }
      if(TIMER_MODE == 2) {
        if(DISPLAY_MODE == 1) { CLOCK_MIN_AL--; if(CLOCK_MIN_AL < 0) CLOCK_MIN_AL = 59; }
        if(DISPLAY_MODE == 2) { CLOCK_HR_AL--; if(CLOCK_HR_AL < 0) CLOCK_HR_AL = 23; }
        EEPROM.write(CLOCK_MIN_AL_ADDR, CLOCK_MIN_AL);
        EEPROM.write(CLOCK_HR_AL_ADDR, CLOCK_HR_AL);        
      }               
    }
  }
}



void loop()
{
    calcClock();
    checkClockAlarm();
    checkTimerAlarm();
    calcTime();
    colon();
    showClockIfReqd();
    showTimerIfReqd();
    checkSetButtonState();
    showTimerAlarmIfReqd();
    showClockAlarmIfReqd();
    render();
    setDelayCtr++; if(setDelayCtr == SET_DELAY) setDelayCtr = 0;
    mode_button.check();
    set_button.check();
    reset_button.check();
   
} 
 
void showNumber( int number, bool zeroPad)
{
    int orgNum = number;
    for( int digit = 0; digit <nbrDigits; digit++)  {
        if(zeroPad  || !((digit == 3 && orgNum < 1000) || (digit == 2 && orgNum < 100) || (digit == 1 && orgNum < 10))) showDigit( number % 10, digit);
        number = number / 10;
    }
} 
 
// Displays given number on a 7-segment display at the given digit position
void showDigit( int number, int digit) {
    digitalWrite( digitPins[digit], LOW );

   for(int segment = 0; segment < 7; segment++)  {
      boolean isBitSet = bitRead(numeral[number], segment);
      // isBitSet will be true if given bit is 1
      isBitSet = ! isBitSet; // Code Option*
      // uncomment the above Code Option line for common anode display
      digitalWrite( segmentPins[segment], isBitSet);
   }
   
   if(DISPLAY_MODE == 1 && ((digit == 0) || (digit == 1))) {  
      if(setDelayCtr < (SET_DELAY / 2)) digitalWrite( digitPins[digit], HIGH );
      else digitalWrite( digitPins[digit], LOW );
   } 
   else if(DISPLAY_MODE == 2 && ((digit == 2) || (digit == 3))) {  
      if(setDelayCtr < (SET_DELAY / 2)) digitalWrite( digitPins[digit], HIGH );
      else digitalWrite( digitPins[digit], LOW );
   } else digitalWrite( digitPins[digit], HIGH );
   
   
   
   if(number == 1) delayMicroseconds(baseDelay); 
   else if(number == 7)  delayMicroseconds(baseDelay + bias);
   else if(number == 4)  delayMicroseconds(baseDelay + 2*bias);
   else if(number == 3)  delayMicroseconds(baseDelay + 3*bias);
   else if(number == 2)  delayMicroseconds(baseDelay + 3*bias);
   else if(number == 5)  delayMicroseconds(baseDelay + 3*bias);
   else if(number == 6)  delayMicroseconds(baseDelay + 4*bias);
   else if(number == 9)  delayMicroseconds(baseDelay + 4*bias);
   else if(number == 0)  delayMicroseconds(baseDelay + 4*bias);
   else delayMicroseconds(baseDelay + 5*bias);
   digitalWrite( digitPins[digit], LOW );
}