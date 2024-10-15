# meeting_indicator
use a BBC Microbit as a visual indicator that you are in a meeting. Works on MacOS.

![meeting indicator in action](meeting_indicator.jpg?raw=true "meeting indicator")

how to use this:

compile the watcher client:
```gcc -o micstatus micstatus.c -framework CoreAudio```

upload microbit.py to your microbit

find your serial port:
```ls /dev/tty.usbmodem*```

run the executable:
```micstatus /dev/tty.usbmodem69420 9600```

The microbit will now light up any time your microphone is active (even when you're muted). Hurray!

P.S.: I included an STL file for a clip designed to allow you to easily hang the microbit off the backside of your Macbook lid, the nubs fit into the holes and it hangs upside down because it's awkward to have the USB port on the top.