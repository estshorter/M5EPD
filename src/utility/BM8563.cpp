#include "BM8563.h"

void BM8563::begin(void)
{
    // assume _wire.begin() is called
    writeReg(0x00, 0x00);
    writeReg(0x01, 0x00);
    writeReg(0x0D, 0x00);
}

void BM8563::writeReg(uint8_t reg, uint8_t data)
{
    _wire.beginTransmission(ADDR);
    _wire.write(reg);
    _wire.write(data);
    _wire.endTransmission();
}

uint8_t BM8563::readReg(uint8_t reg)
{
    _wire.beginTransmission(ADDR);
    _wire.write(reg);
    _wire.endTransmission();
    _wire.requestFrom(ADDR, 1);
    return _wire.read();
}

void BM8563::getDateTime(rtc_date_t &date, rtc_time_t &time)
{
    uint8_t buf[7] = {0};

    _wire.beginTransmission(ADDR);
    _wire.write(0x02);
    _wire.endTransmission();
    _wire.requestFrom(ADDR, 7);
    while (_wire.available())
    {
        buf[0] = _wire.read();
        buf[1] = _wire.read();
        buf[2] = _wire.read();
        buf[3] = _wire.read();
        buf[4] = _wire.read();
        buf[5] = _wire.read();
        buf[6] = _wire.read();
    }

    readTime(time, buf);
    readDate(date, &buf[3]);
}

void BM8563::setDateTime(const rtc_date_t &date, const rtc_time_t &time)
{
    _wire.beginTransmission(ADDR);
    _wire.write(0x02);
    writeTime(time);
    writeDate(date);
    _wire.endTransmission();
}

uint8_t BM8563::Bcd2ToByte(uint8_t Value)
{
    uint8_t tmp = 0;
    tmp = ((Value & 0xF0) >> 0x4) * 10;
    return (tmp + (Value & 0x0F));
}

uint8_t BM8563::ByteToBcd2(uint8_t Value)
{
    uint8_t bcdhigh = 0;

    while (Value >= 10)
    {
        bcdhigh++;
        Value -= 10;
    }

    return ((uint8_t)(bcdhigh << 4) | Value);
}

void BM8563::getTime(rtc_time_t &time)
{
    uint8_t buf[3] = {0};

    _wire.beginTransmission(ADDR);
    _wire.write(0x02);
    _wire.endTransmission();
    _wire.requestFrom(ADDR, 3);

    while (_wire.available())
    {
        buf[0] = _wire.read();
        buf[1] = _wire.read();
        buf[2] = _wire.read();
    }

    readTime(time, buf);
}

void BM8563::setTime(const rtc_time_t &time)
{
    _wire.beginTransmission(ADDR);
    _wire.write(0x02);
    writeTime(time);
    _wire.endTransmission();
}

void BM8563::getDate(rtc_date_t &date)
{

    uint8_t buf[4] = {0};

    _wire.beginTransmission(ADDR);
    _wire.write(0x05);
    _wire.endTransmission();
    _wire.requestFrom(ADDR, 4);

    while (_wire.available())
    {
        buf[0] = _wire.read();
        buf[1] = _wire.read();
        buf[2] = _wire.read();
        buf[3] = _wire.read();
    }

    readDate(date, buf);
}

void BM8563::setDate(const rtc_date_t &date)
{
    _wire.beginTransmission(ADDR);
    _wire.write(0x05);
    writeDate(date);
    _wire.endTransmission();
}

int BM8563::setAlarmIRQ(int afterSeconds)
{
    uint8_t reg_value = 0;
    reg_value = readReg(0x01);

    if (afterSeconds < 0)
    {
        reg_value &= ~(1 << 0);
        writeReg(0x01, reg_value);
        reg_value = 0x03;
        writeReg(0x0E, reg_value);
        return -1;
    }

    uint8_t type_value = 2;
    uint8_t div = 1;
    if (afterSeconds > 255)
    {
        div = 60;
        type_value = 0x83;
    }
    else
    {
        type_value = 0x82;
    }

    afterSeconds = (afterSeconds / div) & 0xFF;
    writeReg(0x0F, afterSeconds);
    writeReg(0x0E, type_value);

    reg_value |= (1 << 0);
    reg_value &= ~(1 << 7);
    writeReg(0x01, reg_value);
    return afterSeconds * div;
}

int BM8563::setAlarmIRQ(const rtc_time_t &time)
{
    uint8_t irq_enable = false;
    uint8_t out_buf[4] = {0x80, 0x80, 0x80, 0x80};

    if (time.min >= 0)
    {
        irq_enable = true;
        out_buf[0] = ByteToBcd2(time.min) & 0x7f;
    }

    if (time.hour >= 0)
    {
        irq_enable = true;
        out_buf[1] = ByteToBcd2(time.hour) & 0x3f;
    }

    out_buf[2] = 0x00;
    out_buf[3] = 0x00;

    uint8_t reg_value = readReg(0x01);

    if (irq_enable)
    {
        reg_value |= (1 << 1);
    }
    else
    {
        reg_value &= ~(1 << 1);
    }

    for (int i = 0; i < 4; i++)
    {
        writeReg(0x09 + i, out_buf[i]);
    }
    writeReg(0x01, reg_value);

    return irq_enable ? 1 : 0;
}

int BM8563::setAlarmIRQ(const rtc_date_t &date, const rtc_time_t &time)
{
    uint8_t irq_enable = false;
    uint8_t out_buf[4] = {0x80, 0x80, 0x80, 0x80};

    if (time.min >= 0)
    {
        irq_enable = true;
        out_buf[0] = ByteToBcd2(time.min) & 0x7f;
    }

    if (time.hour >= 0)
    {
        irq_enable = true;
        out_buf[1] = ByteToBcd2(time.hour) & 0x3f;
    }

    if (date.day >= 0)
    {
        irq_enable = true;
        out_buf[2] = ByteToBcd2(date.day) & 0x3f;
    }

    if (date.week >= 0)
    {
        irq_enable = true;
        out_buf[3] = ByteToBcd2(date.week) & 0x07;
    }

    uint8_t reg_value = readReg(0x01);

    if (irq_enable)
    {
        reg_value |= (1 << 1);
    }
    else
    {
        reg_value &= ~(1 << 1);
    }

    for (int i = 0; i < 4; i++)
    {
        writeReg(0x09 + i, out_buf[i]);
    }
    writeReg(0x01, reg_value);

    return irq_enable ? 1 : 0;
}

void BM8563::clearIRQ()
{
    uint8_t data = readReg(0x01);
    writeReg(0x01, data & 0xf3);
}

void BM8563::disableIRQ()
{
    clearIRQ();
    uint8_t data = readReg(0x01);
    writeReg(0x01, data & 0xfC);
}

void BM8563::writeDate(const rtc_date_t &date)
{
    _wire.write(ByteToBcd2(date.day));
    _wire.write(ByteToBcd2(date.week));

    if (date.year < 2000)
    {
        _wire.write(ByteToBcd2(date.mon) | 0x80);
    }
    else
    {
        _wire.write(ByteToBcd2(date.mon) | 0x00);
    }
    _wire.write(ByteToBcd2(date.year % 100));
}

void BM8563::writeTime(const rtc_time_t &time)
{
    _wire.write(ByteToBcd2(time.sec));
    _wire.write(ByteToBcd2(time.min));
    _wire.write(ByteToBcd2(time.hour));
}

void BM8563::readTime(rtc_time_t &time, uint8_t *buf)
{
    time.sec = Bcd2ToByte(buf[0] & 0x7f);
    time.min = Bcd2ToByte(buf[1] & 0x7f);
    time.hour = Bcd2ToByte(buf[2] & 0x3f);
}

void BM8563::readDate(rtc_date_t &date, uint8_t *buf)
{
    date.day = Bcd2ToByte(buf[0] & 0x3f);
    date.week = Bcd2ToByte(buf[1] & 0x07);
    date.mon = Bcd2ToByte(buf[2] & 0x1f);

    if (buf[2] & 0x80)
    {
        date.year = 1900 + Bcd2ToByte(buf[3] & 0xff);
    }
    else
    {
        date.year = 2000 + Bcd2ToByte(buf[3] & 0xff);
    }
}