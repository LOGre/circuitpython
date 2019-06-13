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

#include <stdint.h>

#include "py/obj.h"
#include "py/runtime.h"

#include "shared-bindings/simpledisplay/__init__.h"
#include "shared-bindings/simpledisplay/Display.h"
#include "shared-bindings/simpledisplay/FourWire.h"


//| :mod:`simpledisplay` --- Native display driving
//| =========================================================================
//|
//| .. module:: simpledisplay
//|   :synopsis: Native helpers for driving displays
//|   :platform: SAMD21, SAMD51, nRF52
//|
//| The `simpledisplay` module contains classes to manage display output
//| including synchronizing with refresh rates and partial updating.
//|
//| Libraries
//|
//| .. toctree::
//|     :maxdepth: 3
//|
//|     Display
//|     FourWire
//|

STATIC const mp_rom_map_elem_t simpledisplay_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_simpledisplay) },
    { MP_ROM_QSTR(MP_QSTR_SimpleDisplay), MP_ROM_PTR(&simpledisplay_display_type) },
    { MP_ROM_QSTR(MP_QSTR_FourWire), MP_ROM_PTR(&simpledisplay_fourwire_type) },
};

STATIC MP_DEFINE_CONST_DICT(simpledisplay_module_globals, simpledisplay_module_globals_table);

const mp_obj_module_t simpledisplay_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&simpledisplay_module_globals,
};
