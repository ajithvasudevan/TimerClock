# TimerClock
An Arduino + RTC (and optionally Wemos) - based Clock and TImer with Alarm

# About
This project implements a Clock and Timer (time counter with alarm). It has 3 buttons - MODE, SET and RESET.


# Button Operations

## Displaying TIMER and CLOCK
When the ClockTimer is powered ON, it displays the CLOCK.
While CLOCK is displayed, long-pressing (LP) MODE button changes the display to TIMER, which displays counting time in minutes and seconds.
While TIMER is displayed, long-pressing (LP) MODE button changes the display to CLOCK, which displays the current time in 12-hr format.

## Timer and Resetting Timer to 00:00
The Timer counts time in seconds and minutes starting from 0 seconds, up to 99 minutes and 59 seconds, after which it starts counting from 0 seconds again.
During the time counting, the Timer alarm rings once when the displayed Time (mm:ss) matches the Timer Alarm setting.
While the Timer (or Clock) is displayed, the Timer count can be reset to 00:00 by pressing the RESET button. This causes the Timer to re-start counting from 0 seconds.

## Timer-Alarm Setting
While TIMER is displayed, MODE SP to change display to Timer-Alarm (Minutes and Seconds) with blinking Timer-Alarm Seconds. Now SET SP increments Timer-Alarm Seconds and SET LP decrements Timer Alarm Seconds.
MODE SP again blinks the Timer-Alarm Minutes display. Now SET SP increments Timer-Alarm Minutes and SET LP decrements Timer-Alarm Minutes.
MODE SP again returns the display to TIMER.

## Setting Timer Alarm to 00:00 quickly
The Timer-Alarm display can be set to 00:00 quickly by MODE LP during the blinking phase of Timer-Alarm Setting. This disables the Timer Alarm Ringing.

## Clock Setting
While CLOCK is displayed, MODE can be single-pressed (SP) to blink the Minutes display. Now SET SP increments Minutes and SET long-press (LP) decrements Minutes.
MODE SP again blinks the Hours display. Now SET SP increments Hours and SET LP decrements Hours.
MODE SP again returns the display to CLOCK, with the changed time.

## Clock Alarm Setting
While CLOCK is displayed, SET LP to display the CLOCK ALARM TIME. Now MODE SP to blink the Clock Alarm Minutes display. Now SET SP increments Clock Alarm Minutes and SET long-press (LP) decrements Clock Alarm Minutes.
MODE SP again blinks the Clock Alarm Hours display. Now SET SP increments Clock Alarm Hours and SET LP decrements Hours.
MODE MODE SP again returns the display to CLOCK ALARM TIME, with the changed Clock Alarm time.
MODE LP to exit Clock Alarm Setting and go back to Clock display.


# Notes

* A **DS3231 RTC** is used to keep accurate time. 
* The **DS 3231** may be removed and a **Wemos D1 Mini** with appropriate code may be used to push *Network Time* to the Arduino. This requires code changes to the Arduino as well. 
* The Wemos-Arduino interfacing is via Serial Rx-Tx lines.
* A6 and A7 of Arduino cannot be used as Digital IO pins. THey can be used as Analog inputs only. So these are not suitable for use as button pins with libraries like AceButton.
* A DF Player Mini is used to make the alarm sound.
* D13, being a communication pin as well, has digital noise on it during boot. Hence this is not suitable as a trigger pin for the DF Player Mini. Specifically, the DF Player starts playing continuously on boot if D13 is used.
* If the DF Player is triggered using Arduino pin D12 directly, there is a 'bep-bep-bep' kind of noise in the played back sound. This went away when a 15k resistor was used in this trigger line.




