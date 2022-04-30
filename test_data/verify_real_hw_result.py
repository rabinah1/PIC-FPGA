import argparse
import os

def parse_args():
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

def main():
    args = parse_args()
    result_file = open(os.path.join(args.input_dir, "real_hw_tb_result.txt"), "r")
    reference_file = open(os.path.join(args.input_dir, "real_hw_tb_reference.txt"), "r")
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
            test_num = result_line.split(" ")[-1].strip()
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
