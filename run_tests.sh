#!/bin/bash

SCRIPT_DIR=$(cd $(dirname "$0") && pwd)

usage="$(basename "$0") [-h, --help] [-p, --serial_port] [-t, --target] [-s, --save_test_output] -- Run tests on real hardware

Optional arguments:
    -h, --help              Show this help message and exit
    -p, --serial_port       Serial port to which Arduino Nano is connected to, default is /dev/ttyUSB0
    -t, --target            Path to the target binary, default is '${SCRIPT_DIR}/sw/src/raspberry_pi/main'
    -s, --save_test_output  Save the output files generated by this script, default is false"

SERIAL_PORT="/dev/ttyUSB0"
TARGET="${SCRIPT_DIR}/sw/src/raspberry_pi/main"
SAVE_TEST_OUTPUT="false"

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            echo "${usage}"
            exit 0
            ;;
        -p|--serial_port)
            SERIAL_PORT="$2"
            shift
            shift
            ;;
        -t|--target)
            TARGET="$2"
            shift
            shift
            ;;
        -s|--save_test_output)
            SAVE_TEST_OUTPUT="true"
            shift
            ;;
        *)
            echo "Unknown argument $1"
            exit 1
            ;;
    esac
done

if [[ ! -e "${TARGET}" ]]; then
    echo "Executable ${TARGET} was not found"
    exit 1
fi

sudo ${TARGET} --serial_port ${SERIAL_PORT} --run_tests
python3 ${SCRIPT_DIR}/test_data/scripts/verify_real_hw_result.py ${SCRIPT_DIR}/test_data/data

if [[ "${SAVE_TEST_OUTPUT}" == "false" ]]; then
    rm -f ${SCRIPT_DIR}/test_data/data/real_hw_tb_result.txt
fi
