from machine import Pin, SPI, SDCard, ADC, idle
import os

# https://github.com/rdagger/micropython-ili9341
from ili9341 import Display, color565
from xpt2046 import Touch
    
# Set up SPI for display
# Baud rate of 80000000 seems about the max
display_spi = SPI(1, baudrate=80000000, sck=Pin(14), mosi=Pin(13))

# Set up display
# The library needs a reset pin, which does not exist on this board
display = Display(display_spi, dc=Pin(2), cs=Pin(15), rst=Pin(15))

# Turn on display backlight
backlight = Pin(21, Pin.OUT)
backlight.on()

# Clear display to yellow
display.clear(color565(255, 255, 0))

# RGB LED at the back
red_led = Pin(4, Pin.OUT)
green_led = Pin(16, Pin.OUT)
blue_led = Pin(17, Pin.OUT)

# Turn on all LEDs (active low)
# RGB LED (and backlight) will also work with machine.PWM for dimming

red_led.off()
green_led.off()
blue_led.off()

# Set up SD card 
sd = SDCard(slot=2, sck=Pin(18), miso=Pin(19), mosi=Pin(23), cs=Pin(5))
# Print SD card info (seems to be card size and sector size?)
print(sd.info())

# Mount SD card and print directory listing
# SD card must be formatted with a file system recognised by ESP32 (FAT)
os.mount(sd, "/sd")
print(os.listdir("/sd"))

# Read light sensor
lightsensor = ADC(34, atten=ADC.ATTN_0DB)
print(lightsensor.read_uv())



# Read touch screen
touch_spi = SPI(1, baudrate=1000000, sck=Pin(25), mosi=Pin(32), miso=Pin(39))

def touchscreen_press(x, y):
    print("Touch at " + str(x) + "," + str(y))

touch = Touch(touch_spi, cs=Pin(33), int_pin=Pin(36), int_handler=touchscreen_press)

# loop to wait for touchscreen test
try:
    while True:
        touch.get_touch()

except KeyboardInterrupt:
    print("\nCtrl-C pressed.  Cleaning up and exiting...")
finally:
    display.cleanup()
