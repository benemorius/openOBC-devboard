file openOBC-devboard.elf
target remote | openocd -finterface/olimex-arm-usb-ocd.cfg -ftarget/lpc1768.cfg -c "gdb_port pipe; log_output /dev/null" -c "lpc1768.cpu configure -event gdb-detach { shutdown }"

monitor adapter_khz 6000
