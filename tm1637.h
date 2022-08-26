#ifndef TM1637_H
#define TM1637_H

#ifdef RASPBERRY_PI
#include <wiringPi.h>
#endif
#include <stdint.h>
#include <stdbool.h>

#define TM1637_I2C_COMM1    0x40
#define TM1637_I2C_COMM2    0xC0
#define TM1637_I2C_COMM3    0x80
#define BITDELAY 100 // pause between writes in microseconds

//      A
//     ---
//  F |   | B
//     -G-
//  E |   | C
//     ---
//      D

#ifndef RASPBERRY_PI
// pour éviter les erreurs, on définit des trucs à la con
uint8_t pinMode(uint8_t a,uint8_t b);
uint8_t delayMicroseconds(uint8_t a);
uint8_t digitalWrite(uint8_t a,int b);
uint8_t digitalRead(uint8_t a);
uint8_t pullUpDnControl(uint8_t a, uint8_t b);
void  wiringPiSetupGpio();
#define OUTPUT 1
#define INPUT 1
#define LOW 0
#define PUD_UP 0
#define PUD_DOWN 1
#endif

class tm1637
{
public:
    tm1637();
    void TMsetup(uint8_t pinClk, uint8_t pinDIO);

    void TMsetBrightness(uint8_t brightness);

    void TMsetSegments(const uint8_t segments[], uint8_t length, uint8_t pos);

    void TMclear();

    void TMshowNumber(int num, uint8_t dots, bool leading_zero, uint8_t length, uint8_t pos);

    void TMshowDouble(double x);

private:
    const uint8_t digitToSegment[16] = {
            // XGFEDCBA
            0b00111111,    // 0
            0b00000110,    // 1
            0b01011011,    // 2
            0b01001111,    // 3
            0b01100110,    // 4
            0b01101101,    // 5
            0b01111101,    // 6
            0b00000111,    // 7
            0b01111111,    // 8
            0b01101111,    // 9
            0b01110111,    // A
            0b01111100,    // b
            0b00111001,    // C
            0b01011110,    // d
            0b01111001,    // E
            0b01110001     // F
            };

    const uint8_t blank[4] = {0,0,0,0};

    uint8_t m_pinClock2;
    uint8_t m_pinDIO;
    uint8_t m_brightness;

    void TMstartWrite();

    void TMstopWrite();
    void TMwriteByte(uint8_t b);
};

#endif // TM1637_H
