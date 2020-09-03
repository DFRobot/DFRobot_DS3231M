/*!
 * @file DFRobot_DS3231M.cpp
 * @brief Define the basic structure of class DFRobot_DS3231M
 * @copyright	Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @licence     The MIT License (MIT)
 * @author [yufeng](yufeng.luo@dfrobot.com)
 * @version  V1.0
 * @date  2019-08-19
 * @get from https://www.dfrobot.com
 * @url https://github.com/DFRobot/DFRobot_DS3231M
 */

#include <DFRobot_DS3231M.h>
const uint8_t daysInMonth [] PROGMEM={31,28,31,30,31,30,31,31,30,31,30,31};

static uint16_t date2days(uint16_t y, uint8_t m, uint8_t d) {
    if (y >= 2000)
        y -= 2000;                              // Remove year offset
    uint16_t days = d;                          // Store numbers of days
    for (uint8_t i = 1; i < m; ++i){
        days += pgm_read_byte(daysInMonth + i - 1); // Add number of days for each month
    } if (m > 2 && y % 4 == 0)
        ++days;                                 // Deal with leap years
    return days + 365 * y + (y + 3) / 4 - 1;    // Return computed value
}

static uint8_t conv2d(const char* p) {
    uint8_t v = 0;
    if ('0' <= *p && *p <= '9')
        v = *p - '0';
    return 10 * v + *++p - '0';
}

DFRobot_DS3231M::~DFRobot_DS3231M() {}

bool DFRobot_DS3231M::begin(void)
{
    Wire.begin();
    delay(100);
    Wire.beginTransmission(_deviceAddr);
    if(Wire.endTransmission() == 0)
        return true;
    else
        return false;
}

uint8_t DFRobot_DS3231M::bcd2bin(uint8_t val){
    return val - 6 * (val >> 4);
}

uint8_t DFRobot_DS3231M::bin2bcd (uint8_t val){
    return val + 6 * (val / 10);
}

eDs3231MSqwPinMode_t DFRobot_DS3231M::readSqwPinMode(){
    int mode[1];
    readReg(DS3231M_REG_CONTROL, mode, 1);
    mode[0] &= 0x93;
    return static_cast<eDs3231MSqwPinMode_t>(mode[0]);
}

void DFRobot_DS3231M::writeSqwPinMode(eDs3231MSqwPinMode_t mode){
    uint8_t ctrl[1];
    readReg(DS3231M_REG_CONTROL, ctrl, 1);
    ctrl[0] &= ~0x04;
    ctrl[0] &= ~0x18;
    if (mode == eDS3231M_OFF) 
        ctrl[0] |= 0x04;
    else
        ctrl[0] |= mode;
    writeReg(DS3231M_REG_CONTROL, ctrl, 1);
}
/*
void DFRobot_DS3231M::dateTime (const char* date, const char* time)
{
  y = conv2d(date + 9); // Compute the year offset
  switch (date[0]) 
  {
    case 'J': m = (date[1] == 'a') ? 1 : ((date[2] == 'n') ? 6 : 7); break; // Jan Jun Jul
    case 'F': m = 2; break;                                                 // Feb
    case 'A': m = date[2] == 'r' ? 4 : 8; break;                            // Apr Aug
    case 'M': m = date[2] == 'r' ? 3 : 5; break;                            // May
    case 'S': m = 9; break;                                                 // Sep
    case 'O': m = 10; break;                                                // Oct
    case 'N': m = 11; break;                                                // Nov
    case 'D': m = 12; break;                                                // Dec
  } // of switch for the month
  d = conv2d(date + 4);
  hh = conv2d(time);
  mm = conv2d(time + 3);
  ss = conv2d(time + 6);
}
*/
void DFRobot_DS3231M::dateTime (){
    char buff[11];
    memcpy_P(buff, F(__DATE__), 11);
    y = conv2d(buff + 9) + 30;
    // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
    switch (buff[0]) {
        case 'J': m = (buff[1] == 'a') ? 1 : ((buff[2] == 'n') ? 6 : 7); break;
        case 'F': m = 2; break;
        case 'A': m = buff[2] == 'r' ? 4 : 8; break;
        case 'M': m = buff[2] == 'r' ? 3 : 5; break;
        case 'S': m = 9; break;
        case 'O': m = 10; break;
        case 'N': m = 11; break;
        case 'D': m = 12; break;
    }
    d = conv2d(buff + 4);
    memcpy_P(buff, F(__TIME__), 8);
    hh = bin2bcd(conv2d(buff));
    mm = conv2d(buff + 3);
    ss = conv2d(buff + 6);
}

uint8_t DFRobot_DS3231M::dayOfTheWeek() const {
  uint16_t day = date2days(_y, _m, _d); // compute the number of days
  return (day + 6) % 7;                 // Jan 1, 2000 is a Saturday
} 

const char* DFRobot_DS3231M::getDayOfTheWeek(){
    return daysOfTheWeek[dayOfTheWeek()];
}

void DFRobot_DS3231M::setHour(uint8_t hour, ehours mode){
    hh = (mode << 5|bin2bcd(hour));
}

const char* DFRobot_DS3231M::getAMorPM(){
    uint8_t buffer[1];
    readReg(DS3231M_REG_RTC_HOUR, buffer, 1);
    buffer[0] = buffer[0] << 1;
    buffer[0] = buffer[0] >> 6;
    return hourOfAM[buffer[0]];
}

void DFRobot_DS3231M::adjust(){
    uint8_t buffer[] = {bin2bcd(ss),bin2bcd(mm),hh,dayOfTheWeek(),bin2bcd(d),bin2bcd(m),bin2bcd(y)};
    //dateTime(F(__DATE__), F(__TIME__));
    writeReg(DS3231M_REG_RTC_SEC, buffer, 7);
    uint8_t statreg[1];
    readReg(DS3231M_REG_STATUS, statreg, 1);
    statreg[0] &= ~0x80; // flip OSF bit
    writeReg(DS3231M_REG_STATUS, statreg, 1);
}

void DFRobot_DS3231M::getNowTime(){
    readReg(DS3231M_REG_RTC_SEC, bcd, 7);
    _ss = bcd2bin(bcd[0] & 0x7F);
    _mm = bcd2bin(bcd[1]);
    if(bcd[2]&0x40){
        bcd[2] = bcd[2] << 3;
        _hh = bcd2bin(bcd[2] >> 3);
    }
    else{
        bcd[2] = bcd[2] << 2;
        _hh = bcd2bin(bcd[2] >> 2);
    }
    _d = bcd2bin(bcd[4]);
    _m = bcd2bin(bcd[5]);
    _y = bcd2bin(bcd[6]) + 1970;
    if(bcd[5] > 80){
        _y += 100;
        _m -= 80;
    }
}

float DFRobot_DS3231M::getTemperatureC(){
    uint8_t buf[2];
    readReg(DS3231M_REG_TEMPERATURE, buf, 2);
    return ((float)buf[0] + (buf[1]>>6)*0.25f);
}

bool DFRobot_DS3231M::lostPower(void) {
    uint8_t status[1];
    readReg(DS3231M_REG_STATUS, status, 1);
    return status[0] >> 7;
}

void DFRobot_DS3231M::setAlarm(eAlarmTypes alarmType, int16_t date,int8_t hour, ehours mode,
                               int8_t minute,int8_t second, const bool state ){
    int16_t dates[] = {bin2bcd(date)};
    int8_t hours[] = {mode << 5|bin2bcd(hour)};
    int8_t minutes[] = {bin2bcd(minute)};
    int8_t seconds[] = {bin2bcd(second)};
    uint8_t days[] = {bin2bcd(dayOfTheWeek())};
    uint8_t buffer[1];
    if (alarmType >= eUnknownAlarm)
        return;
    if (alarmType < eEveryMinute){
        writeReg(DS3231M_REG_ALM1_SEC, seconds, 1);
        writeReg(DS3231M_REG_ALM1_MIN, minutes, 1);
        writeReg(DS3231M_REG_ALM1_HOUR, hours, 1);
        if (alarmType == eSecondsMinutesHoursDateMatch)
            writeReg(DS3231M_REG_ALM1_DAY, dates, 1);
        else
            writeReg(DS3231M_REG_ALM1_DAY, days, 1);
        if(alarmType<eSecondsMinutesHoursDateMatch){
            readReg(DS3231M_REG_ALM1_DAY, buffer, 1);
            buffer[0] |= 0x80;
            writeReg(DS3231M_REG_ALM1_DAY, buffer, 1);
        }
        if(alarmType<eSecondsMinutesHoursMatch){
            readReg(DS3231M_REG_ALM1_HOUR, buffer, 1);
            buffer[0] |= 0x80;
            writeReg(DS3231M_REG_ALM1_HOUR, buffer, 1);
        }
        if(alarmType<eSecondsMinutesMatch){
            readReg(DS3231M_REG_ALM1_MIN, buffer, 1);
            buffer[0] |= 0x80;
            writeReg(DS3231M_REG_ALM1_MIN, buffer, 1);
        }
        if(alarmType==eEverySecond){
            readReg(DS3231M_REG_ALM1_SEC, buffer, 1);
            buffer[0] |= 0x80;
            writeReg(DS3231M_REG_ALM1_SEC, buffer, 1);
        }
        if(alarmType==eSecondsMinutesHoursDayMatch){
            readReg(DS3231M_REG_ALM1_DAY, buffer, 1);
            buffer[0] |= 0x40;
            writeReg(DS3231M_REG_ALM1_DAY, buffer, 1);
        }
        if (state){
            readReg(DS3231M_REG_CONTROL, buffer, 1);
            buffer[0] |= 1;
            writeReg(DS3231M_REG_CONTROL, buffer, 1);
        }
        else{
            readReg(DS3231M_REG_CONTROL, buffer, 1);
            buffer[0] &= 0xFE;
            writeReg(DS3231M_REG_CONTROL, buffer, 1);
        }
    }
    else{
        writeReg(DS3231M_REG_ALM2_MIN, minutes, 1);
        writeReg(DS3231M_REG_ALM2_HOUR, hours, 1);
        if(alarmType == eMinutesHoursDateMatch)
            writeReg(DS3231M_REG_ALM2_DAY, dates, 1);
        else if (alarmType == eMinutesHoursDayMatch){
            days[0] |= 0x80;
            writeReg(DS3231M_REG_ALM2_DAY, days, 1);
        }
        if(alarmType < eMinutesHoursDateMatch){
            readReg(DS3231M_REG_ALM2_DAY, buffer, 1);
            buffer[0] |= 0x80;
            writeReg(DS3231M_REG_ALM2_DAY, buffer, 1);
        }
        if(alarmType < eMinutesHoursMatch){
            readReg(DS3231M_REG_ALM2_HOUR, buffer, 1);
            buffer[0] |= 0x80;
            writeReg(DS3231M_REG_ALM2_HOUR, buffer, 1);
        }
        if(alarmType == eEveryMinute){
            readReg(DS3231M_REG_ALM2_MIN, buffer, 1);
            buffer[0] |= 0x80;
            writeReg(DS3231M_REG_ALM2_MIN, buffer, 1);
        }
        if (state){
            readReg(DS3231M_REG_CONTROL, buffer, 1);
            buffer[0] |= 2;
            writeReg(DS3231M_REG_CONTROL, buffer, 1);
        }
        else{
            readReg(DS3231M_REG_CONTROL, buffer, 1);
            buffer[0] &= 0xFD;
            writeReg(DS3231M_REG_CONTROL, buffer, 1);
        }
    } // of if-then-else use alarm 1 or 2
    clearAlarm(); // Clear the alarm state
    return;
}

void DFRobot_DS3231M::enAbleAlarm1Int(){
    uint8_t crtl[1];
    readReg(DS3231M_REG_CONTROL, crtl, 1);
    crtl[0] |= 0x01;
    writeReg(DS3231M_REG_CONTROL, crtl, 1);
}

void DFRobot_DS3231M::disAbleAlarm1Int(){
    uint8_t crtl[1];
    readReg(DS3231M_REG_CONTROL, crtl, 1);
    crtl[0] &= 0xFE;
    writeReg(DS3231M_REG_CONTROL, crtl, 1);
}

void DFRobot_DS3231M::enAbleAlarm2Int(){
    uint8_t crtl[1];
    readReg(DS3231M_REG_CONTROL, crtl, 1);
    crtl[0] |= 0x02;
    writeReg(DS3231M_REG_CONTROL, crtl, 1);
}

void DFRobot_DS3231M::disAbleAlarm2Int(){
    uint8_t crtl[1];
    readReg(DS3231M_REG_CONTROL, crtl, 1);
    crtl[0] &= 0xFD;
    writeReg(DS3231M_REG_CONTROL, crtl, 1);
}

bool DFRobot_DS3231M::isAlarm() {
    uint8_t status[1];
    readReg(DS3231M_REG_STATUS, status, 1);
    return status[0]&3; // Alarm if either of 2 LSBits set
}

void DFRobot_DS3231M::clearAlarm(){
    uint8_t status[1];
    readReg(DS3231M_REG_STATUS, status, 1);
    status[0] &= 0xFC;
    writeReg(DS3231M_REG_STATUS, status, 1);
} 

void DFRobot_DS3231M::enAble32k(){
    uint8_t status[1];
    readReg(DS3231M_REG_STATUS, status, 1);
    status[0] |= 0x08;
    writeReg(DS3231M_REG_STATUS, status, 1);
}

void DFRobot_DS3231M::disAble32k(){
    uint8_t status[1];
    readReg(DS3231M_REG_STATUS, status, 1);
    status[0] &= 0xF7;
    writeReg(DS3231M_REG_STATUS, status, 1);
}

void DFRobot_DS3231M::writeReg(uint8_t reg, const void* pBuf, size_t size)
{
    if(pBuf == NULL){
        DBG("pBuf ERROR!! : null pointer");
    }
    uint8_t * _pBuf = (uint8_t *)pBuf;
    _pWire->beginTransmission(_deviceAddr);
    _pWire->write(&reg, 1);
    
    for(uint16_t i = 0; i < size; i++){
        _pWire->write(_pBuf[i]);
    }
    _pWire->endTransmission();
}

uint8_t DFRobot_DS3231M::readReg(uint8_t reg, const void* pBuf, size_t size)
{
    if(pBuf == NULL){
        DBG("pBuf ERROR!! : null pointer");
    }
    uint8_t * _pBuf = (uint8_t *)pBuf;
    _pWire->beginTransmission(_deviceAddr);
    _pWire->write(&reg, 1);
    
    if( _pWire->endTransmission() != 0){
        return 0;
    }

    _pWire->requestFrom(_deviceAddr, (uint8_t) size);
    for(uint16_t i = 0; i < size; i++){
        _pBuf[i] = _pWire->read();
    }
    return size;
}
