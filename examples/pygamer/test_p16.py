import simpledisplay
import framebuf
import board
import palette

print("Buffers")

fbuf = bytearray(80*128)

# warning, MSB... 16bits RGB565 should be swapped
c64_cols = [
            0x00, 0x00, 0xFF, 0xFF, 0xC6, 0x89, 0xB7, 0x65, 0xF2, 0x89, 0x09, 0x55, 0x91, 0x41, 0x6E, 0xBE, 
            0xA5, 0x8A, 0x00, 0x52, 0x4C, 0xBB, 0x8A, 0x52, 0xCF, 0x7B, 0x11, 0x97, 0x58, 0x7B, 0xF3, 0x9C
           ] 
colors = bytearray(c64_cols)

print(colors)

tft = board.SIMPLEDISPLAY
pal = palette.Palette(colors, 16)
fb = framebuf.FrameBuffer(fbuf, 160, 128, 160, framebuf.PAL16, pal)

with open("images/test.p16", "rb") as f:
    f.readinto(fbuf)

print("Buffer len {}".format(len(fbuf)))

tft.show(fb)
