# pylint: disable=missing-docstring
# pylint: disable=consider-using-with
import argparse
import os
import subprocess


def _parse_args():
    descr = """
This script compares the result file created by real HW testrun
with the reference file created by the user.
"""
    parser = argparse.ArgumentParser(description=descr,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)

    parser.add_argument("input_dir",
                        type=str,
                        help="Path to the directory where the reference file is located")

    args = parser.parse_args()

    return args


def _check_diff(args):
    result_file = os.path.join(os.getcwd(), "test_data/real_hw_tb_result.txt")
    reference_file = os.path.join(os.getcwd(), "test_data/real_hw_tb_reference.txt")
    ret = subprocess.run(["diff", result_file, reference_file, "-C", "10", "-w"], check=False,
                         capture_output=True)
    if ret.stdout:
        with open(os.path.join(args.input_dir, "real_hw_result_diff.txt"), "w",
                  encoding="utf-8") as diff_file:
            diff_file.write(ret.stdout.decode('ascii'))
        print("Test suite failed, please check real_hw_result_diff.txt")
        return
    print("Test suite passed")


def main():
    args = _parse_args()
    _check_diff(args)


if __name__ == "__main__":
    main()
