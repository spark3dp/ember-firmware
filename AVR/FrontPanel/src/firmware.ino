//  File:   firmware.ino
//  Setup and main loop 
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

#define HARDWARE_VERSION A3

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Arduino / AVR Libraries
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <avr/pgmspace.h> //Flash memory storage
#include <avr/interrupt.h>
#include <SPI.h>
#include <avr/wdt.h>

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  3rd party librarys
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <PinInterrupts.h> //Pin Interrupt Library
#include <TimerOne.h>

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Electric Echidna Libraries
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <Logging.h> //Electric Echidna Logging Library
#include <Button.h>

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  User Customisable Modules
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "variables.h"
#include "hardware.h"

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Project Modules
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "Ring.h"
#include "OLED.h"
#include "I2CInterface.h"

#define LogSerial Serial //!< Use Serial for log messages

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Main Objects
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Ring ring(RING_OE,RING_LATCH,RING_DATA,RING_CLOCK,RING_LEDS,RING_NUMBERING);
OLED oled(OLED_CS,OLED_DC,OLED_RST);
I2CInterface interface(INTERFACE_ADDRESS,INTERFACE_INTERRUPT,&ring, &oled);

Button button1(BUTTON1,DEBOUNCE_TIME_MS,BUTTON_HOLD_TIME_MS);
Button button2(BUTTON2,DEBOUNCE_TIME_MS,BUTTON_HOLD_TIME_MS);

int OVCounter = 0; //counts timer2 overflows
boolean button1Flag = 0;
boolean button2Flag = 0;


/**
 * \brief logging library callback
 */
void printLog(const char *msg,int level) {

#ifdef DEBUG

        if (level == LOG_INFO) {
            LogSerial.print(F("\033[34m"));
            LogSerial.print(millis());
            LogSerial.print(F(": "));
            LogSerial.print(msg);
            LogSerial.print(F("\r\n"));
            LogSerial.print(F("\033[0m"));
            return;
        }
        if (level == LOG_ERROR) {
            LogSerial.print(F("\033[31m"));
            LogSerial.print(millis());
            LogSerial.print(F(": "));
            LogSerial.print(msg);
            LogSerial.print(F("\r\n"));
            LogSerial.print(F("\033[0m"));
            return;
        }
        if (level == LOG_DEBUG) {
            LogSerial.print(F("\033[32m"));
            LogSerial.print(millis());
            LogSerial.print(F(": "));
            LogSerial.print(msg);
            LogSerial.print(F("\r\n"));
            LogSerial.print(F("\033[0m"));
            return;
        }
#endif

}
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  ISRs
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/**
 * \brief I2C request interrupt handler.
 */
void isr_i2c_request() {
//    Serial.println("**I2C request**");
    interface.request();
}

/**
 * \brief I2C receive interrupt handler.
 */
void isr_i2c_receive(int num_bytes) {
    interface.receive(num_bytes);
}

#if DEBUG
volatile int animation = 0;
#endif

/**
 * \brief button 1 ISR
 */
void isr_button1() {
    button1.interrupt();
    
    //time how long the button is held with timer2
    if(button1.state() == ButtonDepressed) {
      button1Flag = 1;
      if(!button2Flag) {
	TCNT2 = 0; //initlize timer2
	TIMSK2 |= (1 << TOIE2); //enable timer2 over flow interrupt
      }
    }
    
    //handle button events, use 'while' instead of 'if' to block against interrupts
    while (button1.state() == ButtonPressed) {
        button1.reset_state();
	resetTimer2();
        //Log.debug("Button 1: pressed");
        if (interface.WakeScreen()){//only send commands if the screen is awake
            interface.process_event(EventButton1Pressed);
            interface.start_interrupt();
        }
#if DEBUG
        animation++;
        ring.start_animation(animation);
        if (animation > 8) animation = 0;
#endif
    }

    while (button1.state() == ButtonHeld) {
        button1.reset_state();
	resetTimer2();
        //Log.debug("Button 1: held");
    }
}

/**
 * \brief button 2 ISR
 */
void isr_button2() {
    button2.interrupt();

    if(button2.state() == ButtonDepressed) {
      button2Flag = 1;
      if(!button1Flag) {
	TCNT2 = 0; //initlize timer2
	TIMSK2 |= (1 << TOIE2); //enable timer2 over flow interrupt
      }
    }

    // again, use 'while' instead of 'if' to block against interrupts
    while (button2.state() == ButtonPressed) {
        button2.reset_state();
        //Log.debug("Button 2: pressed");
        if (interface.WakeScreen()){//only send commands if the screen is awake
            interface.process_event(EventButton2Pressed);
            interface.start_interrupt();
        }
    }

    while (button2.state() == ButtonHeld) {
        button2.reset_state();
        //Log.debug("Button 2: held");
        if (interface.WakeScreen()){//only send commands if the screen is awake
            interface.process_event(EventButton2Held);
            interface.start_interrupt();
        }
    }
}

void isr_timer1() {
    ring.play_animation();
}

ISR(TIMER2_OVF_vect)
{
  //allows held button to trigger event while being held
  OVCounter++;
  if((OVCounter * 32) > BUTTON_HOLD_TIME_MS) {
    OVCounter = 0;
    if (interface.WakeScreen()) {//only send commands if the screen is awake
      if(button1Flag && button2Flag) {
	interface.process_event(EventBothButtonsHeld);
      } else if(button1Flag) {
	interface.process_event(EventButton1Held);
      } else {
	interface.process_event(EventButton2Held);
      }
      interface.start_interrupt();
    }
  }
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Arduino Functions
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void setup() {

    //disable watchdog from reset
    MCUSR=0;
    wdt_disable();

    //initialise logging
    LogSerial.begin(LOG_BAUD);
    Log.init(LOG_LEVEL);
    Log.addHandler(printLog);
    Log.debug(""); //blank line
    Log.debug("Control Panel: starting");

    //initialise LED ring
    ring.off();
    ring.enable();
    Timer1.initialize();
    Timer1.attachInterrupt(isr_timer1);
    Timer1.stop();

    //initialise OLED
    oled.Init();

    //enable watchdog
    wdt_enable(WDTO_2S); 

    // initialise interface
    Wire.onRequest(isr_i2c_request);
    Wire.onReceive(isr_i2c_receive);
    interface.begin();

    //initialise timer2
    TCCR2A = 0;
    TCCR2B = 0;
    TCCR2B |= (1 << CS22); //prescale to 1024
    TCCR2B |= (1 << CS21);
    TCCR2B |= (1 << CS20);
        
    // initialise buttons
    PCattachInterrupt(BUTTON1,isr_button1,CHANGE);
    PCattachInterrupt(BUTTON2,isr_button2,CHANGE);

    // generate startup interrupt
    //interface.process_event(EventStartup);
    //interface.start_interrupt();
    oled.DrawLogo();
    Log.debug("Control Panel: started");
}


void loop() {
    wdt_reset();

    interface.stop_interrupt(); //non-blocking interrupt stop
    
    // use 'while' instead of 'if' to block against interrupts, and
    // thereby prevent crashing
    while(interface.listen());  
}

void resetTimer2() {
  button1Flag = 0;
  button2Flag = 0;
  OVCounter = 0;
  TIMSK2 = 0; //disable overflow interrupts
}
