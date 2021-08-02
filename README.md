Digital Theremin
================

The Digital Theremin uses the on board accelerometer on the STM32F4 Discovery Evaluation Board to control the pitch and volume of the sine wave. The sound is output through the headphone jack via the DAC.

To keep a continuous sine wave running a wavetable is generated on start up which is processed into a DMA double buffer to be read by the DAC.

> Configured for the STM32F401C-DISCO board.

Dependencies
------------

```
brew tap ArmMbed/homebrew-formulae
brew install arm-none-eabi-gcc openocd stlink
```

Usage
-----

To build the project

```
make
```

To flash the board

```
make flash
```

To run gdb (-g must be added to CFLAGS)

```
make debug
```
