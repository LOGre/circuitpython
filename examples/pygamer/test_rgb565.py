import simpledisplay
import framebuf
import board
import palette

print("Buffers")

fbuf = bytearray(160*128*2)

tft = board.SIMPLEDISPLAY
fb = framebuf.FrameBuffer(fbuf, 160, 128, 160, framebuf.RGB565)

with open("images/test.r565", "rb") as f:
    f.readinto(fbuf)

print("Buffer len {}".format(len(fbuf)))

tft.show(fb)
