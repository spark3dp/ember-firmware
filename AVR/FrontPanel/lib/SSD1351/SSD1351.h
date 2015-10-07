//  File:   SSD1351.h
//  Electric Echidna SSD1351 Library
//
//  This file is part of the Ember Front Panel firmware.
//
//  Copyright 2015 Ekidna (Electric Echidna Ltd) <http://www.ekidna.io/ember/>
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//
//  Authors:
//  Evan Davey 
//  Drew Beller
//  Richard Greene
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

/**
 * @file SSD1351.h
 * @brief Electric Echidna SSD1351 Library
 * @copyright (c) 2014, Electric Echidna Ltd All Rights Reserved
 *
 * Used under license.  Refer to your Electric Echidna project manager
 *
 * With optimizations by Drew Beller
 */
#ifndef __SSD1351_H__
#define __SSD1351_H__

#include <SPI.h>


// Color definitions
#define	BLACK       0x0000
#define	BLUE        0x001F
#define	RED         0xF800
#define	GREEN       0x07E0
#define CYAN        0x07FF
#define MAGENTA     0xF81F
#define YELLOW      0xFFE0
#define WHITE      0xFFFF

#define SSD1351_WIDTH 128
#define SSD1351_HEIGHT 128

#define SSD1351_BRIGHTNESS 0x0F

#define OLED_BACKGROUND_COLOR BLACK //!<Background colour

typedef enum TextAlign {
    TextLeft,
    TextCenter,
    TextRight
};

typedef struct {
    uint8_t width;
    uint8_t height;
    char start_char;
    char end_char;
    const unsigned char *font_table;
} FontConfig;


class SSD1351OLED {

    uint8_t _dc;
    uint8_t _cs;
    uint8_t _res;

    public:

        SSD1351OLED(uint8_t cs, uint8_t dc, uint8_t res) {
            _cs = cs;
            _dc = dc;
            _res = res;

            pinMode(_cs,OUTPUT);
            pinMode(_res,OUTPUT);
            pinMode(_dc,OUTPUT);

        }

        void WriteCommand(uint8_t Data) {
            digitalWrite(_dc,LOW);
            digitalWrite(_cs,LOW);
            SPI.transfer(Data);
            digitalWrite(_cs,HIGH);
        }

        void WriteData(uint8_t Data) {
            digitalWrite(_dc,HIGH);
            digitalWrite(_cs,LOW);
            SPI.transfer(Data);
            digitalWrite(_cs,HIGH);
        }

        void Reset() {
            digitalWrite(_res,HIGH);
            delay(500);
            digitalWrite(_res,LOW);
            delay(500);
            digitalWrite(_res,HIGH);
            delay(500);
        }

        /*
         * 0x12 => Driver IC interface is unlocked from entering command.
         * 0x16 => All Commands are locked except 0xFD.
         * 0xB0 => Command 0xA2, 0xB1, 0xB3, 0xBB & 0xBE are inaccessible.
         * 0xB1 => All Commands are accessible.
         */

        void SetCommandLock(uint8_t d) {
            WriteCommand(0xFD);
            WriteData(d);
        }

        /**
         * Set Display Clock Divider / Oscillator Frequency
         * A[3:0] => Display Clock Divider
         * A[3:0] => Display Clock Divider
         *
         */

        void SetDisplayClock(uint8_t d) {
            WriteCommand(0xB3);
            WriteData(d);
        }

        void SetDisplayOff() {
            WriteCommand(0xAE);
        }

        void SetDisplayOn() {
            WriteCommand(0xAF);
        }

        // Set Segment Low Voltage
        //     0xA2 (0x02) => Enable Internal VSL (Kept VSL Pin N.C.)
        //     0xA0 (0x00) => Enable External VSL
        void SetVSL(uint8_t d) {
            WriteCommand(0xB4);
            WriteData(0xA0|d);
            WriteData(0xB5);
            WriteData(0x55);
        }

        // General Purpose IO
        // Default => 0x0A (GPIO Pins output Low Level.)
        // 0x0F => (GPIO Pins output High Level.)
        void SetGPIO(uint8_t d) {
            WriteCommand(0xB5);
            WriteData(d);
        }

        // Set Second Pre-Charge Period
        // Default => 0x08 (8 Display Clocks)
        void SetPrechargePeriod(uint8_t d) {
            WriteCommand(0xB6);
            WriteData(d);
        }

        // Set Pre-Charge Voltage Level
        // Default => 0x17 (0.50*VCC)
        void SetPrechargeVoltage(uint8_t d) {
            WriteCommand(0xBB);
            WriteData(d);
        }

        // Set COM Deselect Voltage Level
        //   Default => 0x05 (0.82*VCC)
        void SetVCOMH(uint8_t d)
        {
            WriteCommand(0xBE);
            WriteData(d);
        }

        // Set Contrast Current for Color A, B, C
        // Default => 0x8A (Color A)
        // Default => 0x51 (Color B)
        // Default => 0x8A (Color C)
        void SetContrastColor(uint8_t a, uint8_t b, uint8_t c) {
            WriteCommand(0xC1);
            WriteData(a);
            WriteData(b);
            WriteData(c);
        }

		// Master Contrast Current Control
        // Default => 0x0F (Maximum)
        void SetMasterCurrent(uint8_t d) {
            WriteCommand(0xC7);
            WriteData(d);
        }

		// Set Multiplex Ratio
        // Default => 0x7F (1/128 Duty)
        void SetMultiplexRatio(uint8_t d) {
            WriteCommand(0xCA);
            WriteData(d);
        }


        void SetPhaseLength(uint8_t d) {
            WriteCommand(0xB1);			// Phase 1 (Reset) & Phase 2 (Pre-Charge) Period Adjustment
            WriteData(d);				//   Default => 0x82 (8 Display Clocks [Phase 2] / 5 Display Clocks [Phase 1])
            //     D[3:0] => Phase 1 Period in 5~31 Display Clocks
            //     D[7:4] => Phase 2 Period in 3~15 Display Clocks
        }

        // Display Enhancemet
        // Default => 0x00 (Normal)nt

        void SetDisplayEnhancement(uint8_t d) {
            WriteCommand(0xB2);
            WriteData(d);
            WriteData(0x00);
            WriteData(0x00);
        }

        // Set Column Address
        void SetColumnAddress(uint8_t a, uint8_t b) {
            WriteCommand(0x15);
            //Write_Data(0x75-b);				//   Default => 0x7F (End Address)
            //Write_Data(0x7F-a);				//   Default => 0x00 (Start Address)
            WriteData(a);				//   Default => 0x00 (Start Address)
            WriteData(b);				//   Default => 0x7F (End Address)
        }

        // Set Row Address
        void SetRowAddress(uint8_t a, uint8_t b) {
            WriteCommand(0x75);
            //Write_Data(0x7F-b);				//   Default => 0x7F (End Address)
            //Write_Data(0x7F-a);				//   Default => 0x00 (Start Address)
            WriteData(a);				//   Default => 0x00 (Start Address)
            WriteData(b);				//   Default => 0x7F (End Address)
        }


		// Enable MCU to Write into RAM
        void SetWriteRAM() {
            WriteCommand(0x5C);
        }

		// Enable MCU to Read from RAM
        void SetReadRAM() {
            WriteCommand(0x5D);
        }

        // Set Re-Map / Color Depth
        // Horizontal Address Increment
        // Column Address 0 Mapped to SEG0
        // Color Sequence: A => B => C
        // Scan from COM0 to COM[N-1]
        // Disable COM Split Odd Even
        // 65,536 Colors
        void SetRemapFormat(uint8_t d) {
            WriteCommand(0xA0);
            WriteData(d);				//   Default => 0x40
        }

        // Set Vertical Scroll by RAM
        void SetStartLine(uint8_t d) {
            WriteCommand(0xA1);
            WriteData(d);				//   Default => 0x00
        }

        // Set Vertical Scroll by Row
        void SetDisplayOffset(uint8_t d) {
            WriteCommand(0xA2);
            WriteData(d);				//   Default => 0x60
        }

        // Set Display Mode
        // 0xA4 (0x00) => Entire Display Off, All Pixels Turn Off
        // 0xA5 (0x01) => Entire Display On, All Pixels Turn On at GS Level 63
        // 0xA6 (0x02) => Normal Display
        // 0xA7 (0x03) => Inverse Display
        void SetDisplayMode(uint8_t d) {
            WriteCommand(0xA4|d);
            //   Default => 0xA6
        }

        // Function Selection
        // Enable Internal VDD Regulator
        // Select 8-bit Parallel Interface
        void SetFunctionSelection(uint8_t d) {
            WriteCommand(0xAB);
            WriteData(d);				//   Default => 0x01
        }


        void SetLinearGrayScaleTable() {
            WriteCommand(0xB9);			// Default
        }

        //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        //  Gray Scale Table Setting (Full Screen)
        //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        void SetGrayScaleTable()
        {
            WriteCommand(0xB8);
            WriteData(0x02);			// Gray Scale Level 1
            WriteData(0x03);			// Gray Scale Level 2
            WriteData(0x04);			// Gray Scale Level 3
            WriteData(0x05);			// Gray Scale Level 4
            WriteData(0x06);			// Gray Scale Level 5
            WriteData(0x07);			// Gray Scale Level 6
            WriteData(0x08);			// Gray Scale Level 7
            WriteData(0x09);			// Gray Scale Level 8
            WriteData(0x0A);			// Gray Scale Level 9
            WriteData(0x0B);			// Gray Scale Level 10
            WriteData(0x0C);			// Gray Scale Level 11
            WriteData(0x0D);			// Gray Scale Level 12
            WriteData(0x0E);			// Gray Scale Level 13
            WriteData(0x0F);			// Gray Scale Level 14
            WriteData(0x10);			// Gray Scale Level 15
            WriteData(0x11);			// Gray Scale Level 16
            WriteData(0x12);			// Gray Scale Level 17
            WriteData(0x13);			// Gray Scale Level 18
            WriteData(0x15);			// Gray Scale Level 19
            WriteData(0x17);			// Gray Scale Level 20
            WriteData(0x19);			// Gray Scale Level 21
            WriteData(0x1B);			// Gray Scale Level 22
            WriteData(0x1D);			// Gray Scale Level 23
            WriteData(0x1F);			// Gray Scale Level 24
            WriteData(0x21);			// Gray Scale Level 25
            WriteData(0x23);			// Gray Scale Level 26
            WriteData(0x25);			// Gray Scale Level 27
            WriteData(0x27);			// Gray Scale Level 28
            WriteData(0x2A);			// Gray Scale Level 29
            WriteData(0x2D);			// Gray Scale Level 30
            WriteData(0x30);			// Gray Scale Level 31
            WriteData(0x33);			// Gray Scale Level 32
            WriteData(0x36);			// Gray Scale Level 33
            WriteData(0x39);			// Gray Scale Level 34
            WriteData(0x3C);			// Gray Scale Level 35
            WriteData(0x3F);			// Gray Scale Level 36
            WriteData(0x42);			// Gray Scale Level 37
            WriteData(0x45);			// Gray Scale Level 38
            WriteData(0x48);			// Gray Scale Level 39
            WriteData(0x4C);			// Gray Scale Level 40
            WriteData(0x50);			// Gray Scale Level 41
            WriteData(0x54);			// Gray Scale Level 42
            WriteData(0x58);			// Gray Scale Level 43
            WriteData(0x5C);			// Gray Scale Level 44
            WriteData(0x60);			// Gray Scale Level 45
            WriteData(0x64);			// Gray Scale Level 46
            WriteData(0x68);			// Gray Scale Level 47
            WriteData(0x6C);			// Gray Scale Level 48
            WriteData(0x70);			// Gray Scale Level 49
            WriteData(0x74);			// Gray Scale Level 50
            WriteData(0x78);			// Gray Scale Level 51
            WriteData(0x7D);			// Gray Scale Level 52
            WriteData(0x82);			// Gray Scale Level 53
            WriteData(0x87);			// Gray Scale Level 54
            WriteData(0x8C);			// Gray Scale Level 55
            WriteData(0x91);			// Gray Scale Level 56
            WriteData(0x96);			// Gray Scale Level 57
            WriteData(0x9B);			// Gray Scale Level 58
            WriteData(0xA0);			// Gray Scale Level 59
            WriteData(0xA5);			// Gray Scale Level 60
            WriteData(0xAA);			// Gray Scale Level 61
            WriteData(0xAF);			// Gray Scale Level 62
            WriteData(0xB4);			// Gray Scale Level 63
        }

        void DrawPixel(uint16_t color, uint8_t x, uint8_t y) {
            //SetColumnAddress(x,0);
            //SetRowAddress(y,0);
            SetColumnAddress(SSD1351_WIDTH-x-2,SSD1351_WIDTH-x-2); //invert x
            SetRowAddress(SSD1351_HEIGHT-y-2,SSD1351_HEIGHT-y-2); //invert y
            SetWriteRAM();
            WriteData(color>>8);
            WriteData(color);
        }

        //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        //  Show Regular Pattern ()
        //
        //    color: RRRRRGGGGGGBBBBB
        //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        void FillBlock(uint16_t color, uint8_t x, uint8_t y, uint8_t w, uint8_t h) {

            // Bounds check
            if ((x >= SSD1351_WIDTH) || (y >= SSD1351_HEIGHT)) return;

            // Y bounds check
            if (y+h > SSD1351_HEIGHT) h = SSD1351_HEIGHT - y - 1;

            // X bounds check
            if (x+w > SSD1351_WIDTH) w = SSD1351_WIDTH - x - 1;

            //SetColumnAddress(x,x+w-1);
            //SetRowAddress(y,y+h-1);
            SetColumnAddress(SSD1351_WIDTH-(w+x),SSD1351_WIDTH-1-x); //invert x
            SetRowAddress(SSD1351_HEIGHT-(h+y),SSD1351_HEIGHT-1-y); //invert y
            SetWriteRAM();
            uint16_t area =w*h;
            digitalWrite(_dc,HIGH);
            digitalWrite(_cs,LOW);
            for(uint16_t i=0;i< area;i++) {
                SPI.transfer(color>>8);
                SPI.transfer(color);
            }
            digitalWrite(_cs,HIGH);
        }

        void FillScreen(uint16_t color) {
            FillBlock(color,0,0,SSD1351_WIDTH,SSD1351_HEIGHT);
        }

        void Init() {
            SPI.begin();
            SPI.setDataMode(SPI_MODE3);
            SPI.setClockDivider(SPI_CLOCK_DIV2); //4Mhz
            Reset();
            ScreenInit();
        }

        void ScreenInit() {

            SetCommandLock(0x12);			// Unlock Driver IC (0x12/0x16/0xB0/0xB1)
            SetCommandLock(0xB1);			// Unlock All Commands (0x12/0x16/0xB0/0xB1)
            SetDisplayOff();
            SetDisplayClock(0xF1);		// Set Clock as 90 Frames/Sec
            SetMultiplexRatio(0x7F);		// 1/128 Duty (0x0F~0x7F)
            SetDisplayOffset(0x00);		// Shift Mapping RAM Counter (0x00~0x7F)
            SetStartLine(0x00);			// Set Mapping RAM Display Start Line (0x00~0x7F)
            SetRemapFormat(0x75);			// Set Vertical Address Increment
            //     Column Address 0 Mapped to SEG0
            //     Color Sequence D[15:0]=[RRRRR:GGGGGG:BBBBB]
            //     Scan from COM127 to COM0
            //     Enable COM Split Odd Even
            //     65,536 Colors Mode (0x74)
            //     * 262,144 Colors Mode (0xB4)
            SetGPIO(0x00);				// Disable GPIO Pins Input
            SetFunctionSelection(0x01);		// Enable Internal VDD Regulator
            SetVSL(0xA0);				// Enable External VSL
            SetContrastColor(0xC8,0x80,0xC8);	// Set Contrast of Color A (Red), B (Green), C (Blue)
            SetMasterCurrent(SSD1351_BRIGHTNESS);		// Set Scale Factor of Segment Output Current Control
            //SetGrayScaleTable();			// Set Pulse Width for Gray Scale Table
            SetPhaseLength(0x32);			// Set Phase 1 as 5 Clocks & Phase 2 as 3 Clocks
            //Set_Precharge_Voltage(0x17);		// Set Pre-Charge Voltage Level as 0.50*VCC
            SetPrechargeVoltage(0x32);		// Set Pre-Charge Voltage Level as 0.50*VCC
            SetDisplayEnhancement(0xA4);		// Enhance Display Performance
            SetPrechargePeriod(0x01);		// Set Second Pre-Charge Period as 1 Clock
            SetVCOMH(0x05);			// Set Common Pins Deselect Voltage Level as 0.82*VCC
            SetDisplayMode(0x02);			// Normal Display Mode (0x00/0x01/0x02/0x03)
            FillScreen(BLACK);			// Clear Screen
            SetDisplayOn();
        }

        //Using writeRAM method
        void DrawBitmapProgmem(const unsigned char *bitmap, int width, int height, int at_x, int at_y)
        {
            SetRemapFormat(0x74);           // Set Horizontal Address Increment
            SetColumnAddress(SSD1351_WIDTH-width-at_x+1,SSD1351_WIDTH-at_x); //invert  x 
            SetRowAddress(SSD1351_HEIGHT-height-at_y,SSD1351_HEIGHT-at_y);
            SetWriteRAM();
            digitalWrite(_dc,HIGH);
            digitalWrite(_cs,LOW);
            uint8_t i=width;
            uint8_t j;
            while(i-- > 0){//invert bitmap
                j=height;
                const unsigned char *i_location = bitmap + (i * width * sizeof(uint16_t));
                while(j-- > 0){//invert bitmap
                    uint16_t pixel = pgm_read_word(i_location+j*sizeof(uint16_t));
                    SPI.transfer(pixel>>8);
                    SPI.transfer(pixel);
                }
            }
            digitalWrite(_cs,HIGH);
            SetRemapFormat(0x75);           // Set Vertical Address Increment
        }


        /*
         * Calculate string length when using proportional font
         */

        uint8_t ProportionalFontStringLength(char *s, FontConfig font) {
            uint8_t len = 0;
            uint8_t bytes_high = font.height / 8 + 1;
            uint8_t bytes_per_char = font.width * bytes_high + 1;
            unsigned const char *p;
            while (*s) {
                p = font.font_table + (*s - font.start_char) * bytes_per_char;
                len+= pgm_read_byte(p);
                len++; //space
                s++;
            }
            return len;
        }

        /**
         * Draw a string using a custom font
         */

        void DrawString(char *s, uint8_t x, uint8_t y,  FontConfig font,
                uint16_t color = WHITE ) {
            uint8_t width;
            if (font.height==14){//if font 1
                while (*s) {
                    width = DrawCharRAM(*s,x,y,font,color);
                    x += (width + 1);
                    s++;
                }
            }else{
                while (*s) {
                    width = DrawChar(*s,x,y,font,color);
                    x += (width + 1);
                    s++;
                }
            }
        }

        /**
         * Draw a character using a custom font
         */
        uint8_t DrawChar(char c, uint8_t x, uint8_t y, FontConfig font,
                uint16_t color) {

            if (c < font.start_char ) c = font.start_char;
            if (c > font.end_char ) c = font.end_char;

            uint8_t bytes_high = font.height / 8 + 1;
            uint8_t bytes_per_char = font.width * bytes_high + 1;
            uint8_t i;
            uint8_t var_width;
            unsigned const char *p;
            p = font.font_table + (c - font.start_char) * bytes_per_char;
            var_width = pgm_read_byte(p);
            p++;
            for ( i = 0; i < var_width; i++ ) {
                uint8_t j;
                uint8_t new_x = x + i;
                for ( j = 0; j < bytes_high; j++ ) {
                    uint8_t dat = pgm_read_byte( p + i * bytes_high  + j );
                    uint8_t bit;
                    uint8_t new_y = y + j * 8;
                    for (bit = 0; bit < 8; bit++) {
                        if (dat & (1<<bit)) {
                            DrawPixel(color, new_x, new_y + bit);
                        } 
                    }
                }
            }
            return var_width;
        }

        //Only works with vertical address increment mode
        //doesn't work for Font2 for unknown reasons (Font2 might use horizontal increment mode)
        uint8_t DrawCharRAM(char c, uint8_t x, uint8_t y,
                FontConfig font, uint16_t color) {
            
            if (c < font.start_char ) c = font.start_char;
            if (c > font.end_char ) c = font.end_char;
            
            uint8_t bytes_high = font.height / 8 + 1;
            uint8_t bytes_per_char = font.width * bytes_high + 1;
            uint8_t i;
            uint8_t var_width;
            unsigned const char *p;
            p = font.font_table + (c - font.start_char) * bytes_per_char;
            var_width = pgm_read_byte(p);
            p++;
            SetColumnAddress(SSD1351_WIDTH-var_width-x-1,SSD1351_WIDTH-x); //invert  x 
            SetRowAddress(SSD1351_HEIGHT-font.height-y-2,SSD1351_HEIGHT-y-1); //invert y
            SetWriteRAM();
            uint8_t j;
            i=var_width;
            digitalWrite(_dc,HIGH);
            digitalWrite(_cs,LOW);
            while(i-- > 0){//invert bitmap
                j=bytes_high;
                while(j-- > 0){//invert bitmap
                    uint8_t dat = pgm_read_byte( p + i*bytes_high + j );
                    uint8_t bit;
                    bit=8;
                    uint16_t pixel;
                    while(bit-- > 0){
                        if (dat & (1<<bit)) {
                            pixel = color;
                        } else {
                            pixel = BLACK;
                        }
                        SPI.transfer(pixel>>8);
                        SPI.transfer(pixel);
                    }
                }
            }
            digitalWrite(_cs,HIGH);
            return var_width;
        }

        void FadeOn(uint16_t time_ms) {
            SetDisplayOn();
            uint16_t d = time_ms / SSD1351_BRIGHTNESS;
            for (uint8_t i=0; i<SSD1351_BRIGHTNESS+1; i++) {
                SetMasterCurrent(i);
                delay(d);
            }
        }

        void FadeOff(uint16_t time_ms) {
            uint16_t d = time_ms / SSD1351_BRIGHTNESS;
            for (uint8_t i=SSD1351_BRIGHTNESS; i>0; i++) {
                SetMasterCurrent(i);
                delay(d);
            }
            SetDisplayOff();
        }

        //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        //  Sleep Mode
        //
        //    "1" Enter Sleep Mode
        //    "0" Exit Sleep Mode
        //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        void Sleep(uint8_t a) {
            switch(a) {
                case 1:
                    SetDisplayOff();
                    break;
                case 0:
                    SetDisplayOn();
                    break;
            }
        }


        //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        //  Vertical Scrolling (Full Screen)
        //
        //    a: Scrolling Direction
        //       "0x00" (Upward)
        //       "0x01" (Downward)
        //    b: Set Numbers of Row Scroll per Step
        //    c: Set Time Interval between Each Scroll Step
        //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        void VerticalScroll(uint8_t a, uint8_t b, uint8_t c) {
            unsigned int i,j;

            switch(a) {
                case 0:
                    for(i=0;i<SSD1351_HEIGHT;i+=b) {
                        SetStartLine(i);
                        delay(c);
                    }
                    break;
                case 1:
                    for(i=0;i<SSD1351_HEIGHT;i+=b) {
                        SetStartLine(SSD1351_HEIGHT-i);
                        for(j=0;j<c;j++) {
                            delay(c);
                        }
                    }
                    break;
            }
            SetStartLine(0x00);
        }

        //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        //  Automatic Horizontal Scrolling (Instruction)
        //
        //    a: Scrolling Direction
        //       "0x00" (Rightward)
        //       "0x01" (Leftward)
        //    b: Set Numbers of Column Scroll per Step
        //    c: Set Row Address of Start
        //    d: Set Numbers of Row to Be Scrolled
        //    e: Set Time Interval between Each Scroll Step in Terms of Frame Frequency
        //    f: Delay Time
        //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        void HorizontalScroll(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f) {
            WriteCommand(0x96);			// Horizontal Scroll Setup
            WriteData((a<<7)|b);
            WriteData(c);
            WriteData(d);
            WriteData(0x00);
            WriteData(e);
            WriteCommand(0x9F);			// Activate Horizontal Scroll
            delay(f);
        }

};


#endif
