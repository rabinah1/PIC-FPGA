# pylint: disable=missing-docstring
# pylint: disable=consider-using-with
import argparse
import os
import subprocess


def _parse_args():
    descr = """
This script compares the result file created by VHDL testbench
with the reference file created by the user.
"""
    parser = argparse.ArgumentParser(description=descr,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)

    parser.add_argument("input_dir",
                        type=str,
                        help="Path to the directory where the reference file is located")

    args = parser.parse_args()

    return args


def _format_results(args):
    in_file = open(os.path.join(args.input_dir, "tb_result.txt"), "r", encoding="utf-8")
    out_file = open(os.path.join(args.input_dir, "tb_result_formatted.txt"), "w", encoding="utf-8")

    line = in_file.readline()
    while line:
        if line.startswith("#"):
            out_file.write(line)
            line = in_file.readline()
            continue
        decimal_value = _binary_to_decimal(line.strip())
        out_file.write(str(decimal_value))
        out_file.write("\n")
        line = in_file.readline()

    in_file.close()
    out_file.close()


def _binary_to_decimal(binary):
    idx = 0
    decimal = 0
    while idx < 8:
        decimal += int(binary[idx]) * pow(2, 7 - idx)
        idx += 1

    return decimal


def _check_diff(args):
    result_file = os.path.join(os.getcwd(), "../test_data/tb_result_formatted.txt")
    reference_file = os.path.join(os.getcwd(), "../test_data/tb_reference_test.txt")
    ret = subprocess.run(["diff", result_file, reference_file, "-C", "10", "-w"], check=False,
                         capture_output=True)
    if ret.stdout:
        with open(os.path.join(args.input_dir, "simulation_result_diff.txt"), "w",
                  encoding="utf-8") as diff_file:
            diff_file.write(ret.stdout.decode('ascii'))
        print("Test suite failed, please check simulation_result_diff.txt")
        return
    print("Test suite passed")


def main():
    args = _parse_args()
    _format_results(args)
    _check_diff(args)


if __name__ == "__main__":
    main()
