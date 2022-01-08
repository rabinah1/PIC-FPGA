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
                        help="Path to the directory where the reference file is located.")

    args = parser.parse_args()

    return args

def main():
    args = parse_args()
    result_file = open(os.path.join(args.input_dir, "real_hw_tb_result.txt"), "r")
    reference_file = open(os.path.join(args.input_dir, "real_hw_tb_reference.txt"), "r")

    result_line = result_file.readline()
    reference_line = reference_file.readline()
    ok_flag = True
    while result_line:
        if not reference_line:
            print("Verification failed: result file contains more lines than reference file.")
            ok_flag = False
            break
        if result_line != reference_line:
            print("Verification failed")
            ok_flag = False
            break
        result_line = result_file.readline()
        reference_line = reference_file.readline()
    if ok_flag:
        if reference_line:
            print("Verification failed: reference file contains more lines than result file.")
        else:
            print("Verification passed.")

    reference_file.close()
    result_file.close()

main()
