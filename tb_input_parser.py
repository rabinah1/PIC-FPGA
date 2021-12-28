import argparse
import os

opcode_dict = {
    "ADDWF":        "000111",
    "ANDWF":        "000101",
    "CLR":          "000001",
    "COMF":         "001001",
    "DECF":         "000011",
    "DECFSZ":       "001011",
    "INCF":         "001010",
    "INCFSZ":       "001111",
    "IORWF":        "000100",
    "MOVF":         "001000",
    "RLF":          "001101",
    "RRF":          "001100",
    "SUBWF":        "000010",
    "SWAPF":        "001110",
    "XORWF":        "000110",
    "ADDLW":        "111110",
    "ANDLW":        "111001",
    "IORLW":        "111000",
    "MOVLW":        "110000",
    "SUBLW":        "111101",
    "XORLW":        "111010",
    "BCF":          "0100",
    "BSF":          "0101",
    "READ_WREG":    "110001",
    "READ_ADDRESS": "110011",
    "READ_STATUS":  "110010"
}

def parse_args():
    descr = """
This script converts the human-readable VHDL tb stimulus file into a format
where each command is a binary string. That format is easier to handle in
the VHDL tb.
"""
    parser = argparse.ArgumentParser(description=descr,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)

    parser.add_argument("input_dir",
                        type=str,
                        help="Path to the directory where the stimulus file is located.")

    args = parser.parse_args()

    return args

def decimal_to_binary(decimal_in, num_bits):
    binary_out = ["0" for i in range(num_bits)]
    idx = num_bits - 1
    while idx >= 0:
        if pow(2, idx) > decimal_in:
            binary_out[num_bits - 1 - idx] = "0"
        else:
            binary_out[num_bits - 1 - idx] = "1"
            decimal_in -= pow(2, idx)
            if decimal_in == 0:
                decimal_in -= 1
        idx -= 1
    return "".join(binary_out)

def parse(line_parts):
    if line_parts[0].strip() == "RESET" or line_parts[0].strip().startswith("#"):
        return line_parts[0].strip()
    opcode_binary = opcode_dict[line_parts[0].strip()]
    binary_input = opcode_binary
    if line_parts[0].strip() in ["ADDLW", "SUBLW", "MOVLW", "ANDLW", "IORLW", "SUBLW", "XORLW"]:
        binary_input += decimal_to_binary(int(line_parts[1].strip()), 8)
    elif line_parts[0].strip() in ["ADDWF", "MOVF", "COMF", "INCF", "ANDWF", "CLR", "DECF", "IORWF",
                                   "RLF", "RRF", "SUBWF", "SWAPF", "XORWF", "DECFSZ", "INCFSZ"]:
        binary_input += line_parts[1].strip()
        binary_input += decimal_to_binary(int(line_parts[2].strip()), 7)
    elif line_parts[0].strip() in ["BCF", "BSF"]:
        binary_input += decimal_to_binary(int(line_parts[1].strip()), 3)
        binary_input += decimal_to_binary(int(line_parts[2].strip()), 7)
    elif line_parts[0].strip() in ["READ_ADDRESS"]:
        binary_input += decimal_to_binary(int(line_parts[1].strip()), 8)
    else:
        binary_input += decimal_to_binary(0, 8)
    return binary_input

def main():
    args = parse_args()
    in_file = open(os.path.join(args.input_dir, "tb_input.txt"), "r")
    out_file = open(os.path.join(args.input_dir, "tb_input_parsed.txt"), "w")

    line = in_file.readline()
    while line:
        if line.startswith("*"):
            line = in_file.readline()
            continue
        line_parts = line.split(",")
        binary_input = parse(line_parts)
        out_file.write(binary_input)
        out_file.write("\n")
        line = in_file.readline()

    in_file.close()
    out_file.close()

main()
