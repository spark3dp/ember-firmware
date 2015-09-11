//  This file is part of the Ember AVR bootloader.
//
//  This product is based on twiboot, Copyright 2010 Olaf Rempel.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Olaf Rempel razzor@kopf-tisch.de
//  Jason Lefley
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; version 2
//  of the License.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, see <http://www.gnu.org/licenses/>.

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <util/crc16.h>

#if defined (__AVR_ATmega8__)
#define VERSION_STRING      "TWIBOOT m8v2.1"
#define SIGNATURE_BYTES     0x1E, 0x93, 0x07

#elif defined (__AVR_ATmega88__)
#define VERSION_STRING      "TWIBOOT m88v2.1"
#define SIGNATURE_BYTES     0x1E, 0x93, 0x0A

#elif defined (__AVR_ATmega168__)
#define VERSION_STRING      "TWIBOOT m168v2.1"
#define SIGNATURE_BYTES     0x1E, 0x94, 0x06

#elif defined (__AVR_ATmega328P__)
#define VERSION_STRING      "TWIBOOTm328pv2.1"
#define SIGNATURE_BYTES     0x1E, 0x95, 0x0F

#else
#error MCU not supported
#endif

#define EEPROM_SUPPORT      1

/*
 * LED_SUPPORT enables bootloader status output to LEDs
 * This is unrelated to the LED ring and is preserved
 * for debugging purposes
 */
#define LED_SUPPORT     0

/* 25ms @8MHz */
#define TIMER_RELOAD        (0xFF - 195)

/*
 * A timeout value of 0 results in the bootloader waiting for
 * a specific TWI command before starting the application
 */
#define TIMEOUT         0

#if LED_SUPPORT
#define LED_INIT()      DDRB = ((1<<PORTB4) | (1<<PORTB5))
#define LED_RT_ON()     PORTB |= (1<<PORTB4)
#define LED_RT_OFF()    PORTB &= ~(1<<PORTB4)
#define LED_GN_ON()     PORTB |= (1<<PORTB5)
#define LED_GN_OFF()    PORTB &= ~(1<<PORTB5)
#define LED_GN_TOGGLE() PORTB ^= (1<<PORTB5)
#define LED_OFF()       PORTB = 0x00
#else
#define LED_INIT()
#define LED_RT_ON()
#define LED_RT_OFF()
#define LED_GN_ON()
#define LED_GN_OFF()
#define LED_GN_TOGGLE()
#define LED_OFF()
#endif

#ifndef TWI_ADDRESS
#define TWI_ADDRESS     0x29
#endif

/* SLA+R */
#define CMD_WAIT        0x00
#define CMD_READ_VERSION    0x01
#define CMD_READ_MEMORY     0x02
#define CMD_READ_CHECKSUM 0x03
/* internal mappings */
#define CMD_READ_CHIPINFO   (0x10 | CMD_READ_MEMORY)
#define CMD_READ_FLASH      (0x20 | CMD_READ_MEMORY)
#define CMD_READ_EEPROM     (0x30 | CMD_READ_MEMORY)
#define CMD_READ_PARAMETERS (0x40 | CMD_READ_MEMORY)    /* only in APP */

/* SLA+W */
#define CMD_SWITCH_APPLICATION  CMD_READ_VERSION
#define CMD_WRITE_MEMORY    CMD_READ_MEMORY
/* internal mappings */
#define CMD_BOOT_BOOTLOADER (0x10 | CMD_SWITCH_APPLICATION) /* only in APP */
#define CMD_BOOT_APPLICATION    (0x20 | CMD_SWITCH_APPLICATION)
#define CMD_WRITE_CHIPINFO  (0x10 | CMD_WRITE_MEMORY)   /* invalid */
#define CMD_WRITE_FLASH     (0x20 | CMD_WRITE_MEMORY)
#define CMD_WRITE_EEPROM    (0x30 | CMD_WRITE_MEMORY)
#define CMD_WRITE_PARAMETERS    (0x40 | CMD_WRITE_MEMORY)   /* only in APP */

/* CMD_SWITCH_APPLICATION parameter */
#define BOOTTYPE_BOOTLOADER 0x00                /* only in APP */
#define BOOTTYPE_APPLICATION    0x80

/* CMD_{READ|WRITE}_* parameter */
#define MEMTYPE_CHIPINFO    0x00
#define MEMTYPE_FLASH       0x01
#define MEMTYPE_EEPROM      0x02
#define MEMTYPE_PARAMETERS  0x03                /* only in APP */

// constants for LED ring
#define  RING_OE                (PORTB0) /* LED ring OE pin */
#define  RING_LATCH             (PORTB1) /* ring latch pin */
#define  RING_DATA              (PORTD6) /* ring data pin */
#define  RING_CLOCK             (PORTD7) /* ring clock pin */

#define RING_OE_HIGH()      PORTB |= (1<<RING_OE)
#define RING_OE_LOW()       PORTB &= ~(1<<RING_OE)
#define RING_LATCH_LOW()    PORTB &= ~(1<<RING_LATCH)
#define RING_LATCH_HIGH()   PORTB |= (1<<RING_LATCH)
#define RING_DATA_HIGH()    PORTD |= (1<<RING_DATA)
#define RING_DATA_LOW()     PORTD &= ~(1<<RING_DATA)
#define RING_CLOCK_HIGH()   PORTD |= (1<<RING_CLOCK)
#define RING_CLOCK_LOW()    PORTD &= ~(1<<RING_CLOCK)

#define NUM_LEDS                21 // number of LEDs in ring
#define LED_RING_TIMES          5  // x 25ms = delay between successive LEDs
static int led_ring_timer_count = LED_RING_TIMES;
volatile static uint8_t show_next_LED = 0;  // flag set by timer ISR

/*
 * LED_GN blinks with 20Hz (while bootloader is running)
 * LED_RT blinks on TWI activity
 *
 * bootloader twi-protocol:
 * - abort boot timeout:
 *   SLA+W, 0x00, STO
 *
 * - show bootloader version
 *   SLA+W, 0x01, SLA+R, {16 bytes}, STO
 *
 * - start application
 *   SLA+W, 0x01, 0x80, STO
 *
 * - read chip info: 3byte signature, 1byte page size, 2byte flash size, 2byte eeprom size
 *   SLA+W, 0x02, 0x00, 0x00, 0x00, SLA+R, {4 bytes}, STO
 *
 * - read one (or more) flash bytes
 *   SLA+W, 0x02, 0x01, addrh, addrl, SLA+R, {* bytes}, STO
 *
 * - read one (or more) eeprom bytes
 *   SLA+W, 0x02, 0x02, addrh, addrl, SLA+R, {* bytes}, STO
 *
 * - write one flash page (64bytes on mega8)
 *   SLA+W, 0x02, 0x01, addrh, addrl, {64 bytes}, STO
 *
 * - write one (or more) eeprom bytes
 *   SLA+W, 0x02, 0x02, addrh, addrl, {* bytes}, STO
 */

const static uint8_t info[16] = VERSION_STRING;
const static uint8_t chipinfo[8] = {
    SIGNATURE_BYTES,

    SPM_PAGESIZE,

    (BOOTLOADER_START >> 8) & 0xFF,
    BOOTLOADER_START & 0xFF,
#if (EEPROM_SUPPORT)
    ((E2END +1) >> 8 & 0xFF),
    (E2END +1) & 0xFF
#else
    0x00, 0x00
#endif
};

static uint8_t checksum[2];

/* wait 40 * 25ms = 1s */
static uint8_t boot_timeout = TIMEOUT;
volatile static uint8_t cmd = CMD_WAIT;

/* flash buffer */
static uint8_t buf[SPM_PAGESIZE];
static uint16_t addr;

static void write_flash_page(void)
{
    uint16_t pagestart = addr;
    uint8_t size = SPM_PAGESIZE;
    uint8_t *p = buf;

    if (pagestart >= BOOTLOADER_START)
        return;

    boot_page_erase(pagestart);
    boot_spm_busy_wait();

    do {
        uint16_t data = *p++;
        data |= *p++ << 8;
        boot_page_fill(addr, data);

        addr += 2;
        size -= 2;
    } while (size);

    boot_page_write(pagestart);
    boot_spm_busy_wait();
    boot_rww_enable();
}

#if (EEPROM_SUPPORT)
static uint8_t read_eeprom_byte(void)
{
    EEARL = addr;
    EEARH = (addr >> 8);
    EECR |= (1<<EERE);
    addr++;
    return EEDR;
}

static void write_eeprom_byte(uint8_t val)
{
    EEARL = addr;
    EEARH = (addr >> 8);
    EEDR = val;
    addr++;
#if defined (__AVR_ATmega8__)
    EECR |= (1<<EEMWE);
    EECR |= (1<<EEWE);
#elif defined (__AVR_ATmega88__) || defined (__AVR_ATmega168__) || defined (__AVR_ATmega328P__)
    EECR |= (1<<EEMPE);
    EECR |= (1<<EEPE);
#endif
    eeprom_busy_wait();
}
#endif /* EEPROM_SUPPORT */

ISR(TWI_vect)
{
    static uint8_t bcnt;
    uint8_t data;
    uint8_t ack = (1<<TWEA);

    switch (TWSR & 0xF8) {
    /* SLA+W received, ACK returned -> receive data and ACK */
    case 0x60:
        bcnt = 0;
        LED_RT_ON();
        TWCR |= (1<<TWINT) | (1<<TWEA);
        break;

    /* prev. SLA+W, data received, ACK returned -> receive data and ACK */
    case 0x80:
        data = TWDR;
        switch (bcnt) {
        case 0:
            switch (data) {
            case CMD_SWITCH_APPLICATION:
            case CMD_WRITE_MEMORY:
                bcnt++;
                /* no break */

            case CMD_WAIT:
                /* abort countdown */
                boot_timeout = 0;
                break;

            default:
                /* boot app now */
                cmd = CMD_BOOT_APPLICATION;
                ack = (0<<TWEA);
                break;
            }
            cmd = data;
            break;

        case 1:
            switch (cmd) {
            case CMD_SWITCH_APPLICATION:
                if (data == BOOTTYPE_APPLICATION) {
                    cmd = CMD_BOOT_APPLICATION;
                }
                ack = (0<<TWEA);
                break;

            case CMD_WRITE_MEMORY:
                bcnt++;
                if (data == MEMTYPE_CHIPINFO) {
                    cmd = CMD_WRITE_CHIPINFO;

                } else if (data == MEMTYPE_FLASH) {
                    cmd = CMD_WRITE_FLASH;
#if (EEPROM_SUPPORT)
                } else if (data == MEMTYPE_EEPROM) {
                    cmd = CMD_WRITE_EEPROM;
#endif
                } else {
                    ack = (0<<TWEA);
                }
                break;

            default:
                ack = (0<<TWEA);
                break;
            }
            break;

        case 2:
        case 3:
            addr <<= 8;
            addr |= data;
            bcnt++;
            break;

        default:
            switch (cmd) {
            case CMD_WRITE_FLASH:
                buf[bcnt -4] = data;
                if (bcnt < sizeof(buf) +3) {
                    bcnt++;
                } else {
                    write_flash_page();
                    ack = (0<<TWEA);
                }
                break;
#if (EEPROM_SUPPORT)
            case CMD_WRITE_EEPROM:
                write_eeprom_byte(data);
                bcnt++;
                break;
#endif
            default:
                ack = (0<<TWEA);
                break;
            }
            break;
        }

        if (ack == 0x00)
            bcnt = 0;

        TWCR |= (1<<TWINT) | ack;
        break;

    /* SLA+R received, ACK returned -> send data */
    case 0xA8:
        bcnt = 0;
        LED_RT_ON();

    /* prev. SLA+R, data sent, ACK returned -> send data */
    case 0xB8:
        switch (cmd) {
        case CMD_READ_VERSION:
            data = info[bcnt++];
            bcnt %= sizeof(info);
            break;

        case CMD_READ_CHIPINFO:
            data = chipinfo[bcnt++];
            bcnt %= sizeof(chipinfo);
            break;

        case CMD_READ_FLASH:
            data = pgm_read_byte_near(addr++);
            break;
#if (EEPROM_SUPPORT)
        case CMD_READ_EEPROM:
            data = read_eeprom_byte();
            break;
#endif
    case CMD_READ_CHECKSUM:
      data = checksum[bcnt++];
      bcnt %= sizeof(checksum);
      break;
        default:
            data = 0xFF;
            break;
        }

        TWDR = data;
        TWCR |= (1<<TWINT) | (1<<TWEA);
        break;

    /* STOP or repeated START */
    case 0xA0:
    /* data sent, NACK returned */
    case 0xC0:
        LED_RT_OFF();
        TWCR |= (1<<TWINT) | (1<<TWEA);
        break;

    /* illegal state -> reset hardware */
    case 0xF8:
        TWCR |= (1<<TWINT) | (1<<TWSTO) | (1<<TWEA);
        break;
    }
}

ISR(TIMER0_OVF_vect)
{
    /* restart timer */
    TCNT0 = TIMER_RELOAD;

    /* see if it's time to show the next LED in the ring */
    if(--led_ring_timer_count < 1)
    {
        show_next_LED = 1;
        led_ring_timer_count = LED_RING_TIMES;
    }

    /* blink LED while running */
    LED_GN_TOGGLE();

    /* count down for app-boot */
    if (boot_timeout > 1)
        boot_timeout--;

    /* trigger app-boot */
    else if (boot_timeout == 1)
        cmd = CMD_BOOT_APPLICATION;
}

static void (*jump_to_app)(void) __attribute__ ((noreturn)) = 0x0000;

/*
 * For newer devices (mega88) the watchdog timer remains active even after a
 * system reset. So disable it as soon as possible.
 * automagically called on startup
 */
#if defined (__AVR_ATmega88__) || defined (__AVR_ATmega168__) || defined (__AVR_ATmega328P__)
void disable_wdt_timer(void) __attribute__((naked, section(".init3")));
void disable_wdt_timer(void)
{
    MCUSR = 0;
    WDTCSR = (1<<WDCE) | (1<<WDE);
    WDTCSR = (0<<WDE);
}
#endif

uint16_t compute_crc16(void)
{
  uint16_t last_addr;
  uint16_t addr;
  uint16_t crc_u16;
  uint8_t byte_u8;

  /* get the last address programmed -- this should be immediately after
   * the interrupt vectors
   * this is computed and added to the application hex file when building
   * application
   * this value includes the maximum address itself  (high water + 1)
   */
  last_addr = pgm_read_dword_near((uint16_t)_VECTORS_SIZE);

  /* check that last address is valid
   * can't be zero and can't exceed application section
   */
  if (last_addr == 0 || last_addr > BOOTLOADER_START - 1)
  {
    /* invalid last address, return error value */
    return 0;
  }

  /* compute crc16 */
  crc_u16 = 0;

  for(addr = 0; addr < last_addr; addr++)
  {
    byte_u8 = pgm_read_byte_near(addr);
    crc_u16 = _crc_xmodem_update(crc_u16, byte_u8);
  }

  return crc_u16;
}

/* illuminate the given LED at the maximum brightness (4095), */
/* turning off all others in the ring */
void show_LED(int ledNum)
{
    // the LED driver has 24 outputs, though only 21 are used
    uint16_t led_buffer[24];

    int led;
    int8_t i, b;
    for(led = 0; led < NUM_LEDS; led++)
        led_buffer[led] = (led == ledNum) ? 4095 : 0;

    // shift out the buffer for the LED ring MSB first, 12 bits per led, 24 outputs
    RING_LATCH_LOW();
    for (i = 24 - 1; i >= 0; i--)
    {
        for (b = 11; b >= 0; b--)
        {
            RING_CLOCK_LOW();
            if(led_buffer[i] & (1 << b))
                RING_DATA_HIGH();
            else
                RING_DATA_LOW();
            RING_CLOCK_HIGH();
        }
    }
    RING_CLOCK_LOW();
    RING_LATCH_HIGH();
    RING_LATCH_LOW();

    // enable the LED driver
    RING_OE_LOW();
}

int main(void) __attribute__ ((noreturn));
int main(void)
{
  /* compute crc16 and store in checksum buffer */
    uint16_t crc16 = compute_crc16();
    checksum[0] = (crc16 >> (8 * 0)) & 0xff;
    checksum[1] = (crc16 >> (8 * 1)) & 0xff;

    LED_INIT();
    LED_GN_ON();

    // initialize outputs for front panel AVR LED ring
    DDRB = ((1<<RING_OE) | (1<<RING_LATCH));
    DDRD = ((1<<RING_DATA) | (1<<RING_CLOCK));
    RING_LATCH_LOW();
    RING_OE_HIGH();  // disable the LEDs until they're initialized

    // for motor controller AVR, disable motor driver by setting PC1 high
    DDRC = (1<<PORTC1);
    PORTC |= (1<<PORTC1);

    // the LED to illuminate next in the ring, starting at the top
    int litLED = 12;

    /* move interrupt-vectors to bootloader */
    /* timer0: running with F_CPU/1024, OVF interrupt */
#if defined (__AVR_ATmega8__)
    GICR = (1<<IVCE);
    GICR = (1<<IVSEL);

    TCCR0 = (1<<CS02) | (1<<CS00);
    TIMSK = (1<<TOIE0);
#elif defined (__AVR_ATmega88__) || defined (__AVR_ATmega168__) || defined (__AVR_ATmega328P__)
    MCUCR = (1<<IVCE);
    MCUCR = (1<<IVSEL);

    TCCR0B = (1<<CS02) | (1<<CS00);
    TIMSK0 = (1<<TOIE0);
#endif

    /* TWI init: set address, auto ACKs with interrupts */
    TWAR = (TWI_ADDRESS<<1);
    TWCR = (1<<TWEA) | (1<<TWEN) | (1<<TWIE);

    sei();
    while (cmd != CMD_BOOT_APPLICATION)
    {
        // see if LED timer has expired
        if(show_next_LED)
        {
            show_next_LED = 0;
            show_LED(litLED);

            // get the next LED
            litLED++;
            if(litLED >= NUM_LEDS)
                litLED = 0;
        }
    }
    cli();

    /* Disable TWI but keep address! */
    TWCR = 0x00;

    /* disable timer0 */
    /* move interrupt vectors back to application */
#if defined (__AVR_ATmega8__)
    TCCR0 = 0x00;
    TIMSK = 0x00;

    GICR = (1<<IVCE);
    GICR = (0<<IVSEL);
#elif defined (__AVR_ATmega88__) || defined (__AVR_ATmega168__) || defined (__AVR_ATmega328P__)
    TIMSK0 = 0x00;
    TCCR0B = 0x00;

    MCUCR = (1<<IVCE);
    MCUCR = (0<<IVSEL);
#endif

    LED_OFF();

    uint16_t wait = 0x0000;
    do {
        __asm volatile ("nop");
    } while (--wait);

    jump_to_app();
}
