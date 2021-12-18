def binary_to_decimal(binary):
    idx = 0
    decimal = 0
    while idx < 8:
        decimal += int(binary[idx]) * pow(2, 7 - idx)
        idx += 1

    return decimal

def main():
    in_file = open("tb_result.txt", "r")
    out_file = open("tb_result_formatted.txt", "w")

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

    reference_file = open("tb_reference.txt", "r")
    result_file = open("tb_result_formatted.txt", "r")

    result_line = result_file.readline()
    reference_line = reference_file.readline()
    test_num = 0
    ok_flag = True
    while result_line:
        if not reference_line:
            print("Verification failed: result file contains more lines than reference file.")
            ok_flag = False
            break
        if result_line.startswith("#"):
            test_num = result_line.split("_")[1]
        if result_line != reference_line:
            print("Verification failed: test {} failed, result_line \"{}\" does not match with " \
                  "reference_line \"{}\".".format(test_num.strip(), result_line.strip(),
                                                  reference_line.strip()))
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
