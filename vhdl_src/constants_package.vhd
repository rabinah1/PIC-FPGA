library ieee;
use ieee.std_logic_1164.all;

package constants_package is

    constant ADDWF               : std_logic_vector(5 downto 0) := "000111";
    constant ANDWF               : std_logic_vector(5 downto 0) := "000101";
    constant BCF                 : std_logic_vector(5 downto 0) := "010000";
    constant BSF                 : std_logic_vector(5 downto 0) := "010100";
    constant CLR                 : std_logic_vector(5 downto 0) := "000001";
    constant COMF                : std_logic_vector(5 downto 0) := "001001";
    constant DECF                : std_logic_vector(5 downto 0) := "000011";
    constant DECFSZ              : std_logic_vector(5 downto 0) := "001011";
    constant INCF                : std_logic_vector(5 downto 0) := "001010";
    constant INCFSZ              : std_logic_vector(5 downto 0) := "001111";
    constant IORWF               : std_logic_vector(5 downto 0) := "000100";
    constant MOVF                : std_logic_vector(5 downto 0) := "001000";
    constant RLF                 : std_logic_vector(5 downto 0) := "001101";
    constant RRF                 : std_logic_vector(5 downto 0) := "001100";
    constant SUBWF               : std_logic_vector(5 downto 0) := "000010";
    constant SWAPF               : std_logic_vector(5 downto 0) := "001110";
    constant XORWF               : std_logic_vector(5 downto 0) := "000110";
    constant ADDLW               : std_logic_vector(5 downto 0) := "111110";
    constant ANDLW               : std_logic_vector(5 downto 0) := "111001";
    constant IORLW               : std_logic_vector(5 downto 0) := "111000";
    constant MOVLW               : std_logic_vector(5 downto 0) := "110000";
    constant SUBLW               : std_logic_vector(5 downto 0) := "111101";
    constant XORLW               : std_logic_vector(5 downto 0) := "111010";
    constant READ_WREG           : std_logic_vector(5 downto 0) := "110001";
    constant READ_STATUS         : std_logic_vector(5 downto 0) := "110010";
    constant READ_ADDRESS        : std_logic_vector(5 downto 0) := "110011";
    constant NOP                 : std_logic_vector(5 downto 0) := "000000";
    constant DUMP_MEM            : std_logic_vector(5 downto 0) := "101000";
    constant INDF_ADDRESS        : std_logic_vector(6 downto 0) := "0000000";
    constant TMR0_ADDRESS        : std_logic_vector(6 downto 0) := "0000001";
    constant OPTION_ADDRESS      : std_logic_vector(6 downto 0) := "0000010";
    constant STATUS_ADDRESS      : std_logic_vector(6 downto 0) := "0000011";
    constant FSR_ADDRESS         : std_logic_vector(6 downto 0) := "0000100";
    constant EECON_ADDRESS       : std_logic_vector(6 downto 0) := "0000111";
    constant EEDATA_ADDRESS      : std_logic_vector(6 downto 0) := "0001000";
    constant EEADR_ADDRESS       : std_logic_vector(6 downto 0) := "0001001";
    constant RAM_SIZE            : integer                      := 127;
    constant SLAVE_ADDRESS_READ  : std_logic_vector(7 downto 0) := "10100001";
    constant SLAVE_ADDRESS_WRITE : std_logic_vector(7 downto 0) := "10100000";

end package constants_package;
