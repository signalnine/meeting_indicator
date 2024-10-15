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