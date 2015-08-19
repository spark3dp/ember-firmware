/**
 * Developed by Electric Echidna Ltd 
 *
 * With optimizations by Drew Beller
 */

#ifndef __OLED_H__
#define __OLED_H__

#include <SSD1351.h>
#include "logo.cpp"

#include "autodesk-font.cpp" //!< custom font size 1
#include "autodesk-font2.cpp" //!< custom font size 2

const uint8_t max_lines = 10; // max lines (+1) that can be fast cleared

class OLED : public SSD1351OLED {

    public:
        // last drawn bounds 
        uint8_t last_min_x;
        uint8_t last_max_y;
        uint8_t last_max_x;
        uint8_t last_min_y;
        // last items (lines or bitmaps) drawn 
        // 0xFF indicates max bounding box needs clearing
        uint8_t drawn;
        uint8_t lengths[max_lines];
        uint8_t xs[max_lines];
        uint8_t ys[max_lines];
        FontConfig fonts[max_lines];

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

        /**
         * Clear either the maximum bounding box
         * or the last objects drawn (if less than max_lines)
         */

        void Clear() {
            if ((drawn == 0xFF)||(drawn >= max_lines)){
                FillBlock(BLACK, last_min_x, last_min_y, last_max_x - last_min_x, last_max_y - last_min_y);
                last_min_x = SSD1351_WIDTH;
                last_max_y = 0;
                last_max_x = 0;
                last_min_y = SSD1351_HEIGHT;
            }else{
                for (int i = 0; i< drawn; i++){
                    FillBlock(BLACK, xs[i]+1, ys[i]+1, lengths[i], fonts[i].height);
                }
            }
            drawn = 0;
        }

        void DrawLogo() {
            Off(); //can be removed to save about 500ms
            uint8_t x = (SSD1351_WIDTH / 2) - (LOGO_WIDTH / 2);
            uint8_t y = (SSD1351_HEIGHT / 2) - (LOGO_HEIGHT / 2);
            DrawBitmapProgmem(logo,LOGO_WIDTH,LOGO_HEIGHT, x,y);
            //the screen is now fast enough that drawing the logo is instant
            FadeOn();//can be removed to save about 500ms

            if (x < last_min_x){
                last_min_x = x;
            }if (y < last_min_y){
                last_min_y = y;
            }if (x + LOGO_WIDTH > last_max_x){
                last_max_x = x + LOGO_WIDTH + 5;
            }if (y + LOGO_HEIGHT > last_max_y){
                last_max_y = y + LOGO_HEIGHT + 5;
            }
            drawn = 0xFF;
        }


        //set the brightness of the screen (1-16)
        void SetBrightness(uint8_t b){
            SetMasterCurrent(b);
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
                x = x - (length / 2);
            }
            if (align == TextRight) {
                x = x - length;
            }
            if (x > SSD1351_WIDTH){//prevents wrap around errors
                x = SSD1351_WIDTH - x;
                last_min_x = 0;
                last_max_x = SSD1351_WIDTH;
                last_min_y = 0;
                last_max_y = SSD1351_HEIGHT;
            }if(y > SSD1351_HEIGHT){
                y = SSD1351_HEIGHT - y;
                last_min_x = 0;
                last_max_x = SSD1351_WIDTH;
                last_min_y = 0;
                last_max_y = SSD1351_HEIGHT;
            }

            DrawString(s,x,y,font,color);

            if (x < last_min_x){
                last_min_x = x;
            }if (align == TextLeft){
                x = 0;
                last_min_x = 0;
            }if (y < last_min_y){
                last_min_y = y;
            }if (x+length > last_max_x){
                last_max_x = x + length;
            }if (y + font.height > last_max_y){
                last_max_y = y + font.height;
            }if (last_max_x > SSD1351_WIDTH || last_max_y > SSD1351_HEIGHT || 
                 x > SSD1351_WIDTH || y > SSD1351_HEIGHT){
                //prevent wrap around errors, by clearing the whole screen
                last_min_x = 0;
                last_max_x = SSD1351_WIDTH;
                last_min_y = 0;
                last_max_y = SSD1351_HEIGHT;
                drawn = 0xFF;
            }

            if(drawn < max_lines){
                lengths[drawn] = length;
                xs[drawn] = x;
                ys[drawn] = y;
                fonts[drawn] = font;
                drawn++;
            }
        }
};

#endif
