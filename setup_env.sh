#!/bin/bash

echo "Setting up required environment..."
cd /sys/class/gpio
if [[ ! -d "gpio21" ]]; then
    echo 21 > export
fi
cd gpio21
sudo chmod 666 edge
echo "both" >> edge
echo "Done"
