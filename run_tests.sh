#!/bin/bash

if [[ $# -ne 1 ]]; then
   echo "Number of arguments provided was not 1."
   exit 0
fi

if [[ "$1" == "-h" ]]; then
    echo "Usage: run tests on real HW. Requires the serial port to Arduino as an argument."
    exit 0
fi

if [[ ! -e "`dirname "$0"`/gpio_program" ]]; then
    echo "Executable gpio_program does not exist."
    exit 0
fi

sudo `dirname "$0"`/gpio_program $1 testing
python3 `dirname "$0"`/verify_real_hw_result.py `dirname "$0"`
rm -f `dirname "$0"`/real_hw_tb_result.txt
