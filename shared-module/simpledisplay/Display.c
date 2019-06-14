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

#include "py/mperrno.h"
#include "py/runtime.h"

#include "extmod/modframebuf.h"

#include <stdio.h>

// More info here: https://www.tonylabs.com/wp-content/uploads/MIPI_DCS_specification_v1.02.00.pdf
enum mipi_command {
    MIPI_COMMAND_SET_COLUMN_ADDRESS = 0x2a,
    MIPI_COMMAND_SET_PAGE_ADDRESS = 0x2b,
    MIPI_COMMAND_WRITE_MEMORY_START = 0x2c,
};

// driver implementation of show()
// Currently only send white RGB565 pixels
void common_hal_simpledisplay_display_show(simpledisplay_display_obj_t* self, mp_obj_framebuf_t* fb) {

    printf("common_hal_simpledisplay_display_show\n");

     // get format, palette and pointer to byte buffer
    uint8_t format = fb->format;   
    
    // retrieve buffer
    byte *p = fb->buf;    

    // retrieve palette (if exists)
    mp_obj_palette_t * palette = fb->palette; 

    printf("fb %p format: %d buflen: %d width: %d\n", fb, format, fb->buf_len, fb->width);
    if(fb->format != FRAMEBUF_RGB565) {
        uint16_t * cols = palette->colors;
        printf("palette: %p colors: %d\n", palette, palette->nb_colors);
        for(int i=0; i<palette->nb_colors; i++) {
            printf("col[%d] : 0x%04x\n", i, cols[i]);
        }
    }

    // send ST7735_RAMWR command aka MIPI_COMMAND_WRITE_MEMORY_START
    uint8_t cmdBuf[] = { MIPI_COMMAND_WRITE_MEMORY_START };
    common_hal_simpledisplay_fourwire_send(self->bus, true, cmdBuf, 1);

    const uint16_t * pal;
    switch (format) {
        case FRAMEBUF_RGB565:         
            common_hal_simpledisplay_fourwire_send(self->bus, false, p, fb->buf_len);
            break;            
        case FRAMEBUF_PAL16:    
            // after some tests, sending by packets of 8 pixels is the most efficient 
            // and not too greedy in terms of memory
            pal = palette->colors;
            uint8_t pixels_per_tansfer = 8;
            for (int i=0;i<fb->buf_len;i+=pixels_per_tansfer){
                uint8_t cc[pixels_per_tansfer*4];
                uint8_t counter = 0;
                for(int j=0; j<pixels_per_tansfer; j++){                
                    cc[counter++] = pal[(p[i+j] >> 4) & 0xf] >> 8;
                    cc[counter++] = pal[(p[i+j] >> 4) & 0xf] & 0xff;					
                    cc[counter++] = pal[p[i+j] & 0xf] >> 8;
                    cc[counter++] = pal[p[i+j] & 0xf] & 0xff;					
                }
                common_hal_simpledisplay_fourwire_send(self->bus, false, (uint8_t*)&cc, pixels_per_tansfer*4);
            }        
            break;
        case FRAMEBUF_PAL256:
            pal = palette->colors;
            uint8_t pixels8_per_tansfer = 16;
            // after some tests, sending by packets of 16 pixels is the most efficient 
            // and not too greedy in terms of memory			
            for (int i=0;i<fb->buf_len;i+=pixels8_per_tansfer){
                uint8_t cc[pixels8_per_tansfer*2];
                uint8_t counter = 0;
                for(int j=0; j<pixels8_per_tansfer; j++){                
                    cc[counter++] = pal[p[i+j]] >> 8;
                    cc[counter++] = pal[p[i+j] & 0xff];
                }
                common_hal_simpledisplay_fourwire_send(self->bus, false, (uint8_t*)&cc, pixels8_per_tansfer*2);
			}		          
            break;  
        default:
            mp_raise_ValueError(translate("invalid framebuffer format"));
    }                          
}

