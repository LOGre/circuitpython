import framebuf
import time
import board

def average(lst): 
    return sum(lst) / len(lst) 

def compute_time(fb):
    gtime = time.monotonic
    tft = board.SIMPLEDISPLAY
    blit = tft.show

    t0 = gtime()
    blit(fb)
    t1 = gtime()
    dt = t1 - t0

    return dt

def run_tests(fb):
    print("Starting tests...")

    dt = []
    for i in range(0,10):
        dt.append(compute_time(fb))

    avg_dt = average(dt)
    return avg_dt

# reserve the biggest buffer needed. To avoid gc...
fbuf = bytearray(160*128*2)
results_str = "type;freqs;average\n"

print("Loading RGB565 image...")
fb565 = framebuf.FrameBuffer(fbuf, 160, 128, 160, framebuf.RGB565)
f = open("images/test.r565", "rb")
f.readinto(fbuf)
f.close()

avg_dt = run_tests(fb565)/1000
print("average blit time: {}ms".format(avg_dt))
results_str += "{};{}\n".format("rgb565", avg_dt)

print(results_str)
print("Done!!!")
