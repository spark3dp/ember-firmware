//  File:   Ring.h
//  Defines the front panel LED ring behaviors
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

#ifndef __RING_H__
#define __RING_H__

#include <avr/interrupt.h>

#include "variables.h"
#include "hardware.h"


class Ring {

    private:

        uint8_t _num_leds;
        uint8_t* _led_numbering;
        uint8_t* _disabled_leds;
        uint16_t* _led_buffer;

        uint32_t _animation_step;
        uint8_t _animation;

        uint8_t _oe;
        uint8_t _latch;
        uint8_t _sdin;
        uint8_t _sclk;

        unsigned char clockSelectBits;
        char oldSREG;					// To hold Status Register while ints disabled


    public:


        /**
         * Ring constructor
         * \param oe output enable pin number
         * \param latch latch pin number
         * \param sdin data in pin number
         * \param sclk clock pin number
         * \param numLeds number of used leds (21)
         * \param led_numbering array of size numLeds mapping led numbers to 
         *   output pins
         *
         *  Sets ring internal variables and sets up hardware
         */
        Ring(uint8_t oe, uint8_t latch, uint8_t sdin, uint8_t sclk,uint8_t numLeds, uint8_t *led_numbering) {
            _num_leds = numLeds;
            _led_numbering = led_numbering;
            _led_buffer = (uint16_t *) calloc(sizeof(uint16_t),24); //24 outputs
            _disabled_leds = (uint8_t *) calloc(sizeof(uint8_t),_num_leds);
            _oe = oe;
            _latch = latch;
            _sdin = sdin;
            _sclk = sclk;
            pinMode(_oe,OUTPUT);
            pinMode(_latch,OUTPUT);
            pinMode(_sdin, OUTPUT);
            pinMode(_sclk,OUTPUT);
            digitalWrite(_latch,LOW);
            disable();
        }

        /**
         * Enable LED output
         */
        void enable() {
            digitalWrite(_oe,LOW);
        }

        /**
         * Disable LED output
         */
        void disable() {
            digitalWrite(_oe,HIGH);
        }

        /**
         * Shift out led buffer MSB first, 12 bits per led, 24 outputs
         */
        void write() {
            digitalWrite(_latch,LOW);
            for (int8_t i = 24 - 1; i >= 0; i--) {
                for (int8_t b = 11; b >= 0; b--) {
                    digitalWrite(_sclk,LOW);
                    if (_led_buffer[i] & (1 << b)) {
                        digitalWrite(_sdin,HIGH);
                    } else {
                        digitalWrite(_sdin,LOW);
                    }
                    digitalWrite(_sclk,HIGH);
                }
            }
            digitalWrite(_sclk,LOW);
            digitalWrite(_latch,HIGH);
            digitalWrite(_latch,LOW);
        }

        /**
         * Set specified led to pwm value in the buffer. Follow by a write
         * operation to show on ring.
         *
         * \param led led number (0..num_leds)
         * \param pwm pwm value (0..4095)
         */
        void set_led_buffer(uint8_t led,uint16_t pwm) {
            if (_disabled_leds[led]) return;
            _led_buffer[_led_numbering[led]] = pwm;
        }

        /**
         * Set specified led to pwm value
         * \param led led number (0..num_leds)
         * \param pwm pwm value (0..4095)
         */
        void set_led(uint8_t led,uint16_t pwm) {
            set_led_buffer(led,pwm);
            write();
        }

        /**
         * Get specified led's pwm value
         * \param led led number (0..num_leds)
         */
        uint16_t get_led(uint8_t led) {
            return _led_buffer[_led_numbering[led]];
        }

        /**
         * Set all leds to given pwm value
         * \param pwm pwm value (0..4095)
         */
        void set_leds(uint16_t pwm) {
            for(uint8_t led = 0; led < _num_leds; led++) {
                set_led_buffer(led,pwm);
            }
            write();
        }

        /**
         * All leds on
         */
        void on() {
            set_leds(RING_ON_PWM);
        }

        /**
         * All leds off
         */
        void off() {
            set_leds(0);
        }

        void enable_leds() {
            memset(_disabled_leds,0,_num_leds);
        }

        void disable_leds() {
            for (uint8_t led = 0; led < _num_leds; led++) {
                if (get_led(led) == 0) {
                    _disabled_leds[led]=1;
                }
            }
        }

        /**
         * Stop current running animation
         */

        void stop_animation() {
            _animation = 0;
            enable_leds();
            Timer1.stop();
        }

         /**
         * Start led animation
         * \param animation animation number (0..8)
         *
         *  Define variables in variable.h
         *
         *  0: stop playing animation
         *  
         *  1: loop clockwise (loop time = A1_LOOP_MS) 
         *  
         *  2: loop clockwise (loop time = A2_LOOP_MS) 
         *  
         *  3: fade on in A3_FADE_ON_MS ms with A3_FADE_ON_STEP, pause for
         *  A3_ON_MS then off clockwise in A3_OFF_MS ms 
         *
         *  4: loop two leds in opposite directions (loop time = A4_LOOP_TIME_MS) 
         *
         *  5: fade on in A5_FADE_ON_MS ms with step A5_FADE_ON_STEP
         *  stay on for A5_ON_MS then fade off in A5_FADE_OFF_MS ms with 
         *  step A5_FADE_OFF_STEP
         *
         *  6: stay on for A6_ON_MS then turn off clockwise in A6_OFF_MS 
         *
         *  7: stay on for A7_ON_MS then turn off vertically in A7_OFF_MS
         *
         *  8: looping fade on in A5_FADE_ON_MS ms with step A5_FADE_ON_STEP
         *  stay on for A5_ON_MS then fade off in A5_FADE_OFF_MS ms with 
         *  step A5_FADE_OFF_STEP
         *
         *  9: looping fade on in A5_FADE_ON_MS ms with step A5_FADE_ON_STEP
         *  stay on for A5_ON_MS then fade off in A5_FADE_OFF_MS ms with 
         *  step A5_FADE_OFF_STEP, but only on LED previously initialized
         *
         *  10: turn LEDs off
         *
         *  11: looping waterfall animation turning on vertically in A11_FALL_MS,
         *      then off vertically in A11_FALL_MS
         *
         *  12: looping waterfall animation falling vertically in A11_FALL_MS,
         *      with a fading tail
         */
        void start_animation(uint8_t animation) {
            _animation = animation;
            _animation_step = 1;
            play_animation();
        }

        /**
         * Animation handler, add to main loop
         * Plays the animation specified in start_animation
         * \return true when animation has ended
         */
        boolean play_animation() {
            if (!_animation) {
                stop_animation();
                return false;
            }

            boolean res = false;

                switch (_animation) {
                    case 1:
                        res = animation_loop_cw(A1_LOOP_MS);
                        break;
                    case 2:
                        res = animation_loop_cw(A2_LOOP_MS);
                        break;
                    case 3:
                        res = animation_fade_on_off_cw(A3_FADE_ON_MS,
                                A3_FADE_ON_STEP,A3_ON_MS,A3_OFF_MS);
                        break;
                    case 4:
                        res = animation_loop_opposite(A4_LOOP_MS);
                        break;
                    case 5:
                        res = animation_fade_on_off(A5_FADE_ON_MS,A5_FADE_ON_STEP,
                                A5_ON_MS,A5_FADE_OFF_MS,A5_FADE_OFF_STEP);
                        break;
                    case 6:
                        res = animation_off_cw(A6_ON_MS,A6_OFF_MS);
                        break;
                    case 7:
                        res = animation_off_vertically(A7_ON_MS,A7_OFF_MS);
                        break;
                    case 8:
                        res = animation_fade_on_off(A5_FADE_ON_MS,A5_FADE_ON_STEP,
                                A5_ON_MS,A5_FADE_OFF_MS,A5_FADE_OFF_STEP,true);
                        break;
                    case 9:
                        disable_leds();
                        res = animation_fade_on_off(A5_FADE_ON_MS,A5_FADE_ON_STEP,
                                A5_ON_MS,A5_FADE_OFF_MS,A5_FADE_OFF_STEP,true,false);
                        break;
                    case 10:
                        stop_animation();
                        off();
                        break;
                    case 11:
                        res = animation_waterfall(A11_FALL_MS);
                        break;
                    case 12:
                        res = animation_fading_waterfall(A11_FALL_MS);
                        break;
                }
            return res;
        }

        /**
         * Clockwise led ring loop animation
         * \param loop_time_ms time to complete one animation loop in ms
         */
        boolean animation_loop_cw(uint32_t loop_time_ms, boolean fading = false, uint16_t fade_increment = 1000) {
            
            if (fading){
                increment_leds(-fade_increment);// use instead of off() for fading animation
            }else{
                off();
            }
            set_led(_animation_step % _num_leds,RING_ON_PWM);
            _animation_step++;
            if (_animation_step > _num_leds) _animation_step = 1;
            Timer1.setPeriod(loop_time_ms / _num_leds);
            return false;
        }

        /**
         * Fade on then turn off clockwise animation
         * \param fade_on_time_ms time to fade on in ms
         * \param fade_on_increment fade increment step size
         * \param on_time_ms time to pause after fade completed
         * \param turn_off_time_ms time to complete turn off animation in ms
         */
        boolean animation_fade_on_off_cw(uint32_t fade_on_time_ms, uint16_t fade_on_increment, 
                uint32_t on_time_ms, uint32_t turn_off_time_ms) {

            uint16_t turn_on_steps = RING_ON_PWM / fade_on_increment;
            uint16_t turn_on_gain = RING_ON_PWM / log(turn_on_steps);

            if (_animation_step == 1) {
                //start off
                off();
                _animation_step++;
            }

            if (_animation_step > 1 && _animation_step < turn_on_steps) {
                //fade on
                uint16_t delta =  turn_on_gain * (log(turn_on_steps - _animation_step) - log(turn_on_steps - _animation_step - 1));
                increment_leds(delta);
                Timer1.setPeriod(fade_on_time_ms / turn_on_steps);
            } else if (_animation_step == turn_on_steps)  {
                //pause on
                    on();
                    Timer1.setPeriod(on_time_ms);
            } else if ((_animation_step > turn_on_steps) && (_animation_step < turn_on_steps+_num_leds)) {
                //off cw
                uint8_t led = (_animation_step-turn_on_steps - 1) % _num_leds;
                set_led(led,0);
                Timer1.setPeriod(turn_off_time_ms / _num_leds);
            } else {
                //finished
                off();
                stop_animation();
                return true;
            }

            _animation_step++;
            return false;
        }

        /**
         * Stay on for specified time then turn off in clockwise direction
         * \param on_time_ms time to stay on in ms
         * \param turn_off_time_ms time to turn off in ms
         */
        boolean animation_off_cw(uint32_t on_time_ms, uint32_t turn_off_time_ms) {

            if (_animation_step == 1) {
                //pause on
                on();
                Timer1.setPeriod(on_time_ms);
            } else if ((_animation_step > 1) && (_animation_step < _num_leds)) {
                //turn off cw
                set_led(_animation_step - 2,0);
                Timer1.setPeriod(turn_off_time_ms / _num_leds);
            } else {
                //finished
                off();
                stop_animation();
                return true;
            }

            _animation_step++;
            return false;
        }

        /**
         * Loop two leds in opposite directions
         * \param loop_time_ms time to complete a loop (ms)
         */
        boolean animation_loop_opposite(uint32_t loop_time_ms, boolean fading = false, uint16_t fade_increment = 1000) {
            if (fading){
                increment_leds(-fade_increment);// use instead of off() for fading animation
            }else{
                off();
            }
            set_led(_animation_step % _num_leds,RING_ON_PWM);
            set_led(_num_leds - 1 - (_animation_step % _num_leds),RING_ON_PWM);
            Timer1.setPeriod(loop_time_ms / _num_leds);
            _animation_step++;
            //prevent jump when long max reached
            if (_animation_step >= _num_leds) {
                _animation_step = 2;
            }
            return false;
        }

        /**
         * Increment leds
         */
        void increment_leds(int16_t increment) {
            for(uint8_t led = 0; led < _num_leds; led++) {
                int16_t new_pwm = get_led(led) + increment;
                if (0 <= new_pwm <= RING_ON_PWM){
                    new_pwm = max(new_pwm,0);
                    new_pwm = min(new_pwm,RING_ON_PWM);
                    set_led_buffer(led,new_pwm);
                }
            }
            write();
        }

        /**
         * Fade on leds, pause then fade off
         * \param fade_on_time_ms time to fade on (ms)
         * \param fade_on_increment fade step (0..4095)
         * \param on_time_ms pause time in ms
         * \param fade_off_time time to fade off in ms
         * \param fade_off_increment fade step (0..4095)
         * \param loop set to true for animation to loop (default false)
         * \param reverse set to true for off then on (default false)
         * \param initialise set to false to skip initialisation (default true)
         */
        boolean animation_fade_on_off(uint32_t fade_on_time_ms, uint16_t fade_on_increment, uint32_t on_time_ms, 
                uint32_t fade_off_time_ms, uint16_t fade_off_increment, boolean loop = false, 
                boolean reverse = false, boolean initialise = true) {

            uint16_t turn_on_steps = RING_ON_PWM / fade_on_increment;
            uint16_t turn_off_steps = RING_ON_PWM / fade_off_increment;
            uint16_t turn_on_gain = RING_ON_PWM / log(turn_on_steps);
            uint16_t turn_off_gain = RING_ON_PWM / log(turn_off_steps);
            if (initialise) {
                if (_animation_step == 1) {
                    //start off
                    if (!reverse) {
                        off();
                    } else {
                        on();
                    }
                _animation_step++;
                }
            }

            if (_animation_step > 1 && _animation_step < turn_on_steps) {
                uint16_t delta =  turn_on_gain * (log(turn_on_steps - _animation_step)-log(turn_on_steps - _animation_step - 1));
                if (!reverse) {
                    //fade on
                    increment_leds(delta);
                } else {
                    //fade off
                    increment_leds(-delta);
                }
                Timer1.setPeriod(fade_on_time_ms / turn_on_steps);
            } else if (_animation_step == turn_on_steps)  {
                if (!reverse) {
                    //pause on
                    on();
                } else {
                    off();
                }
                Timer1.setPeriod(on_time_ms);
            }
            else if ((_animation_step > turn_on_steps) && (_animation_step < (turn_on_steps+turn_off_steps)))  {
                uint16_t delta = turn_off_gain * (log(_animation_step - turn_on_steps) - log(_animation_step - turn_on_steps - 1));
                if (!reverse) {
                    //fade off
                    increment_leds(-delta);
                } else {
                    //fade on
                    increment_leds(delta);
                }
                Timer1.setPeriod(fade_off_time_ms / turn_off_steps);
            } else {
                if (loop) {
                    //loop
                    _animation_step = 1;
                    return false;
                } else {
                    //finished
                    off();
                    stop_animation();
                    return true;
                }
            }

            _animation_step++;
            return false;

        }

        /**
         * Pause on for specified time then turn off vertically
         * \param on_time_ms time to stay on (ms)
         * \param turn_off_time_ms time taken to turn off (ms)
         */
        boolean animation_off_vertically(uint32_t on_time_ms, uint32_t turn_off_time_ms) {
            if (_animation_step == 1) {
                //pause on
                on();
                Timer1.setPeriod(on_time_ms);
            } else if ((_animation_step > 1) && (_animation_step < ((_num_leds/2)+2))) {
                //turn off vertically
                set_led(_animation_step - 2,0);
                set_led(_num_leds-_animation_step + 1,0);
                Timer1.setPeriod(turn_off_time_ms / _num_leds);
            } else {
                //finished
                off();
                stop_animation();
                return true;
            }

            _animation_step++;
            return false;
        }

        /**
         * Looping animation
         * Vertically descending waterfall, fills all LEDs, than empties
         */
        boolean animation_waterfall(uint32_t fall_time_ms, boolean loop = true) {
            uint32_t period=fall_time_ms / _num_leds;
            uint8_t cycle = (_num_leds / 2) + 2;
            if (_animation_step == 1) {
                //pause on
                off();
                Timer1.setPeriod(10 * period);
            } else if ((_animation_step > 1) && (_animation_step <= cycle)) {
                //turn on vertically
                set_led(_animation_step - 2,RING_ON_PWM);
                set_led(_num_leds-_animation_step + 1,RING_ON_PWM);
                Timer1.setPeriod(period);
            } else if ((_animation_step > cycle) && (_animation_step < 2 * cycle)) {
                //turn off vertically
                uint8_t reverse_step = _animation_step - cycle;
                set_led(reverse_step - 2,0);
                set_led(_num_leds - reverse_step + 1,0);
                Timer1.setPeriod(period);
            } else {
                if (loop) {
                    //loop
                    _animation_step = 1;
                    return false;
                } else {
                    //finished
                    off();
                    stop_animation();
                    return true;
                }
            }

            _animation_step++;
            return false;
        }

        /**
         * Looping animation
         * Vertically descending waterfall with fading tail
         */

        boolean animation_fading_waterfall(uint32_t fall_time_ms, uint16_t fade_increment = 1000, boolean loop = true) {
            uint32_t period=fall_time_ms / _num_leds;
            uint8_t cycle = (_num_leds / 2) + 2;
            uint8_t off_steps = RING_ON_PWM / fade_increment;
            if (_animation_step == 1) {
                //pause on
                off();
                Timer1.setPeriod(period);
            } else if ((_animation_step > 1) && (_animation_step <= cycle + off_steps)) {
                //turn on vertically
                increment_leds(-fade_increment);//causing fading tail
                if (_animation_step <= cycle){//to fade all LEDs off
                    set_led(_animation_step - 2,RING_ON_PWM);
                    set_led(_num_leds - _animation_step + 1,RING_ON_PWM);
                }
                Timer1.setPeriod(period);
            } else {
                if (loop) {
                    //loop
                    _animation_step = 1;
                    return false;
                } else {
                    //finished
                    off();
                    stop_animation();
                    return true;
                }
            }

            _animation_step++;
            return false;
        }


};



#endif
