//  File:   I2CInterface.h
//  Interface functions and definitions
//
//  This file is part of the Ember Front Panel firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Evan Davey  <http://www.ekidna.io/ember/>
//  Drew Beller
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

#include <ByteBuffer.h> //!< ByteBuffer library
#include <Wire.h> //!< I2C library
#include <avr/wdt.h> //!< Watchdog library
#include "Ring.h" //!< Ring class
#include "OLED.h" //!< Oled class
#include "interface-commands.h" //!< command definitions
#include "autodesk-font.cpp"

#define CMD_BUFFER_SIZE 300 //!< Size of I2C command buffer
#define FRAME_TIMEOUT 1000 //!< Full frame must be received in 1s



typedef enum InterfaceEvent {
    EventButton1Pressed = 0,
    EventButton1Held = 1,
    EventButton2Pressed = 2,
    EventButton2Held = 3,
    EventInterfaceProcessing = 4,
    //EventCommandReceived = 5,
    //EventAnimationEnd = 6,
    //EventError = 7
};

typedef enum InterfaceError {
    IFErrorNone = 0,
    IFErrorBufferFull = 1,
    IFErrorFrameTimeout = 2,
    IFErrorBadFrame = 3
};

/*
 * CRC update
 * Calculates an 8 bit CRC x^8+x^5+x^+1 (0x8C)
 */

uint8_t crc_update(uint8_t crc, uint8_t data) {

        uint8_t i;

        crc = crc ^ data;
        for (i = 0; i < 8; i++)
        {
            if (crc & 0x01)
                crc = (crc >> 1) ^ 0x8C;
            else
                crc >>= 1;
        }

        return crc;
}

/*
 * CRC check
 *
 */

boolean check_crc(ByteBuffer *data, uint8_t data_length) {
    uint8_t crc = 0, i;

    for (i = 0; i < data_length; i++) {
        crc = crc_update(crc, data->peek(i));
    }

    Log.debug("CRC is %02x",crc);

    if (crc) return false;
    return true;
}


class I2CInterface {

    uint8_t _int_pin;
    uint32_t _interrupt_start_time;
    boolean _interrupt;
    uint8_t _addr;
    uint8_t _registerAddr;
    uint8_t _register;
    int _frame_length;
    uint32_t _frame_start_time;
    InterfaceError _err;
    Ring *_ring;
    OLED *_oled;
    ByteBuffer cmd_buffer;

    // screen save
    bool sleeping;
    uint8_t last_sequence;
    uint8_t minutes; //number of minutes for screen to stay awake
    uint8_t sleep_sequence;

    unsigned long last_active_time;

    public:

        /**
         * I2CInterface Constructor
         *
         * \param addr I2C address
         * \param pin Interrupt pin number
         * \param *ring Ring class pointer
         * \param *oled Oled class pointer
         */
        I2CInterface(uint8_t addr,uint8_t pin, Ring *ring, OLED *oled) {
                cmd_buffer.init(CMD_BUFFER_SIZE);
                _int_pin = pin;
                _addr = addr;
                _interrupt = false;
                _frame_length = 0;
                _ring = ring;
                _oled = oled;
                _err = IFErrorNone;
                _register = 0x00;
                _registerAddr = REG_DISPLAY_STATUS;
                pinMode(_int_pin,OUTPUT);
                digitalWrite(_int_pin,LOW);
                last_active_time = millis();
                sleeping=false;
                last_sequence=10;//disables LEDs
                minutes=30;//by default screen saver goes on every 30 minutes
                sleep_sequence=8;//looping animation that fades LEDs on and off slowly
        }

        /**
         * Start interface
         */
        void begin() {
            Wire.begin(_addr);
            Log.debug(F("Interface: listening at 0x%2x"), _addr);
        }

        /**
         * I2C request interrupt handler
         */
        void request() {
            Wire.write(_register);
            if (_registerAddr == REG_BTN_STATUS) {
                _register = 0x00;
            }
        }


        /**
         * I2C receive interrupt handler
         */
        void receive(uint8_t num_bytes) {
            _registerAddr = Wire.read(); // register address
            //place I2C bytes into buffer
            for (int i=1; i<num_bytes; i++) {
                int c = Wire.read();
                if (!(cmd_buffer.put(c))) {
                    _err = IFErrorBufferFull;
                }
            }

            if (cmd_buffer.getSize()) process_event(EventInterfaceProcessing);

        }

        void dump_buffer() {
#if DEBUG
            Log.info("Buffer Dump");
            for (int i=0; i<cmd_buffer.getSize(); i++) {
                Serial.print(cmd_buffer.peek(i),HEX);
                Serial.print(',');
            }
            Serial.println("");
#endif
        }

        /**
         * Start interrupt
         */
        void start_interrupt() {
            if (!_interrupt) {
                digitalWrite(_int_pin,HIGH);
                _interrupt = true;
                _interrupt_start_time = millis();
            }
        }

        /**
         * Stop interrupt, non-blocking
         */
        void stop_interrupt() {
            if (_interrupt) {
                if (millis() >= (_interrupt_start_time+INTERFACE_INTERRUPT_TIME)) {
                    _interrupt = false;
                    digitalWrite(_int_pin,LOW);
                }
            }
        }

        /**
         * Process interface event
         *
         * \param event the event that occured
         */
        void process_event(InterfaceEvent event) {
            bitSet(_register,event);
        }

        /**
         * Clear interface event
         *
         * \param event the event that is cleared
         */
        void clear_event(InterfaceEvent event) {
            bitClear(_register,event);
        }
        /**
         * Listen for commands non-blocking
         */
        boolean listen() {
            if (_err) {
                Log.error("Interface Error %d", _err);
                _err = IFErrorNone; //clear error
            }
            if (check_for_frame()) {
                last_active_time = millis();
                if(sleeping){
                    _oled->Sleep(0);
                    _ring->start_animation(5);//fade active LEDs off
                    _ring->start_animation(last_sequence);
                }
                sleeping = false;
                process_commands();
                return true;
            }
            // after (minutes) put screen to sleep, if not already sleeping
            // (unless minutes == 0, i.e. screensaver disabled)
            // millis() runs a little slow so only 40000 ticks per minute instead of 60000
            // millis() overflows about every 50 days, which can trigger an early screen saver
            if ((minutes != 0) && (millis() - last_active_time > 40000 * minutes) && (!sleeping)){
                //screen saving procedure
                _oled->Sleep(1);
                if(last_sequence != 4){
                    // only change if special attention (sequence #4) isn't needed
                    _ring->enable_leds();//animate all LEDs
                    _ring->start_animation(sleep_sequence);
                }
                sleeping = true;
            }
            return false;
        }

        /**
         * Software reset using watchdog timer
         */
        void software_reset() {
            wdt_enable(WDTO_1S); //reset after 1 second
            while (true) {
                Log.info("Shutting down");
                delay(500);
            }
        }

        /**
         * Check command buffer for a frame
         * If frame found, process commands
         * \returns true if command frame received
         */
        boolean check_for_frame() {
            if (!_frame_length) {
                //Log.debug("Command buffer size: %d",cmd_buffer.getSize());
                if (cmd_buffer.getSize()>=2) {
                    if (cmd_buffer.get() == CMD_START) {
                        //new frame, assume first byte is frame length
                        _frame_length = (int) cmd_buffer.get();
                        _frame_start_time = millis();

                        Log.debug(F("Interface: Frame start"));
                        Log.debug(F("\tInterface: Expecting frame length: %d"),_frame_length);
                    }
                }
            } else {
                //Log.info("Buffer size is %d", cmd_buffer.getSize());
                if (cmd_buffer.getSize() >= _frame_length) {
                    //frame received
                    Log.debug(F("Interface: Frame end"));
                    //if (!check_crc(&cmd_buffer,_frame_length+1)) {
                    //        _err = IFErrorBadFrame;
                    //        Log.error("CRC check failed");
                    //}
                    dump_buffer();
                    if (cmd_buffer.peek(_frame_length) != CMD_END) {
                            _err = IFErrorBadFrame;
                            _frame_length=0;
                            return false;
                    }
                    _frame_length=0;
                    return true;
                }
                if ((millis() - _frame_start_time) > FRAME_TIMEOUT) {
                    _err = IFErrorFrameTimeout;
                    _frame_length=0;
                }
            }
            if (!cmd_buffer.getSize()) {
                clear_event(EventInterfaceProcessing);
            }
            return false;
        }


        /**
         * Process command frame
         */
        void process_commands() {
                uint8_t cmd = cmd_buffer.get();

                switch(cmd) {
                    case CMD_SYNC:
                        Log.debug(F("\tInterface: SYNC command"));
                        //interrupt
                        break;
                    case CMD_RESET:
                        Log.debug(F("\tInterface: RESET command"));
                        software_reset();
                        //interrupt
                        break;
                    case CMD_RING:
                        Log.debug(F("\tInterface: RING command"));
                        process_ring_command();
                        break;
                    case CMD_OLED:
                        Log.debug(F("\tInterface: OLED command"));
                        process_OLED_command();
                        break;
                    /*
                    *Sets the number of minutes the screen will stay on untill sleeping
                    *0 will disable the sleep function
                    */
                    case CMD_SLEEP:
                        Log.debug(F("\tInterface: SLEEP command"));
                        minutes = (int) cmd_buffer.get();
                        break;
                    default:
                        break;
                        //invalid data
                }

                cmd_buffer.get(); //strip frame end
        }

        /**
         * Process a ring command
         */
        void process_ring_command() {
            uint8_t cmd = cmd_buffer.get();

            if (cmd==CMD_RING_LED) {
                uint8_t led = (int) cmd_buffer.get();
                uint8_t hi = cmd_buffer.get();
                uint8_t lo = cmd_buffer.get();
                uint16_t pwm = hi << 8;
                pwm |= lo;
                _ring->set_led(led,pwm);
                Log.debug(F("\tInterface: Set led %d to %lu"),led,pwm);
                return;
            }
            if (cmd==CMD_RING_LEDS) {
                uint8_t hi = cmd_buffer.get();
                uint8_t lo = cmd_buffer.get();
                uint16_t pwm = hi << 8;
                pwm |= lo;
                _ring->set_leds(pwm);
                last_sequence = 10; // turn leds off  after sleep
                Log.debug(F("\tInterface: Set leds to %lu"),pwm);
                return;
            }

            if (cmd==CMD_RING_OFF) {
                _ring->off();
                last_sequence = 10; // turn leds off  after sleep
                Log.debug(F("\tInterface: Ring off"));
            }

            if (cmd==CMD_RING_SEQUENCE) {
                uint8_t sequence= (int) cmd_buffer.get();
                _ring->start_animation(sequence);
                last_sequence = sequence;
                if(last_sequence==0||last_sequence==9){
                    last_sequence=10;  // turn leds off  after sleep
                }
                Log.debug(F("\tInterface: Start animation %d"),sequence);
                return;
            }
        }

        /**
         * Process an OLED command
         */
        void process_OLED_command() {
            uint8_t cmd = cmd_buffer.get();

            if (cmd == CMD_OLED_TEXT || cmd ==  CMD_OLED_CENTERTEXT 
                    || cmd == CMD_OLED_RIGHTTEXT) {
                uint8_t x = (int) cmd_buffer.get();
                uint8_t y = (int) cmd_buffer.get();
                uint8_t size = (int) cmd_buffer.get();
                uint16_t color = ((int) cmd_buffer.get() << 8);
                color |= (int) cmd_buffer.get();
                uint8_t txtLength = (int) cmd_buffer.get();

                char txt[txtLength+1];
                int i;
                for (i=0;i<txtLength;i++) {
                    txt[i] = (char) cmd_buffer.get();
                }
                txt[i] = '\0';

                //_oled->setTextSize(size);
                //_oled->setTextColor(color);
                //_oled->setCursor(x-1,y-1); //API uses 1 index, x=0 center text
                TextAlign align = TextCenter;

                if (cmd==CMD_OLED_TEXT) align = TextLeft;
                if (cmd==CMD_OLED_RIGHTTEXT) align = TextRight;

                _oled->SetText(txt,x-1,y-1,size,align,color);
                Log.debug(F("\tInterface: Oled set text: %s"),txt);
                Log.debug(F("\t\tx:%d y:%d size:%d, color: %u"),x,y,size,color);
                return;

            }

            if (cmd == CMD_OLED_CLEAR) {
                _oled->Clear();
                Log.debug(F("\tInterface: Oled clear"));
                return;
            }

            if (cmd == CMD_OLED_OFF) {
                _oled->Off();
                Log.debug(F("\tInterface: Oled off"));
                return;
            }

            if (cmd == CMD_OLED_ON) {
                _oled->On();
                Log.debug(F("\tOled on"));
                return;
            }

            if (cmd == CMD_OLED_LOGO) {
                _oled->DrawLogo();
                Log.debug(F("\tOled logo"));
                return;
            }

            if (cmd == CMD_OLED_SETPIXEL) {
                Log.debug(F("\tInterface: Oled set pixel"));
                uint8_t x = (int) cmd_buffer.get();
                uint8_t y = (int) cmd_buffer.get();
                uint8_t hi = cmd_buffer.get();
                uint8_t lo = cmd_buffer.get();
                uint16_t color = hi << 8;
                color |= lo;
                _oled->DrawPixel(color,x,y);
                return;
            }
        }
        /*
        *Returns true if screen is awake
        *Returns false if screen is asleep, and wakes screen
        */
        bool WakeScreen() {
            bool retVal = true;
            if (sleeping){
                sleeping = false;
                _oled->Sleep(0);
                _ring->start_animation(5);//fade active LEDs off
                _ring->start_animation(last_sequence);
                retVal = false;
            }
            last_active_time = millis();
            return retVal;
        }
};


