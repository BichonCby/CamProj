#include "tm1637.h"

#ifndef RASPBERRY_PI
// pour éviter les erreurs, on définit des trucs à la con
uint8_t pinMode(uint8_t a,uint8_t b) {return a+b;}
uint8_t delayMicroseconds(uint8_t a){return a;}
uint8_t digitalWrite(uint8_t a,int b){return a+b;}
uint8_t digitalRead(uint8_t a){return a;}
uint8_t pullUpDnControl(uint8_t a, uint8_t b){return a+b;}
void  wiringPiSetupGpio(){return;}
/*#define OUTPUT 1
#define INPUT 1
#define LOW 0
#define PUD_UP 0
#define PUD_DOWN 1*/
#endif
tm1637::tm1637()
{

}

void tm1637::TMstartWrite()
    {
    pinMode(m_pinDIO, OUTPUT);
    delayMicroseconds(BITDELAY);
    }

void tm1637::TMstopWrite()
    {
    pinMode(m_pinDIO, OUTPUT);
    delayMicroseconds(BITDELAY);
    pinMode(m_pinClock2, INPUT);
    delayMicroseconds(BITDELAY);
    pinMode(m_pinDIO, INPUT);
    delayMicroseconds(BITDELAY);
    }

void tm1637::TMwriteByte(uint8_t b)
    {
    uint8_t data = b;

    // 8 Data Bits
    for(uint8_t i = 0; i < 8; i++)
        {
        // CLK low
        pinMode(m_pinClock2, OUTPUT);
        delayMicroseconds(BITDELAY);

        // Set data bit
        if (data & 0x01)
          pinMode(m_pinDIO, INPUT);
        else
          pinMode(m_pinDIO, OUTPUT);

        delayMicroseconds(BITDELAY);

        // CLK high
        pinMode(m_pinClock2, INPUT);
        delayMicroseconds(BITDELAY);
        data = data >> 1;
        }

    // Wait for acknowledge
    // CLK to zero
    pinMode(m_pinClock2, OUTPUT);
    pinMode(m_pinDIO, INPUT);
    delayMicroseconds(BITDELAY);

    // CLK to high
    pinMode(m_pinClock2, INPUT);
    delayMicroseconds(BITDELAY);
    if (digitalRead(m_pinDIO) == 0) pinMode(m_pinDIO, OUTPUT);
    delayMicroseconds(BITDELAY);
    pinMode(m_pinClock2, OUTPUT);
    delayMicroseconds(BITDELAY);
    }

void tm1637::TMsetup(uint8_t pinClk, uint8_t pinDIO)
  //! Initialize a TMsetup object, setting the clock and data pins
  //! (uses wiringpi numbering scheme : https://pinout.xyz/pinout/wiringpi#)
  //! @param pinClk : digital pin connected to the clock pin of the module
  //! @param pinDIO : digital pin connected to the DIO pin of the module
    {
   // wiringPiSetup();

    // Copy the pin numbers
    m_pinClock2 = pinClk;
    m_pinDIO = pinDIO;

    // Set the pin direction and default value.
    // Both pins are set as inputs, allowing the pull-up resistors to pull them up
    pinMode(m_pinClock2, INPUT);
    pinMode(m_pinDIO,INPUT);
    digitalWrite(m_pinClock2, LOW);
    digitalWrite(m_pinDIO, LOW);
    }

void tm1637::TMsetBrightness(uint8_t brightness)
  //! Sets the brightness of the display.
  //!
  //! Takes effect when a command is given to change the data being displayed.
  //!
  //! @param brightness A number from 0 (lower brightness) to 7 (highest brightness)
    {
    m_brightness = ((brightness & 0x7) | 0x08) & 0x0f;
    }

void tm1637::TMsetSegments(const uint8_t segments[], uint8_t length, uint8_t pos)
  //! Display arbitrary data on the module
  //!
  //! This function receives raw segment values as input and displays them. The segment data
  //! is given as a byte array, each byte corresponding to a single digit. Within each byte,
  //! bit 0 is segment A, bit 1 is segment B etc.
  //! The function may either set the entire display or any desirable part on its own. The first
  //! digit is given by the @ref pos argument with 0 being the leftmost digit. The @ref length
  //! argument is the number of digits to be set. Other digits are not affected.
  //!
  //! @param segments An array of size @ref length containing the raw segment values
  //! @param length The number of digits to be modified
  //! @param pos The position from which to start the modification (0 - leftmost, 3 - rightmost)
    {
    // Write COMM1
    TMstartWrite();
    TMwriteByte(TM1637_I2C_COMM1);
    TMstopWrite();

    // Write COMM2 + first digit address
    TMstartWrite();
    TMwriteByte(TM1637_I2C_COMM2 + (pos & 0x03));

    // Write the data bytes
    for (uint8_t k=0; k < length; k++)
      TMwriteByte(segments[k]);

    TMstopWrite();

    // Write COMM3 + brightness
    TMstartWrite();
    TMwriteByte(TM1637_I2C_COMM3 + m_brightness);
    TMstopWrite();
    }

void tm1637::TMclear()
    {
    TMsetSegments(blank,4,0);
    }

void tm1637::TMshowNumber(int num, uint8_t dots, bool leading_zero, uint8_t length, uint8_t pos)
  //! Displays a decimal number, with dot control
  //!
  //! Displays the given argument as a decimal number. The dots between the digits (or colon)
  //! can be individually controlled
  //!
  //! @param num The number to be shown
  //! @param dots Dot/Colon enable. The argument is a bitmask, with each bit corresponding to a dot
  //!        between the digits (or colon mark, as implemented by each module). i.e.
  //!        For displays with dots between each digit:
  //!        * 0000 (0)
  //!        * 0.000 (0b10000000)
  //!        * 00.00 (0b01000000)
  //!        * 000.0 (0b00100000)
  //!        * 0.0.0.0 (0b11100000)
  //!        For displays with just a colon:
  //!        * 00:00 (0b01000000)
  //!        For displays with dots and colons colon:
  //!        * 0.0:0.0 (0b11100000)
  //! @param leading_zero When true, leading zeros are displayed. Otherwise unnecessary digits are
  //!        blank
  //! @param length The number of digits to set
  //! @param pos The position least significant digit (0 - leftmost, 3 - rightmost)
    {
    uint8_t digits[4];
    const static int divisors[] = { 1, 10, 100, 1000 };
    bool leading = true;

    for(int8_t k = 0; k < 4; k++)
        {
        int divisor = divisors[4 - 1 - k];
        int d = num / divisor;
        uint8_t digit = 0;

        if (d == 0)
            {
            if (leading_zero || !leading || (k == 3)) digit = digitToSegment[d];
            else digit = 0;
            }
        else
            {
            digit = digitToSegment[d];
            num -= d * divisor;
            leading = false;
            }

        // Add the decimal point/colon to the digit
        digit |= (dots & 0x80);
        dots <<= 1;
        digits[k] = digit;
        }

    TMsetSegments(digits + (4 - length), length, pos);
    }

void tm1637::TMshowDouble(double x)
    //! Displays a double as 00.00
    {
    const uint8_t
        minus[] =     {64,64,64,64},
        zeropoint[] = {0B10111111};
    int x100;
    if (x>99) x=99.99;
    x100=x*(x<0 ? -1000 : 1000);// round and abs
    x100=x100/10+(x100%10 > 4);	//
    if (x100<100)
        {
        TMsetSegments(zeropoint,1,1);
        TMshowNumber(x100,0b1000000,true,2,2);
        TMsetSegments(x<0 ? minus : blank , 1, 0);
        }
    else if (x<0)
        {
        TMsetSegments(minus, 1, 0);
        TMshowNumber(x100,0b1000000,false,3,1);
        }
    else TMshowNumber(x100,0b1000000,false,4,0);
    }
