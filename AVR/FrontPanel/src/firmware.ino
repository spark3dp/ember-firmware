/**
 * \file   firmware.ino
 * \Author Electric Echidna (help@electric-echidna.com)
 * \brief  Autodesk Control Panel Firmware
 */


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
    Serial.println("**I2C request**");
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

    //handle button events
    if (button1.state() == ButtonPressed) {
        button1.reset_state();
        //Log.debug("Button 1: pressed");
        if (interface.WakeScreen()){//only send commands if the screen is awake
            interface.process_event(EventButton1Pressed);
            interface.start_interrupt();
        }
#if DEBUG
        animation++;
        ring.start_animation(animation);
        if (animation >8) animation = 0;
#endif
    }

    if (button1.state() == ButtonHeld) {
        button1.reset_state();
        //Log.debug("Button 1: held");
        if (interface.WakeScreen()){//only send commands if the screen is awake
            interface.process_event(EventButton1Held);
            interface.start_interrupt();
        }
    }
}

/**
 * \brief button 2 ISR
 */
void isr_button2() {
    button2.interrupt();

    if (button2.state() == ButtonPressed) {
        button2.reset_state();
        //Log.debug("Button 2: pressed");
        if (interface.WakeScreen()){//only send commands if the screen is awake
            interface.process_event(EventButton2Pressed);
            interface.start_interrupt();
        }
    }

    if (button2.state() == ButtonHeld) {
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


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Arduino Functions
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void setup() {

    //disable watchdog from reset
    MCUSR=0;
    wdt_disable();
    wdt_enable(WDTO_8S);


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

    // initialise interface
    Wire.onRequest(isr_i2c_request);
    Wire.onReceive(isr_i2c_receive);
    interface.begin();

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
    //button1.update_status();
    //button2.update_status();
    interface.stop_interrupt(); //non-blocking interrupt stop

    //non-blocking animations
    //if (ring.play_animation()) {
    //    Log.debug("Ring: animation finished");
        // generate animation end interrupt
        //interface.process_event(EventAnimationEnd);
        //interface.start_interrupt();
    //}

    //check for commands
    if (interface.listen()) {
        Log.debug("Interface: command frame processed");
        //generate command acknowledged interrupt
        //interface.process_event(EventCommandReceived);
        //interface.start_interrupt();
    }

}


