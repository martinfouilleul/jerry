#!/bin/sh

# GPIO2_3 reset
MCU_RESET=P8.8
MCU_RESET_PATH=/sys/class/gpio/gpio67/value
# GPIO2_5 boot0
MCU_BOOT0=P8.9
MCU_BOOT0_PATH=/sys/class/gpio/gpio69/value

# echo cape-universaln > /sys/devices/bone_capemgr.9/slots
# config-pin p9.24 uart
# config-pin p9.26 uart
# config-pin $MCU_BOOT0 out
# config-pin $MCU_RESET out

echo 0 > $MCU_BOOT0_PATH
echo 1 > $MCU_RESET_PATH

while true
do
inotifywait -e modify /tmp/coala.bin
echo 0 > $MCU_RESET_PATH
echo 1 > $MCU_BOOT0_PATH
echo 1 > $MCU_RESET_PATH
../stm32flash -b 115200 -w /tmp/coala.bin -v /dev/ttyO1 > /root/log 2>&1
echo 0 > $MCU_RESET_PATH
echo 0 > $MCU_BOOT0_PATH
echo 1 > $MCU_RESET_PATH
done
