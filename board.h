/*
    Copyright (c) 2012 <benemorius@gmail.com>

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use,
    copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following
    conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.
*/

#define DELAY50MS() for(uint32_t i = 0; i < 416635; i++)
#define DELAY250MS() for(uint32_t i = 0; i < 2083175; i++)

#define DEBOUNCE_DELAY (20)

#define REFERENCE_VOLTAGE (3.0f)

extern "C" char* get_stack_top(void);
extern "C" char* get_heap_end(void);

#define SD_PRESENT_PORTNUM (0)
#define SD_PRESENT_PINNUM (5)

#define SPI0_MOSI_PORT (0)
#define SPI0_MOSI_PIN (18)
#define SPI0_MISO_PORT (0)
#define SPI0_MISO_PIN (17)
#define SPI0_SCK_PORT (0)
#define SPI0_SCK_PIN (15)
#define SPI0_CLOCKRATE (100000)

#define SPI1_MOSI_PORT (0)
#define SPI1_MOSI_PIN (9)
#define SPI1_MISO_PORT (0)
#define SPI1_MISO_PIN (8)
#define SPI1_SCK_PORT (0)
#define SPI1_SCK_PIN (7)
#define SPI1_CLOCKRATE (100000)

#define LCD_SELECT_PORT (2)
#define LCD_SELECT_PIN (7)
#define LCD_REFRESH_PORT (1)
#define LCD_REFRESH_PIN (28)
#define LCD_UNK0_PORT (2)
#define LCD_UNK0_PIN (5)
#define LCD_UNK1_PORT (2)
#define LCD_UNK1_PIN (6)
#define LCD_RESET_PORT (0)
#define LCD_RESET_PIN (22)

#define DEBUG_TX_PORTNUM (0)
#define DEBUG_TX_PINNUM (2)
#define DEBUG_RX_PORTNUM (0)
#define DEBUG_RX_PINNUM (3)
#define DEBUG_BAUD (115200)


#define KLINE_TX_PORTNUM (4)
#define KLINE_TX_PINNUM (28)
#define KLINE_RX_PORTNUM (4)
#define KLINE_RX_PINNUM (29)
#define KLINE_BAUD (9600)

#define LLINE_TX_PORTNUM (2)
#define LLINE_TX_PINNUM (8)
#define LLINE_RX_PORTNUM (2)
#define LLINE_RX_PINNUM (9)
#define LLINE_BAUD (9600)


#define LCD_BACKLIGHT_PORT (1)
#define LCD_BACKLIGHT_PIN (23)

#define CLOCK_BACKLIGHT_PORT (3)
#define CLOCK_BACKLIGHT_PIN (25)

#define LCD_BIASCLOCK_PORT (1)
#define LCD_BIASCLOCK_PIN (26)

#define KEYPAD_BACKLIGHT_PORT (3)
#define KEYPAD_BACKLIGHT_PIN (26)

#define OUT_RESET_PORT (0)
#define OUT_RESET_PIN (19)
#define OUT0_CS_PORT (0)
#define OUT0_CS_PIN (20)
#define OUT1_CS_PORT (2)
#define OUT1_CS_PIN (3)

#define RUN_PORT (2)
#define RUN_PIN (11)

#define SDCARD_DETECT_PORT (1)
#define SDCARD_DETECT_PIN (17)
#define SDCARD_CS_PORT (1)
#define SDCARD_CS_PIN (16)

#define CCM_DATA_PORT (1)
#define CCM_DATA_PIN (9)
#define CCM_CLOCK_PORT (1)
#define CCM_CLOCK_PIN (4)
#define CCM_LATCH_PORT (1)
#define CCM_LATCH_PIN (8)

#define FUEL_LEVEL_PORT (0)
#define FUEL_LEVEL_PIN (6)

#define STALK_BUTTON_PORT (2)
#define STALK_BUTTON_PIN (10)

#define SPEED_PORT (0)
#define SPEED_PIN (4)

#define FUEL_CONS_PORT (0)
#define FUEL_CONS_PIN (5)

#define BATTERY_VOLTAGE_PORT (0)
#define BATTERY_VOLTAGE_PIN (25)

#define EXT_TEMP_PORT (0)
#define EXT_TEMP_PIN (23)
