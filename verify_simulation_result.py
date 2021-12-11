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
        decimal_value = binary_to_decimal(line.strip())
        out_file.write(str(decimal_value))
        out_file.write("\n")
        line = in_file.readline()

    in_file.close()
    out_file.close()

    reference_file = open("tb_reference.txt", "r")
    result_file = open("tb_result_formatted.txt", "r")

    reference = reference_file.readlines()
    result = result_file.readlines()
    if reference == result:
        print("RESULT: verification passed")
    else:
        print("RESULT: verification failed")

    reference_file.close()
    result_file.close()

main()
