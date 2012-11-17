file openOBC-devboard.elf
target remote | openocd -finterface/arm-usb-ocd.cfg -f board/mcb1700.cfg -c "gdb_port pipe; log_output /dev/null" -c "lpc1768.cpu configure -event gdb-detach { reset; shutdown }"
monitor reset init
thb main

