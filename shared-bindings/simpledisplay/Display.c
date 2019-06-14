/*
 * This file is part of the Micro Python project, http://micropython.org/
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

#include <stdint.h>
#include <stdio.h>

#include "lib/utils/context_manager_helpers.h"
#include "py/binary.h"
#include "py/objproperty.h"
#include "py/objtype.h"
#include "py/runtime.h"

#include "shared-bindings/microcontroller/Pin.h"
#include "shared-bindings/util.h"
#include "shared-bindings/board/__init__.h"
#include "shared-bindings/simpledisplay/FourWire.h"
#include "shared-bindings/simpledisplay/Display.h"

#include "shared-module/simpledisplay/__init__.h"

//|   .. method:: Python constructor(4wireSPI, screen width, screen height)
//|
//|     The driver will be created and the screen will be initialized
//|
STATIC mp_obj_t simpledisplay_display_make_new(const mp_obj_type_t *type, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_display_bus, ARG_width, ARG_height };
    
    // Check python arguments
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_display_bus, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_width, MP_ARG_INT | MP_ARG_KW_ONLY | MP_ARG_REQUIRED, },
        { MP_QSTR_height, MP_ARG_INT | MP_ARG_KW_ONLY | MP_ARG_REQUIRED, },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // Retrieve 4wireSPI bus object
    mp_obj_t display_bus = args[ARG_display_bus].u_obj;

    // create driver if not done at boot, SHOULD NOT!
    simpledisplay_display_obj_t *screen = &board_display_obj;
    if(screen == NULL) {
        screen = m_new_obj(simpledisplay_display_obj_t);
        screen->base.type = &simpledisplay_display_type;
        common_hal_simpledisplay_display_construct(screen,
            display_bus, 
            args[ARG_width].u_int, 
            args[ARG_height].u_int);
    }

    return screen;
}

//|   .. method:: show()
//|
//|     Send data to the driver to be displayed
//|
STATIC mp_obj_t simpledisplay_display_obj_show(mp_obj_t self, mp_obj_t fb_obj) {
    
    printf("simpledisplay_display_obj_show\n");

    // get the driver pointer based on the python object
    simpledisplay_display_obj_t *screen = MP_OBJ_TO_PTR(self);

    printf("screen ptr: %p width: %d height: %d\n", screen, screen->width, screen->height);

    // get the fb pointer based on the python object
    mp_obj_framebuf_t *fb = MP_OBJ_TO_PTR(fb_obj);

    // Not sure why I need to do it after the framebuf make_new.... else buf is empty
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(fb_obj, &bufinfo, MP_BUFFER_READ);
    fb->buf = bufinfo.buf;
    fb->buf_len = bufinfo.len;
    
    printf("fb ptr: %p format: %d width: %d len: %d\n", fb, fb->format, fb->width, fb->buf_len);

    if(fb->format != FRAMEBUF_RGB565) {
        mp_obj_palette_t * pal = fb->palette;
        printf("palette: %p colors: %d\n", pal, pal->nb_colors);
    }

    // call module implementation
    common_hal_simpledisplay_display_show(screen, fb);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(simpledisplay_display_show_obj, simpledisplay_display_obj_show);


//|   .. attribute:: bus
//|
//|	The bus being used by the display
//|
//|
STATIC mp_obj_t simpledisplay_display_obj_get_bus(mp_obj_t self) {
    simpledisplay_display_obj_t *screen = MP_OBJ_TO_PTR(self);
    return screen->bus;
}
MP_DEFINE_CONST_FUN_OBJ_1(simpledisplay_display_get_bus_obj, simpledisplay_display_obj_get_bus);

const mp_obj_property_t simpledisplay_display_bus_obj = {
    .base.type = &mp_type_property,
    .proxy = {(mp_obj_t)&simpledisplay_display_get_bus_obj,
              (mp_obj_t)&mp_const_none_obj,
              (mp_obj_t)&mp_const_none_obj},
};


STATIC const mp_rom_map_elem_t simpledisplay_display_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_show), MP_ROM_PTR(&simpledisplay_display_show_obj) },
    { MP_ROM_QSTR(MP_QSTR_bus), MP_ROM_PTR(&simpledisplay_display_bus_obj) },
};
STATIC MP_DEFINE_CONST_DICT(simpledisplay_display_locals_dict, simpledisplay_display_locals_dict_table);

const mp_obj_type_t simpledisplay_display_type = {
    { &mp_type_type },
    .name = MP_QSTR_SimpleDisplay,
    .make_new = simpledisplay_display_make_new,
    .locals_dict = (mp_obj_dict_t*)&simpledisplay_display_locals_dict,
};
