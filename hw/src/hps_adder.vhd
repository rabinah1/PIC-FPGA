library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity hps_adder is
    port (
        clk       : in    std_logic;
        reset     : in    std_logic;
        read      : in    std_logic;
        write     : in    std_logic;
        writedata : in    std_logic_vector(15 downto 0);
        address   : in    std_logic_vector(1 downto 0);
        readdata  : out   std_logic_vector(15 downto 0)
    );
end entity hps_adder;

architecture rtl of hps_adder is

    signal operand_1 : std_logic_vector(15 downto 0);
    signal operand_2 : std_logic_vector(15 downto 0);

begin

    hps_adder : process (clk, reset, read, write, writedata, address) is
    begin

        if (reset = '1') then
            readdata  <= (others => '0');
            operand_1 <= (others => '0');
            operand_2 <= (others => '0');
        elsif (rising_edge(clk)) then
            if (read = '1') then
                if (address = "00") then
                    readdata <= '1' & operand_1(14 downto 0);
                elsif (address = "01") then
                    readdata <= '1' & operand_2(14 downto 0);
                elsif (address = "10") then
                    readdata <= '1' & (operand_1(14 downto 0) + operand_2(14 downto 0));
                else
                    readdata <= (others => '0');
                end if;
            elsif (write = '1') then
                if (address = "00") then
                    operand_1 <= writedata;
                elsif (address = "01") then
                    operand_2 <= writedata;
                else
                    operand_1 <= (others => '0');
                    operand_2 <= (others => '0');
                end if;
            end if;
        end if;

    end process hps_adder;

end architecture rtl;
