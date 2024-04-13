# pylint: disable=missing-docstring
from verify_simulation_result import _binary_to_decimal


def test_binary_to_decimal():
    binaries = ["00000001", "00000010", "00000100", "00001000", "00010000", "00100000", "01000000",
                "10000000", "00000011", "00000101", "00001001", "00010001", "00100001", "01000001",
                "10000001", "00000110", "00001010", "00010010", "00100010", "01000010", "10000010",
                "00000000"]
    decimals = [1, 2, 4, 8, 16, 32, 64, 128, 3, 5, 9, 17, 33, 65, 129, 6, 10, 18, 34, 66, 130, 0]
    for binary, decimal in zip(binaries, decimals):
        assert _binary_to_decimal(binary) == decimal
