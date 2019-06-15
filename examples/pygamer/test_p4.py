import simpledisplay
import framebuf
import board
import palette

fbuf = bytearray(40*128)
palette_4 = [0x7F, 0xEF, 0xF4, 0x2A, 0x69, 0x11, 0xFA, 0x74]
colors = bytearray(palette_4)

print(colors)

tft = board.SIMPLEDISPLAY
pal = palette.Palette(colors, 4)
fb = framebuf.FrameBuffer(fbuf, 160, 128, 160, framebuf.PAL4, pal)

with open("images/test.p4", "rb") as f:
    f.readinto(fbuf)

print("Buffer len {}".format(len(fbuf)))

tft.show(fb)
