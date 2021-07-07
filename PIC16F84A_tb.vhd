library ieee;
library std;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;
use std.textio.all;
use ieee.std_logic_textio.all;

entity PIC16F84A_tb is
end PIC16F84A_tb;

architecture behavior of PIC16F84A_tb is
    signal serial_in_literal : std_logic := '0';
    signal serial_in_opcode : std_logic := '0';
    signal clk : std_logic := '0';
    signal reset : std_logic := '0';
    signal enable_opcode : std_logic := '0';
    signal enable_literal : std_logic := '0';
    signal enable_ALU : std_logic := '0';
    signal enable_w_register : std_logic := '0';
    signal enable_wreg_out : std_logic := '0';
    signal ALU_output_raspi : std_logic := '0';
    constant clk_period : time := 1 us;
    signal check : natural := 0;

    component PIC16F84A is
        generic (N : natural := 8);
        port (serial_in_literal : in std_logic;
              serial_in_opcode : in std_logic;
              clk : in std_logic;
              reset : in std_logic;
              enable_opcode : in std_logic;
              enable_literal : in std_logic;
              enable_ALU : in std_logic;
              enable_w_register : in std_logic;
              enable_wreg_out : in std_logic;
              ALU_output_raspi : out std_logic);
    end component;

    begin
        dut : PIC16F84A
            port map(serial_in_literal => serial_in_literal, serial_in_opcode => serial_in_opcode,
                     clk => clk, reset => reset, enable_opcode => enable_opcode,
                     enable_literal => enable_literal, enable_ALU => enable_ALU,
                     enable_w_register => enable_w_register, enable_wreg_out => enable_wreg_out,
                     ALU_output_raspi => ALU_output_raspi);

        clk_process : process is
            begin
                clk <= '0';
                wait for clk_period/2;
                clk <= '1';
                wait for clk_period/2;
                if (check = 1) then
                    wait;
                end if;
        end process clk_process;

        stimulus : process is
            file stimulus_file : text open read_mode is "C:\Users\henry\Documents\PIC-FPGA\Input.txt";
            variable data_in_literal : std_logic_vector(21 downto 0);
            variable data_in_opcode : std_logic_vector(21 downto 0);
            variable comma : character;
            variable linein : line;
            variable count : integer := 0;

            begin
                reset <= '0';
                wait for 2 us;
                reset <= '1';
                wait for 10.5 us;
                reset <= '0';
                while (not endfile(stimulus_file)) loop
                    readline(stimulus_file, linein);
                    read(linein, data_in_literal);
                    read(linein, comma);
                    read(linein, data_in_opcode);
                    wait until rising_edge(clk);
                    enable_literal <= '1';
                    count := 21;
                    while (count >= 0) loop
                        serial_in_literal <= data_in_literal(count);
                        count := count - 1;
                        wait until rising_edge(clk);
                    end loop;
                    enable_literal <= '0';
                    wait for 10 us;
                    wait until rising_edge(clk);
                    enable_opcode <= '1';
                    count := 21;
                    while (count >= 0) loop
                        serial_in_opcode <= data_in_opcode(count);
                        count := count - 1;
                        wait until rising_edge(clk);
                    end loop;
                    enable_opcode <= '0';
                    wait for 10 us;
                    enable_ALU <= '1';
                    wait until rising_edge(clk);
                    enable_ALU <= '0';
                    wait for 10 us;
                    enable_w_register <= '1';
                    wait for 10 us;
                    enable_w_register <= '0';
                    wait for 10 us;
                    enable_wreg_out <= '1';
                    wait for 20 us;
                    enable_wreg_out <= '0';
                end loop;
                file_close(stimulus_file);
                check <= 1;
                wait;
        end process stimulus;
    end architecture behavior;