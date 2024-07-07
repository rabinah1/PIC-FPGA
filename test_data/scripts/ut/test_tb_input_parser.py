# pylint: disable=missing-docstring
import os
import sys
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
# pylint: disable=import-error
# pylint: disable=wrong-import-position
from tb_input_parser import decimal_to_binary, parse  # noqa: E402


def test_decimal_to_binary():
    decimal_values = {0: 3, 1: 2, 45: 6, 89: 9, 123: 7, 23: 5, 583: 10, 14: 4}
    reference_binary_values = ["000", "01", "101101", "001011001", "1111011", "10111",
                               "1001000111", "1110"]
    idx = 0
    for decimal, num_bits in decimal_values.items():
        assert decimal_to_binary(decimal, num_bits) == reference_binary_values[idx]
        idx += 1


def test_parse():
    assert parse(["RESET"]) == "RESET"
    assert parse(["# Test N"]) == "# Test N"
    assert parse(["ADDLW", "2"]) == "11111000000010"
    assert parse(["INCF", "1", "15"]) == "00101010001111"
    assert parse(["BCF", "3", "100"]) == "01000111100100"


sys.path.pop(0)
