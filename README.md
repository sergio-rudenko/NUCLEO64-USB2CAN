# NUCLEO64-USB2CAN
USB (UART) to CAN bus tranciesver ([LAWICEL Protocol](http://www.can232.com/docs/can232_v3.pdf)) based on NUCLEO64-F091 boards
Can be useful with [CANhacker program](https://www.mictronics.de/img/2009/12/CANHackerV2.00.01.zip), 
Linux kernel module [slcan](https://github.com/torvalds/linux/blob/master/drivers/net/can/slcan.c) 
and DIY implementations also.

### Realized protocol commands:
- 't'	: send CAN frames
- 'Sx'	: set bitrate
- 'Zx'	: enable|disable timestamp in monitoring packets
- 'O'	: open channel
- 'C'	: close channel
- 'V', 'v': hardware and software version

Change Log
----------
See [CHANGELOG.md](CHANGELOG.md)

Links
----------
[CAN-USB адаптер из stm32vldiscovery](https://habr.com/ru/post/256493/)
[USB-CAN Bus Interface](https://www.mictronics.de/posts/USB-CAN-Bus/)
[STM32-CAN - Hardware V1.2](https://www.elektronik-keller.de/index.php/stm32-projekte1/stm32-can-v1-2)