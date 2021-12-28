struct mapping {
    char *command;
    char *binary;
};

struct num_args_mapping {
    char *command;
    int num_args;
};

struct mapping slave_0_commands[] = {
    "ADDWF",         "000111",
    "ANDWF",         "000101",
    "CLR",           "000001",
    "COMF",          "001001",
    "DECF",          "000011",
    "DECFSZ",        "001011",
    "INCF",          "001010",
    "INCFSZ",        "001111",
    "IORWF",         "000100",
    "MOVF",          "001000",
    "RLF",           "001101",
    "RRF",           "001100",
    "SUBWF",         "000010",
    "SWAPF",         "001110",
    "XORWF",         "000110",
    "ADDLW",         "111110",
    "ANDLW",         "111001",
    "IORLW",         "111000",
    "MOVLW",         "110000",
    "SUBLW",         "111101",
    "XORLW",         "111010",
    "BCF",           "0100",
    "BSF",           "0101",
    "READ_WREG",     "110001",
    "READ_STATUS",   "110010",
    "READ_ADDRESS",  "110011",
    "DUMP_MEM",      "101000",
    "NOP",           "000000",
    "READ_FILE",     "000000",
    "ENABLE_CLOCK",  "000000",
    "DISABLE_CLOCK", "000000",
    "ENABLE_RESET",  "000000",
    "DISABLE_RESET", "000000",
    "EXIT",          "000000",
    "HELP",          "000000",
    "SELECT_SLAVE",  "000000",
    "SHOW_SLAVE",    "000000"
};

struct num_args_mapping slave_0_commands_to_args[] = {
    "ADDWF",         2,
    "ANDWF",         2,
    "CLR",           2,
    "COMF",          2,
    "DECF",          2,
    "DECFSZ",        2,
    "INCF",          2,
    "INCFSZ",        2,
    "IORWF",         2,
    "MOVF",          2,
    "RLF",           2,
    "RRF",           2,
    "SUBWF",         2,
    "SWAPF",         2,
    "XORWF",         2,
    "ADDLW",         1,
    "ANDLW",         1,
    "IORLW",         1,
    "MOVLW",         1,
    "SUBLW",         1,
    "XORLW",         1,
    "BCF",           2,
    "BSF",           2,
    "READ_WREG",     0,
    "READ_STATUS",   0,
    "READ_ADDRESS",  1,
    "DUMP_MEM",      0,
    "NOP",           0,
    "READ_FILE",     1,
    "ENABLE_CLOCK",  0,
    "DISABLE_CLOCK", 0,
    "ENABLE_RESET",  0,
    "DISABLE_RESET", 0,
    "EXIT",          0,
    "HELP",          0,
    "SELECT_SLAVE",  1,
    "SHOW_SLAVE",    0
};
