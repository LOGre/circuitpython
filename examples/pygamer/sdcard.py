import sys
import os
import adafruit_sdcard
import board
import busio
import digitalio
import storage

# Connect to the card and mount the filesystem.
spi = busio.SPI(board.SCK, board.MOSI, board.MISO)
cs = digitalio.DigitalInOut(board.SD_CS)
sdcard = adafruit_sdcard.SDCard(spi, cs)
vfs = storage.VfsFat(sdcard)
storage.mount(vfs, "/sd")
sys.path.append("/sd")
os.chdir("/sd")

buf = bytearray(160*128)
f = open('test.dat', 'rb')
f.readinto(buf)
f.close()
