#!/bin/sh

# GPIO2_3 reset
MCU_RESET_PATH=/sys/class/gpio/gpio67/value

echo 0 > $MCU_RESET_PATH
echo 1 > $MCU_RESET_PATH

