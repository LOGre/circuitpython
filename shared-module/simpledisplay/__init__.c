/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Scott Shawcroft for Adafruit Industries
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "shared-bindings/simpledisplay/Display.h"

#include "py/runtime.h"
#include "shared-bindings/simpledisplay/FourWire.h"
#include "shared-bindings/microcontroller/Pin.h"
#include "shared-bindings/time/__init__.h"
#include "shared-module/simpledisplay/__init__.h"

#include "shared-bindings/digitalio/DigitalInOut.h"
#include "shared-bindings/pulseio/PWMOut.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "tick.h"


// TFT STST7735R Init Sequence
#define DELAY 0x80

uint8_t display_init_sequence[] = {
    0x01, 0 | DELAY, 150, // SWRESET
    0x11, 0 | DELAY, 255, // SLPOUT
    0xb1, 3, 0x01, 0x2C, 0x2D, // _FRMCTR1
    0xb2, 3, 0x01, 0x2C, 0x2D, //
    0xb3, 6, 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D,
    0xb4, 1, 0x07, // _INVCTR line inversion
    0xc0, 3, 0xa2, 0x02, 0x84, // _PWCTR1 GVDD = 4.7V, 1.0uA
    0xc1, 1, 0xc5, // _PWCTR2 VGH=14.7V, VGL=-7.35V
    0xc2, 2, 0x0a, 0x00, // _PWCTR3 Opamp current small, Boost frequency
    0xc3, 2, 0x8a, 0x2a,
    0xc4, 2, 0x8a, 0xee,
    0xc5, 1, 0x0e, // _VMCTR1 VCOMH = 4V, VOML = -1.1V
    0x2a, 0, // _INVOFF
    0x36, 1, 0x00, // _MADCTL top to bottom refresh in vsync aligned order.
    // 1 clk cycle nonoverlap, 2 cycle gate rise, 3 sycle osc equalie,
    // fix on VTL
    0x3a, 1, 0x05, // COLMOD - 16bit color
    0xe0, 16, 0x02, 0x1c, 0x07, 0x12, // _GMCTRP1 Gamma
              0x37, 0x32, 0x29, 0x2d,
              0x29, 0x25, 0x2B, 0x39,
              0x00, 0x01, 0x03, 0x10,
    0xe1, 16, 0x03, 0x1d, 0x07, 0x06, // _GMCTRN1
              0x2E, 0x2C, 0x29, 0x2D,
              0x2E, 0x2E, 0x37, 0x3F,
              0x00, 0x00, 0x02, 0x10,
    0x2a, 3, 0x02, 0x00, 0x81, // _CASET XSTART = 2, XEND = 129
    0x2b, 3, 0x02, 0x00, 0x81, // _RASET XSTART = 2, XEND = 129
    0x13, 0 | DELAY, 10, // _NORON
    0x29, 0 | DELAY, 100, // _DISPON
};

// internal driver constructor
void common_hal_simpledisplay_display_construct(simpledisplay_display_obj_t* self,
        mp_obj_t bus, uint16_t width, uint16_t height) {

    self->bus = bus;
    self->width = width;
    self->height = height;

    // init
    uint32_t i = 0;

    // check SPI availability
    while (!common_hal_simpledisplay_fourwire_begin_transaction(self->bus)) {
    #ifdef MICROPY_VM_HOOK_LOOP
            MICROPY_VM_HOOK_LOOP ;
    #endif
    }    
    // send init commands+parameters as data with delays
    while (i < sizeof(display_init_sequence)) {
        uint8_t *cmd = display_init_sequence + i;
        uint8_t data_size = *(cmd + 1);
        bool delay = (data_size & DELAY) != 0;
        data_size &= ~DELAY;
        uint8_t *data = cmd + 2;
        common_hal_simpledisplay_fourwire_send(self->bus, true, cmd, 1);

        common_hal_simpledisplay_fourwire_send(self->bus, false, data, data_size);
        
        uint16_t delay_length_ms = 10;
        if (delay) {
            data_size++;
            delay_length_ms = *(cmd + 1 + data_size);
            if (delay_length_ms == 255) {
                delay_length_ms = 500;
            }
        }
        common_hal_time_delay_ms(delay_length_ms);
        i += 2 + data_size;
    }

    common_hal_simpledisplay_fourwire_end_transaction(self->bus);

    // Set brightness
    const mcu_pin_obj_t * backlight_pin = &pin_PA01;
    pulseio_pwmout_obj_t backlight_pwm;
    digitalio_digitalinout_obj_t backlight_inout;

    if (common_hal_mcu_pin_is_free(backlight_pin)) {
        pwmout_result_t result = common_hal_pulseio_pwmout_construct(&backlight_pwm, backlight_pin, 0, 50000, false);
        if (result != PWMOUT_OK) {
            common_hal_digitalio_digitalinout_construct(&backlight_inout, backlight_pin);
            never_reset_pin_number(backlight_pin->number);
            common_hal_digitalio_digitalinout_set_value(&backlight_inout, 1.0f > 0.99);
        } 
        else {
            common_hal_pulseio_pwmout_never_reset(&backlight_pwm);
            common_hal_pulseio_pwmout_set_duty_cycle(&backlight_pwm, (uint16_t) (0xffff * 1.0f));
        }
    }    

}
