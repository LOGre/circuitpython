/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Damien P. George
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

#include <stdio.h>
#include <string.h>

#include "py/runtime.h"
#include "modframebuf.h"

#if MICROPY_PY_FRAMEBUF

#include "ports/stm32/font_petme128_8x8.h"

// Functions for RGB565 format
void rgb565_setpixel(const mp_obj_framebuf_t *fb, int x, int y, uint32_t col) {
    col = COL(col);
    uint16_t color = ((col&0xff) << 8) | ((col >> 8) & 0xff);
    ((uint16_t*)fb->buf)[x + y * fb->stride] = color;
}

uint32_t rgb565_getpixel(const mp_obj_framebuf_t *fb, int x, int y) {
    return ((uint16_t*)fb->buf)[x + y * fb->stride];
}

void rgb565_fill_rect(const mp_obj_framebuf_t *fb, int x, int y, int w, int h, uint32_t col) {
    col = COL(col);
    uint16_t color = ((col&0xff) << 8) | ((col >> 8) & 0xff);
    uint16_t *b = &((uint16_t*)fb->buf)[x + y * fb->stride];
    while (h--) {
        for (int ww = w; ww; --ww) {
            *b++ = color;
        }
        b += fb->stride - w;
    }
}

// Functions for PAL4 format
void pal4_setpixel(const mp_obj_framebuf_t *fb, int x, int y, uint32_t col) {
    uint8_t *pixel = &((uint8_t*)fb->buf)[(x + y * fb->stride) >> 1];
    switch (x % 4) {
        case 0:
            *pixel = ((uint8_t)col & 0x03) | (*pixel & 0x03);
            break;
        case 1:
            *pixel = ((uint8_t)col << 2) | (*pixel & 0x03);
            break;
        case 2:
            *pixel = ((uint8_t)col << 4) | (*pixel & 0x03);
            break;
        case 3:
            *pixel = ((uint8_t)col << 6) | (*pixel & 0x03);
            break;                        
    }
}

uint32_t pal4_getpixel(const mp_obj_framebuf_t *fb, int x, int y) {
    switch (x % 4) {
        case 0:
            return ((uint8_t*)fb->buf)[(x + y * fb->stride) >> 2] & 0x03;
            break;
        case 1:
            return ((uint8_t*)fb->buf)[(x + y * fb->stride) >> 2] >> 2 & 0x03;
            break;
        case 2:
            return ((uint8_t*)fb->buf)[(x + y * fb->stride) >> 2] >> 4 & 0x03;
            break;
        case 3:
            return ((uint8_t*)fb->buf)[(x + y * fb->stride) >> 2] >> 6 & 0x03;
            break;                        
    }
    return ((uint8_t*)fb->buf)[(x + y * fb->stride) >> 2] & 0x03; 
}

void pal4_fill_rect(const mp_obj_framebuf_t *fb, int x, int y, int w, int h, uint32_t col) {
    for (int xx=x; xx < x+w; xx++) {
        for (int yy=y; yy < y+h; yy++) {
            pal4_setpixel(fb, xx, yy, col);
        }
    }
}

// Functions for PAL16 format
void pal16_setpixel(const mp_obj_framebuf_t *fb, int x, int y, uint32_t col) {
    uint8_t *pixel = &((uint8_t*)fb->buf)[(x + y * fb->stride) >> 1];
    if(x & 1) {
        *pixel = ((uint8_t)col & 0x0f) | (*pixel & 0xf0);
    } else {
        *pixel = ((uint8_t)col << 4) | (*pixel & 0x0f);
    }
}

uint32_t pal16_getpixel(const mp_obj_framebuf_t *fb, int x, int y) {
    if(x & 1) {
        return ((uint8_t*)fb->buf)[(x + y * fb->stride) >> 1] & 0x0f;
    }
    return ((uint8_t*)fb->buf)[(x + y * fb->stride) >> 1] >> 4;
}

void pal16_fill_rect(const mp_obj_framebuf_t *fb, int x, int y, int w, int h, uint32_t col) {
    for (int xx=x; xx < x+w; xx++) {
        for (int yy=y; yy < y+h; yy++) {
            pal16_setpixel(fb, xx, yy, col);
        }
    }
}

// Functions for PAL256 format
void pal256_setpixel(const mp_obj_framebuf_t *fb, int x, int y, uint32_t col) {
    uint8_t *pixel = &((uint8_t*)fb->buf)[(x + y * fb->stride) >> 1];
    *pixel = ((uint8_t)col);
}

uint32_t pal256_getpixel(const mp_obj_framebuf_t *fb, int x, int y) {
    return ((uint8_t*)fb->buf)[(x + y * fb->stride) >> 1];
}

void pal256_fill_rect(const mp_obj_framebuf_t *fb, int x, int y, int w, int h, uint32_t col) {
    for (int xx=x; xx < x+w; xx++) {
        for (int yy=y; yy < y+h; yy++) {
            pal256_setpixel(fb, xx, yy, col);
        }
    }
}


void fill_rect(const mp_obj_framebuf_t *fb, int x, int y, int w, int h, uint32_t col) {
    if (h < 1 || w < 1 || x + w <= 0 || y + h <= 0 || y >= fb->height || x >= fb->width) {
        // No operation needed.
        return;
    }

    // clip to the framebuffer
    int xend = MIN(fb->width, x + w);
    int yend = MIN(fb->height, y + h);
    x = MAX(x, 0);
    y = MAX(y, 0);

    formats[fb->format].fill_rect(fb, x, y, xend - x, yend - y, col);
}

// arguments
// 0. python object with buffer
// 1. width 
// 2. height
// 3. stride
// 4. format
// 5. palette object
STATIC mp_obj_t framebuf_make_new(const mp_obj_type_t *type, size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    mp_arg_check_num(n_args, kw_args, 4, 6, false);

    mp_obj_framebuf_t *o = m_new_obj(mp_obj_framebuf_t);
    o->base.type = type;
    o->buf_obj = args[0];

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[0], &bufinfo, MP_BUFFER_WRITE);
    o->buf = bufinfo.buf;

    o->width = mp_obj_get_int(args[1]);
    o->height = mp_obj_get_int(args[2]);
    o->stride = mp_obj_get_int(args[3]);
    o->format = mp_obj_get_int(args[4]);    

    switch (o->format) {
        case FRAMEBUF_MONO:
        case FRAMEBUF_RGB565:
            break;
        case FRAMEBUF_PAL4:
        case FRAMEBUF_PAL16:
        case FRAMEBUF_PAL256:
            if (n_args == 6) {
                o->palette = MP_OBJ_TO_PTR(args[5]);       
            }  
            else {
                mp_raise_ValueError(translate("Missing palette"));   
            }        
            break;
        default:
            printf("format: %d\n", o->format);
            mp_raise_ValueError(translate("invalid framebuffer format"));
    }

    return MP_OBJ_FROM_PTR(o);
}

STATIC mp_int_t framebuf_get_buffer(mp_obj_t self_in, mp_buffer_info_t *bufinfo, mp_uint_t flags) {
 (void)flags;
    mp_obj_framebuf_t *self = MP_OBJ_TO_PTR(self_in);
    bufinfo->buf = self->buf;
    float bpp = 0;
    switch(self->format) {
        case FRAMEBUF_RGB565:
            bpp = 2.0;
            break;
        case FRAMEBUF_PAL256:
            bpp = 1.0;
            break;
        case FRAMEBUF_PAL16:
            bpp = 0.5;    
            break;          
        case FRAMEBUF_PAL4:
            bpp = 0.25;    
            break;                
    }
    bufinfo->len = (self->stride * self->height) * bpp;
    bufinfo->typecode = 'B'; // view framebuf as bytes
    return 0;
}

STATIC mp_obj_t framebuf_fill(mp_obj_t self_in, mp_obj_t col_in) {
    mp_obj_framebuf_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t col = mp_obj_get_int(col_in);
    formats[self->format].fill_rect(self, 0, 0, self->width, self->height, col);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(framebuf_fill_obj, framebuf_fill);

STATIC mp_obj_t framebuf_fill_rect(size_t n_args, const mp_obj_t *args) {
    (void)n_args;

    mp_obj_framebuf_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_int_t x = mp_obj_get_int(args[1]);
    mp_int_t y = mp_obj_get_int(args[2]);
    mp_int_t width = mp_obj_get_int(args[3]);
    mp_int_t height = mp_obj_get_int(args[4]);
    mp_int_t col = mp_obj_get_int(args[5]);

    fill_rect(self, x, y, width, height, col);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuf_fill_rect_obj, 6, 6, framebuf_fill_rect);

STATIC mp_obj_t framebuf_pixel(size_t n_args, const mp_obj_t *args) {
    mp_obj_framebuf_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_int_t x = mp_obj_get_int(args[1]);
    mp_int_t y = mp_obj_get_int(args[2]);
    if (0 <= x && x < self->width && 0 <= y && y < self->height) {
        if (n_args == 3) {
            // get
            return MP_OBJ_NEW_SMALL_INT(getpixel(self, x, y));
        } else {
            // set
            setpixel(self, x, y, mp_obj_get_int(args[3]));
        }
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuf_pixel_obj, 3, 4, framebuf_pixel);

STATIC mp_obj_t framebuf_hline(size_t n_args, const mp_obj_t *args) {
    (void)n_args;

    mp_obj_framebuf_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_int_t x = mp_obj_get_int(args[1]);
    mp_int_t y = mp_obj_get_int(args[2]);
    mp_int_t w = mp_obj_get_int(args[3]);
    mp_int_t col = mp_obj_get_int(args[4]);

    fill_rect(self, x, y, w, 1, col);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuf_hline_obj, 5, 5, framebuf_hline);

STATIC mp_obj_t framebuf_vline(size_t n_args, const mp_obj_t *args) {
    (void)n_args;

    mp_obj_framebuf_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_int_t x = mp_obj_get_int(args[1]);
    mp_int_t y = mp_obj_get_int(args[2]);
    mp_int_t h = mp_obj_get_int(args[3]);
    mp_int_t col = mp_obj_get_int(args[4]);

    fill_rect(self, x, y, 1, h, col);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuf_vline_obj, 5, 5, framebuf_vline);

STATIC mp_obj_t framebuf_rect(size_t n_args, const mp_obj_t *args) {
    (void)n_args;

    mp_obj_framebuf_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_int_t x = mp_obj_get_int(args[1]);
    mp_int_t y = mp_obj_get_int(args[2]);
    mp_int_t w = mp_obj_get_int(args[3]);
    mp_int_t h = mp_obj_get_int(args[4]);
    mp_int_t col = mp_obj_get_int(args[5]);

    fill_rect(self, x, y, w, 1, col);
    fill_rect(self, x, y + h- 1, w, 1, col);
    fill_rect(self, x, y, 1, h, col);
    fill_rect(self, x + w- 1, y, 1, h, col);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuf_rect_obj, 6, 6, framebuf_rect);

STATIC mp_obj_t framebuf_line(size_t n_args, const mp_obj_t *args) {
    (void)n_args;

    mp_obj_framebuf_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_int_t x1 = mp_obj_get_int(args[1]);
    mp_int_t y1 = mp_obj_get_int(args[2]);
    mp_int_t x2 = mp_obj_get_int(args[3]);
    mp_int_t y2 = mp_obj_get_int(args[4]);
    mp_int_t col = mp_obj_get_int(args[5]);

    mp_int_t dx = x2 - x1;
    mp_int_t sx;
    if (dx > 0) {
        sx = 1;
    } else {
        dx = -dx;
        sx = -1;
    }

    mp_int_t dy = y2 - y1;
    mp_int_t sy;
    if (dy > 0) {
        sy = 1;
    } else {
        dy = -dy;
        sy = -1;
    }

    bool steep;
    if (dy > dx) {
        mp_int_t temp;
        temp = x1; x1 = y1; y1 = temp;
        temp = dx; dx = dy; dy = temp;
        temp = sx; sx = sy; sy = temp;
        steep = true;
    } else {
        steep = false;
    }

    mp_int_t e = 2 * dy - dx;
    for (mp_int_t i = 0; i < dx; ++i) {
        if (steep) {
            if (0 <= y1 && y1 < self->width && 0 <= x1 && x1 < self->height) {
                setpixel(self, y1, x1, col);
            }
        } else {
            if (0 <= x1 && x1 < self->width && 0 <= y1 && y1 < self->height) {
                setpixel(self, x1, y1, col);
            }
        }
        while (e >= 0) {
            y1 += sy;
            e -= 2 * dx;
        }
        x1 += sx;
        e += 2 * dy;
    }

    if (0 <= x2 && x2 < self->width && 0 <= y2 && y2 < self->height) {
        setpixel(self, x2, y2, col);
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuf_line_obj, 6, 6, framebuf_line);

STATIC mp_obj_t framebuf_blit(size_t n_args, const mp_obj_t *args) {
    mp_obj_framebuf_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_obj_framebuf_t *source = MP_OBJ_TO_PTR(args[1]);
    mp_int_t x = mp_obj_get_int(args[2]);
    mp_int_t y = mp_obj_get_int(args[3]);
    mp_int_t key = -1;
    if (n_args > 4) {
        key = mp_obj_get_int(args[4]);
    }

    if (
        (x >= self->width) ||
        (y >= self->height) ||
        (-x >= source->width) ||
        (-y >= source->height)
    ) {
        // Out of bounds, no-op.
        return mp_const_none;
    }

    // Clip.
    int x0 = MAX(0, x);
    int y0 = MAX(0, y);
    int x1 = MAX(0, -x);
    int y1 = MAX(0, -y);
    int x0end = MIN(self->width, x + source->width);
    int y0end = MIN(self->height, y + source->height);

    for (; y0 < y0end; ++y0) {
        int cx1 = x1;
        for (int cx0 = x0; cx0 < x0end; ++cx0) {
            uint32_t col = getpixel(source, cx1, y1);
            if (col != (uint32_t)key) {
                setpixel(self, cx0, y0, col);
            }
            ++cx1;
        }
        ++y1;
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuf_blit_obj, 4, 5, framebuf_blit);

STATIC mp_obj_t framebuf_scroll(mp_obj_t self_in, mp_obj_t xstep_in, mp_obj_t ystep_in) {
    mp_obj_framebuf_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t xstep = mp_obj_get_int(xstep_in);
    mp_int_t ystep = mp_obj_get_int(ystep_in);
    int sx, y, xend, yend, dx, dy;
    if (xstep < 0) {
        sx = 0;
        xend = self->width + xstep;
        dx = 1;
    } else {
        sx = self->width - 1;
        xend = xstep - 1;
        dx = -1;
    }
    if (ystep < 0) {
        y = 0;
        yend = self->height + ystep;
        dy = 1;
    } else {
        y = self->height - 1;
        yend = ystep - 1;
        dy = -1;
    }
    for (; y != yend; y += dy) {
        for (int x = sx; x != xend; x += dx) {
            setpixel(self, x, y, getpixel(self, x - xstep, y - ystep));
        }
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(framebuf_scroll_obj, framebuf_scroll);

STATIC mp_obj_t framebuf_text(size_t n_args, const mp_obj_t *args) {
    // extract arguments
    mp_obj_framebuf_t *self = MP_OBJ_TO_PTR(args[0]);
    const char *str = mp_obj_str_get_str(args[1]);
    mp_int_t x0 = mp_obj_get_int(args[2]);
    mp_int_t y0 = mp_obj_get_int(args[3]);
    mp_int_t col = 1;
    if (n_args >= 5) {
        col = mp_obj_get_int(args[4]);
    }

    // loop over chars
    for (; *str; ++str) {
        // get char and make sure its in range of font
        int chr = *(uint8_t*)str;
        if (chr < 32 || chr > 127) {
            chr = 127;
        }
        // get char data
        const uint8_t *chr_data = &font_petme128_8x8[(chr - 32) * 8];
        // loop over char data
        for (int j = 0; j < 8; j++, x0++) {
            if (0 <= x0 && x0 < self->width) { // clip x
                uint vline_data = chr_data[j]; // each byte is a column of 8 pixels, LSB at top
                for (int y = y0; vline_data; vline_data >>= 1, y++) { // scan over vertical column
                    if (vline_data & 1) { // only draw if pixel set
                        if (0 <= y && y < self->height) { // clip y
                            setpixel(self, x0, y, col);
                        }
                    }
                }
            }
        }
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuf_text_obj, 4, 5, framebuf_text);


/* -------------------------------------------------------------------------------------- */
// colors[]
// nb colors
STATIC mp_obj_t palette_make_new(const mp_obj_type_t *type, size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    mp_arg_check_num(n_args, kw_args, 2, 2, false);

    mp_obj_palette_t *o = m_new_obj(mp_obj_palette_t);
    o->base.type = type;
    o->buf_obj = args[0];

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[0], &bufinfo, MP_BUFFER_WRITE);
    o->colors = bufinfo.buf;
    o->nb_colors = mp_obj_get_int(args[1]);

    return MP_OBJ_FROM_PTR(o);
}

STATIC mp_int_t palette_get_buffer(mp_obj_t self_in, mp_buffer_info_t *bufinfo, mp_uint_t flags) {
    (void)flags;
    mp_obj_palette_t *self = MP_OBJ_TO_PTR(self_in);
    bufinfo->buf = self->colors;
    bufinfo->len = self->nb_colors*2;
    bufinfo->typecode = 'B'; // view framebuf as bytes
    return 0;
}

/* -------------------------------------------------------------------------------------- */

STATIC const mp_rom_map_elem_t framebuf_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_fill), MP_ROM_PTR(&framebuf_fill_obj) },
    { MP_ROM_QSTR(MP_QSTR_fill_rect), MP_ROM_PTR(&framebuf_fill_rect_obj) },
    { MP_ROM_QSTR(MP_QSTR_pixel), MP_ROM_PTR(&framebuf_pixel_obj) },
    { MP_ROM_QSTR(MP_QSTR_hline), MP_ROM_PTR(&framebuf_hline_obj) },
    { MP_ROM_QSTR(MP_QSTR_vline), MP_ROM_PTR(&framebuf_vline_obj) },
    { MP_ROM_QSTR(MP_QSTR_rect), MP_ROM_PTR(&framebuf_rect_obj) },
    { MP_ROM_QSTR(MP_QSTR_line), MP_ROM_PTR(&framebuf_line_obj) },
    { MP_ROM_QSTR(MP_QSTR_blit), MP_ROM_PTR(&framebuf_blit_obj) },
    { MP_ROM_QSTR(MP_QSTR_scroll), MP_ROM_PTR(&framebuf_scroll_obj) },
    { MP_ROM_QSTR(MP_QSTR_text), MP_ROM_PTR(&framebuf_text_obj) },
};
STATIC MP_DEFINE_CONST_DICT(framebuf_locals_dict, framebuf_locals_dict_table);

STATIC const mp_obj_type_t mp_type_framebuf = {
    { &mp_type_type },
    .name = MP_QSTR_FrameBuffer,
    .make_new = framebuf_make_new,
    .buffer_p = { .get_buffer = framebuf_get_buffer },
    .locals_dict = (mp_obj_dict_t*)&framebuf_locals_dict,
};

STATIC const mp_rom_map_elem_t framebuf_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),    MP_ROM_QSTR(MP_QSTR_framebuf) },
    { MP_ROM_QSTR(MP_QSTR_FrameBuffer), MP_ROM_PTR(&mp_type_framebuf) },
    { MP_ROM_QSTR(MP_QSTR_RGB565),      MP_ROM_INT(FRAMEBUF_RGB565) },
    { MP_ROM_QSTR(MP_QSTR_PAL4),       MP_ROM_INT(FRAMEBUF_PAL4) },
    { MP_ROM_QSTR(MP_QSTR_PAL16),       MP_ROM_INT(FRAMEBUF_PAL16) },
    { MP_ROM_QSTR(MP_QSTR_PAL256),      MP_ROM_INT(FRAMEBUF_PAL256) },
};

STATIC MP_DEFINE_CONST_DICT(framebuf_module_globals, framebuf_module_globals_table);

const mp_obj_module_t mp_module_framebuf = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&framebuf_module_globals,
};

/* -------------------------------------------------------------------------------------- */

//STATIC const mp_rom_map_elem_t palette_locals_dict_table[] = {
//    { MP_ROM_QSTR(MP_QSTR_), MP_ROM_PTR(&framebuf_fill_obj) },
//};
//STATIC MP_DEFINE_CONST_DICT(palette_locals_dict, palette_locals_dict_table);

STATIC const mp_obj_type_t mp_type_palette = {
    { &mp_type_type },
    .name = MP_QSTR_Palette,
    .make_new = palette_make_new,
    .buffer_p = { .get_buffer = palette_get_buffer },
//    .locals_dict = (mp_obj_dict_t*)&palette_locals_dict,
};

STATIC const mp_rom_map_elem_t palette_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),    MP_ROM_QSTR(MP_QSTR_palette) },
    { MP_ROM_QSTR(MP_QSTR_Palette), MP_ROM_PTR(&mp_type_palette) },    
};

STATIC MP_DEFINE_CONST_DICT(palette_module_globals, palette_module_globals_table);

const mp_obj_module_t mp_module_palette = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&palette_module_globals,
};

#endif // MICROPY_PY_FRAMEBUF
