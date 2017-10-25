# exixe clock: nanosecond-precision GPS-disciplined Nixie tube clock

![Alt text](https://i.imgur.com/I1maqhw.jpg)

exixe clock is a GNSS-aided high precision Nixie tube clock. With GPS fix acquired, displayed time would be within 60 **nanoseconds** of the atomic clocks onboard the satellites.

exixe clock makes use of the exixe modules, and works with both IN-14 and IN-12 tubes. It also comes with a wealth of sensors and interfaces on-board, allowing it to be hacked beyond just a timepiece.

## Features

### Precision Timing

* 60ns with GPS fix
* 2ppm without GPS fix
* Auto time acquisition
* Battery backup

### Simple & modular construction

* Uses exixe driver module
* Works with both IN-12 and IN-14 tubes
* Single 5V supply from USB
* Off-the-shelf high voltage supply module

### Versatile interfaces

* USB data transfer / firmware upload
* SD card slot
* Temperature sensor
* Light sensor
* UART extension header
* GNSS extension header
* 2 capacitive touch buttons
* 2 physical buttons
* Fix indicator LED

## Technical details

I was looking through existing Nixie clock projects before starting my own, and I'm rather unconvinced with some trends that many have in common:

* Using through hole components

While there's technically nothing wrong, I feel that in this day and age using PTH looks bulky and unprofessional.

* 12V supply

I want my clock to be powered by a single USB cable, using a 12V powerbrick is both cumbersome and complicates the design.

* Fixed and multiplexed tubes

This is my biggest gripe, with soldered down tubes, it's almost impossible to replace if one breaks, or to repurpose them for other projects. And as Nixie tubes have been out of productions for 30 years, they should be conserved as much as possible.

And as a result of fixed tubes, they are almost always multiplexed.

* Gimmick features

Who actually wants to set a alarm on a Nixie clock though 200 presses on a single button?

* Cheap RTC

Arduino plus a $3 I2C RTC module 

actual timing accuracy is often ignored

 traditional approach, modular exixe modules, actually focus on timing

ublox m8q precision timing

## User guide

