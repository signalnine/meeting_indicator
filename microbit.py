from microbit import *

# Set up the serial connection with the Mac
uart.init(baudrate=9600)

# Main loop
while True:
    # Check if data is available over serial
    if uart.any():
        signal = uart.read(1)  # Read one byte from the serial input
        
        if signal == b'1':
            # display a square on the onboard matrix
            display.show(Image.SQUARE)
        elif signal == b'0':
            # Turn off display
            display.clear()
            
    sleep(100)  # Short delay for stability
