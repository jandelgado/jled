# JLed for the Raspberry Pi Pico

This examples demonstrates how to use JLed on the Raspberry Pi Pico. The
built-in LED and a LED on GPIO 16 will be faded.

You need the [pico-sdk](https://github.com/raspberrypi/pico-sdk) and 
necessary tools to compile everything installed.

The `PICO_SDK_PATH` environment variable must point to the installation
directory of the SDK.

* to compile the demo sketch, run `cmake . && make` 
* To deploy the demo sketch, press and hold `BOOTSEL` on the Pico and connect
  the Pico to your PC. The Pico will now be mounted as an external drive. Copy
  the file `pico_demo.uf2` to the mount point. The sketch should now start
  automatically.

## Author 

(c) Copyright 2021 by Jan Delgado, License: MIT

