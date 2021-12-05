struct mapping {
    char *command;
    char *binary;
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
