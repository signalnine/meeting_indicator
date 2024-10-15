from microbit import *

# Set up the serial connection with the Mac
uart.init(baudrate=9600)

def flip_image(image):
    new = Image(5, 5)
    for y in range(5):
        for x in range(5):
            new.set_pixel(x, 4 - y, image.get_pixel(x, y))
    return new
    
# Main loop
while True:
    # Check if data is available over serial
    if uart.any():
        signal = uart.read(1)  # Read one byte from the serial input
        
        if signal == b'1':
            # display a square on the onboard matrix
            # I am flipping the image upside down because sometimes I use a different image and the wires are less messy in this orientation
            display.show(flip_image(Image.SQUARE))
        elif signal == b'0':
            # Turn off display
            display.clear()
            
    sleep(100)  # Short delay for stability
