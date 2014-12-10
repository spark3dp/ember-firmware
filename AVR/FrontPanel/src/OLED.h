#ifndef __OLED_H__
#define __OLED_H__

#include <SSD1351.h>
#include "logo.cpp"

#include "autodesk-font.cpp" //!< custom font size 1
#include "autodesk-font2.cpp" //!< custom font size 2

class OLED : public SSD1351OLED {

    public:
        OLED(uint8_t cs, uint8_t dc, uint8_t rst) :
            SSD1351OLED(cs,dc,rst) {};

        void On() {
            SetDisplayOn();
        }

        void Off() {
            SetDisplayOff();
        }

        void FadeOn() {
            SSD1351OLED::FadeOn(1000);
        }
        void FadeOff() {
            SSD1351OLED::FadeOff(1000);
        }

        void Clear() {
            FillScreen(BLACK);
        }

        void DrawLogo() {
            Off();
            FillScreen(BLACK);
            DrawBitmapProgmem(logo,LOGO_WIDTH,LOGO_HEIGHT, (SSD1351_WIDTH/2)-(LOGO_WIDTH/2),
                    (SSD1351_HEIGHT/2)-(LOGO_HEIGHT/2));
            FadeOn();
        }

        /**
         * Set OLED text at x and y
         * Size determines the font
         * Center = true to center around x
         * TODO: cleanup
         */

        void SetText(char *s, uint8_t x, uint8_t y, uint8_t size, TextAlign align = TextCenter, uint16_t color = WHITE) {

            FontConfig font;


            switch(size) {
                case 2:
                    font.font_table = adfont2;
                    font.width = FONT2_WIDTH;
                    font.height = FONT2_HEIGHT;
                    font.start_char = FONT2_START_CHAR;
                    font.end_char = FONT2_END_CHAR;
                    break;
                default:
                    font.font_table = adfont1;
                    font.width = FONT1_WIDTH;
                    font.height = FONT1_HEIGHT;
                    font.start_char = FONT1_START_CHAR;
                    font.end_char = FONT1_END_CHAR;
                    break;

            }
            uint8_t length = ProportionalFontStringLength(s,font);
            if (align == TextCenter) {
                x = x - (length/2);
            }
            if (align == TextRight) {
                x = x - length;
            }
            FillBlock(BLACK,x,y,length*font.width,font.height);
            DrawString(s,x,y,font,color);
        }

};

#endif
