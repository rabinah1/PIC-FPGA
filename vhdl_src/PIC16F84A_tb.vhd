library ieee;
library std;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;
use std.textio.all;
use ieee.std_logic_textio.all;

entity PIC16F84A_tb is
    generic (input_file : string := "";
             output_file : string := "");
end PIC16F84A_tb;

architecture behavior of PIC16F84A_tb is
    signal serial_in : std_logic := '0';
    signal clk : std_logic := '0';
    signal clk_50mhz_in : std_logic := '0';
    signal reset : std_logic := '0';
    signal miso : std_logic := '0';
    signal mosi : std_logic := '0';
    signal sda : std_logic := '0';
    signal scl : std_logic := '0';
    signal timer_external_input : std_logic := '0';
    signal alu_output_raspi : std_logic := '0';
    signal check : natural := 0;
    signal write_comment : std_logic := '0';
    signal result_num : integer := 1;
    constant clk_period : time := 250 us;
    constant ext_clk_period : time := 1000 ms;
    constant clk_50mhz_period : time := 20 ns;

    component PIC16F84A is
        port (serial_in : in std_logic;
              clk : in std_logic;
              clk_50mhz_in : in std_logic;
              reset : in std_logic;
              miso : out std_logic;
              mosi : in std_logic;
              sda : inout std_logic;
              scl : out std_logic;
              timer_external_input : in std_logic;
              alu_output_raspi : out std_logic);
    end component;

    begin
        dut : PIC16F84A
            port map(serial_in => serial_in,
                     clk => clk,
                     clk_50mhz_in => clk_50mhz_in,
                     reset => reset,
                     miso => miso,
                     mosi => mosi,
                     sda => sda,
                     scl => scl,
                     timer_external_input => timer_external_input,
                     alu_output_raspi => alu_output_raspi);

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

        timer_clk_process : process is
        begin

            timer_external_input <= '0';
            wait for ext_clk_period/2;
            timer_external_input <= '1';
            wait for ext_clk_period/2;
            if (check = 1) then
                wait;
            end if;

        end process timer_clk_process;

        clk_50mhz_process : process is
        begin

            clk_50mhz_in <= '0';
            wait for clk_50mhz_period/2;
            clk_50mhz_in <= '1';
            wait for clk_50mhz_period/2;
            if (check = 1) then
                wait;
            end if;

        end process clk_50mhz_process;

        write_to_file : process is
            file result_file : text open write_mode is input_file;
            variable lineout : line;
            variable write_flag : std_logic := '0';
            variable first_flag : std_logic := '1';
        begin

            wait until falling_edge(clk);
            if (write_comment = '1') then
                write(lineout, string'("# result_") & integer'image(result_num));
                writeline(result_file, lineout);
            end if;
            if (miso = '1' and first_flag = '1') then
                first_flag := '0';
            elsif (miso = '1' and first_flag = '0') then
                write(lineout, alu_output_raspi);
                write_flag := '1';
            elsif (write_flag = '1') then
                writeline(result_file, lineout);
                write_flag := '0';
                first_flag := '1';
            elsif (check = 1) then
                file_close(result_file);
                wait;
            end if;

        end process write_to_file;

        stimulus : process is
            file stimulus_file : text open read_mode is output_file;
            variable comma : character;
            variable linein : line;
            variable count : integer := 0;
            variable binary_command : std_logic_vector(13 downto 0) := (others => '0');
        begin

            reset <= '1';
            wait for 1 ms;
            reset <= '0';
            while (not endfile(stimulus_file)) loop
                readline(stimulus_file, linein);
                if (linein.all = "RESET") then
                    result_num <= result_num + 1;
                    reset <= '1';
                    wait for 1 ms;
                    reset <= '0';
                    next;
                elsif (linein.all(1) = '#') then
                    write_comment <= '1';
                    next;
                end if;
                read(linein, binary_command);
                wait until falling_edge(clk);
                write_comment <= '0';
                mosi <= '1';
                count := 13;
                while (count >= 0) loop
                    serial_in <= binary_command(count);
                    count := count - 1;
                    wait until falling_edge(clk);
                end loop;
                mosi <= '0';
                wait for 5 ms;
            end loop;
            wait for 100 ms;
            check <= 1;
            file_close(stimulus_file);
            wait;

        end process stimulus;

end architecture behavior;
