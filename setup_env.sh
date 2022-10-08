#!/bin/bash

cd /sys/class/gpio
if [[ ! -d "gpio21" ]]; then
    echo 21 > export
fi
cd gpio21
sudo chmod 666 edge
echo "both" >> edge
