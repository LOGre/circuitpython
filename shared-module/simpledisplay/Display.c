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

#include "shared-module/simpledisplay/__init__.h"
#include "shared-bindings/simpledisplay/FourWire.h"

// More info here: https://www.tonylabs.com/wp-content/uploads/MIPI_DCS_specification_v1.02.00.pdf
enum mipi_command {
    MIPI_COMMAND_SET_COLUMN_ADDRESS = 0x2a,
    MIPI_COMMAND_SET_PAGE_ADDRESS = 0x2b,
    MIPI_COMMAND_WRITE_MEMORY_START = 0x2c,
};

// driver implementation of show()
// Currently only send white RGB565 pixels
void common_hal_simpledisplay_display_show(simpledisplay_display_obj_t* self) {

    // send ST7735_RAMWR command aka MIPI_COMMAND_WRITE_MEMORY_START
    uint8_t cmdBuf[] = { MIPI_COMMAND_WRITE_MEMORY_START };
    common_hal_simpledisplay_fourwire_send(self->bus, true, cmdBuf, 1);

    // DUMMY: send white screen data to fill the screen
    uint8_t dataBuf[160*128*2]; 
    int i = 0;
    for(i=0; i<160*128*2; i++) {
        dataBuf[i] = 0xFF;
    }
    common_hal_simpledisplay_fourwire_send(self->bus, false, dataBuf, 160*128*2);
}

