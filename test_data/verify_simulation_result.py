import argparse
import os

def parse_args():
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

def binary_to_decimal(binary):
    idx = 0
    decimal = 0
    while idx < 8:
        decimal += int(binary[idx]) * pow(2, 7 - idx)
        idx += 1

    return decimal

def main():
    args = parse_args()
    in_file = open(os.path.join(args.input_dir, "tb_result.txt"), "r")
    out_file = open(os.path.join(args.input_dir, "tb_result_formatted.txt"), "w")

    line = in_file.readline()
    while line:
        if line.startswith("#"):
            out_file.write(line)
            line = in_file.readline()
            continue
        decimal_value = binary_to_decimal(line.strip())
        out_file.write(str(decimal_value))
        out_file.write("\n")
        line = in_file.readline()

    in_file.close()
    out_file.close()

    result_file = open(os.path.join(args.input_dir, "tb_result_formatted.txt"), "r")
    reference_file = open(os.path.join(args.input_dir, "tb_reference.txt"), "r")
    result_line = result_file.readline()
    reference_line = reference_file.readline()
    case_passed = True
    suite_passed = True
    test_num = None

    while result_line:
        if result_line.startswith("#"):
            if test_num is not None:
                if case_passed:
                    print(f"Test {test_num} passed")
                else:
                    print(f"Test {test_num} failed")
            test_num = result_line.split("_")[-1].strip()
            case_passed = True
        if not reference_line:
            print("Test suite failed: result file contains more lines than reference file")
            return
        if result_line != reference_line:
            suite_passed = False
            case_passed = False
        result_line = result_file.readline()
        reference_line = reference_file.readline()
    if suite_passed:
        if reference_line:
            print("Test suite failed: reference file contains more lines than result file")
            return
        else:
            print(f"Test {test_num} passed")
            print("Test suite passed")
    else:
        print(f"Test {test_num} failed")
        print("Test suite failed")

    reference_file.close()
    result_file.close()

if __name__ == "__main__":
    main()
