/*!
 * @file lowPowerAndWakeUp.ino
 * @brief Set alarm, and use interrput pin to trigger MCU wake up
 * @n Experiment phenomenon: Set the alarm clock to trigger at a specified time.
 * @n                        Connect SQW pin with DIGITALPIN2.
 * @n                        Print information on serial port after the alarm clock is triggered.
 * @n                        This demo only works on avr
 * @copyright	Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @licence     The MIT License (MIT)
 * @author [LuoYufeng](yufeng.luo@dfrobot.com)
 * @version  V0.1
 * @date  2019-08-19
 * @get from https://www.dfrobot.com
 * @url https://github.com/DFRobot/DFRobot_DS3231M
 */
#include "DFRobot_DS3231M.h"
#include <avr/sleep.h>

volatile  int8_t alarmFlag = 0;

DFRobot_DS3231M rtc;
int t = 0;

void setup(void)
{
    Serial.begin(9600);
    /*Wait for the chip to be initialized completely, and then exit*/
    while(rtc.begin() != true){
        Serial.println("failed to init chip, please check if the chip connection is correct. ");
        delay(1000);
    }
    /*!
     *@brief Set the value of pin sqw
     *@param mode eDS3231M_OFF             = 0x01 // Not output square wave, enter interrupt mode
     *@n          eDS3231M_SquareWave_1Hz  = 0x00 // 1Hz square wave
     *@n          eDS3231M_SquareWave_1kHz = 0x08 // 1kHz square wave
     *@n          eDS3231M_SquareWave_4kHz = 0x10 // 4kHz square wave
     *@n          eDS3231M_SquareWave_8kHz = 0x18 // 8kHz square wave
     */
    rtc.writeSqwPinMode(eDS3231M_OFF);
    rtc.enAbleAlarm1Int();
    //rtc.disAbleAlarm1Int();
    rtc.enAbleAlarm2Int();
    //rtc.disAbleAlarm2Int();
    /*!
     *@brief Set alarm clock 
     *@param alarmType Alarm clock working mode typedef enum{
     *@n                                  eEverySecond,
     *@n                                  eSecondsMatch,
     *@n                                  eSecondsMinutesMatch,
     *@n                                  eSecondsMinutesHoursMatch,
     *@n                                  eSecondsMinutesHoursDateMatch,
     *@n                                  eSecondsMinutesHoursDayMatch, //Alarm1
     *@n                                  eEveryMinute,
     *@n                                  eMinutesMatch,
     *@n                                  eMinutesHoursMatch,
     *@n                                  eMinutesHoursDateMatch,
     *@n                                  eMinutesHoursDayMatch,        //Alarm2
     *@n                                  eUnknownAlarm
     *@n                                  }eAlarmTypes;
     *@param days    Alarm clock (day)
     *@param hours   Alarm clock (hour)
     *@param mode:   e24hours, eAM, ePM
     *@param minutes Alarm clock (minute)
     *@param seconds Alarm clock (second)
     */
    rtc.setAlarm(eSecondsMatch,/*date,0-30*/29,/*hour,1-12 in 12hours,0-23 in 24hours*/9,e24hours,/*minute,0-59*/10,/*second,0-59*/40);
    
    rtc.setYear(19);//Set year, default in the 21st century. 
    rtc.setMonth(9);
    rtc.setDate(29);
    /*!
     *@brief Set the hours and 12hours or 24hours
     *@param hour:1-12 in 12hours,0-23 in 24hours
     *@param mode:e24hours, eAM, ePM
     */
    rtc.setHour(9,e24hours);
    rtc.setMinute(10);
    rtc.setSecond(30);
    rtc.adjust();
    attachInterrupt(0, interrupt, FALLING);
    
    /*!
     *@brief Set avr sleep mode
     */
    set_sleep_mode (SLEEP_MODE_PWR_DOWN);
    sleep_cpu();
    sleep_enable();
    
}
void loop() {
    /*!
     *@brief Judge if the alarm clock is triggered
     *@return true, triggered; false, not triggered
     */
    if(alarmFlag == 1){
        Serial.println("Alarm is up");
        rtc.clearAlarm();
        alarmFlag = 0;
        while (t < 10){
            rtc.getNowTime();
            Serial.print(rtc.year(), DEC);
            Serial.print('/');
            Serial.print(rtc.month(), DEC);
            Serial.print('/');
            Serial.print(rtc.day(), DEC);
            Serial.print(" (");
            Serial.print(rtc.getDayOfTheWeek());
            Serial.print(") ");
            Serial.print(rtc.hour(), DEC);
            Serial.print(':');
            Serial.print(rtc.minute(), DEC);
            Serial.print(':');
            Serial.print(rtc.second(), DEC);
            Serial.print(' ');
            /*if rtc works in 24hours mode,this function doesn't print anything*/
            Serial.print(rtc.getAMorPM());
            Serial.println();
            delay(1000);
            t = t + 1;
        }
        t = 0;
    }
    else
        delay(1000);
    /*!
     *@brief Judge if it is power-down 
     *@return if return true, power-down, time needs to reset; false, work well
     */
    if (rtc.lostPower()) {
        Serial.println("RTC lost power, please reset the time!");
    }
    sleep_enable();
    //energy.PowerDown();
}


void interrupt(){
    alarmFlag = 1;
    sleep_disable();
}
