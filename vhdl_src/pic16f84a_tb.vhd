library ieee;
library std;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;
use std.textio.all;
use ieee.std_logic_textio.all;

entity pic16f84a_tb is
    generic (
        input_file  : string := "";
        output_file : string := ""
    );
end entity pic16f84a_tb;

architecture behavior of pic16f84a_tb is

    signal   serial_in            : std_logic := '0';
    signal   clk                  : std_logic := '0';
    signal   clk_50mhz_in         : std_logic := '0';
    signal   clk_100khz           : std_logic := '0';
    signal   clk_200khz           : std_logic := '0';
    signal   reset                : std_logic := '0';
    signal   miso                 : std_logic := '0';
    signal   mosi                 : std_logic := '0';
    signal   sda                  : std_logic := '0';
    signal   scl                  : std_logic := '0';
    signal   timer_external_input : std_logic := '0';
    signal   alu_output_raspi     : std_logic := '0';
    signal   check                : natural   := 0;
    signal   write_comment        : std_logic := '0';
    signal   result_num           : integer   := 1;
    signal   enable_50mhz         : std_logic := '0';
    signal   enable               : std_logic := '0';
    constant CLK_PERIOD           : time      := 250 us;
    constant EXT_CLK_PERIOD       : time      := 1000 ms;
    constant CLK_50MHZ_PERIOD     : time      := 20 ns;

    component pic16f84a is
        port (
            serial_in            : in    std_logic;
            clk                  : in    std_logic;
            clk_50mhz_in         : in    std_logic;
            reset                : in    std_logic;
            mosi                 : in    std_logic;
            timer_external_input : in    std_logic;
            miso                 : out   std_logic;
            scl                  : out   std_logic;
            alu_output_raspi     : out   std_logic;
            sda                  : inout std_logic;
            memory_mem_a         : out   std_logic_vector(12 downto 0);
            memory_mem_ba        : out   std_logic_vector(2 downto 0);
            memory_mem_ck        : out   std_logic;
            memory_mem_ck_n      : out   std_logic;
            memory_mem_cke       : out   std_logic;
            memory_mem_cs_n      : out   std_logic;
            memory_mem_ras_n     : out   std_logic;
            memory_mem_cas_n     : out   std_logic;
            memory_mem_we_n      : out   std_logic;
            memory_mem_reset_n   : out   std_logic;
            memory_mem_dq        : inout std_logic_vector(7 downto 0);
            memory_mem_dqs       : inout std_logic;
            memory_mem_dqs_n     : inout std_logic;
            memory_mem_odt       : out   std_logic;
            memory_mem_dm        : out   std_logic;
            memory_oct_rzqin     : in    std_logic;
            reset_reset_n        : in    std_logic
        );
    end component;

    component pcf8582_simulator is
        port (
            reset      : in    std_logic;
            clk_100khz : in    std_logic;
            clk_200khz : in    std_logic;
            scl        : in    std_logic;
            sda        : inout std_logic
        );
    end component pcf8582_simulator;

    component clk_div is
        port (
            clk_in     : in    std_logic;
            reset      : in    std_logic;
            clk_100khz : out   std_logic;
            clk_200khz : out   std_logic
        );
    end component clk_div;

begin

    dut_1 : component pic16f84a
        port map (
            serial_in            => serial_in,
            clk                  => clk,
            clk_50mhz_in         => clk_50mhz_in,
            reset                => reset,
            miso                 => miso,
            mosi                 => mosi,
            sda                  => sda,
            scl                  => scl,
            timer_external_input => timer_external_input,
            alu_output_raspi     => alu_output_raspi,
            memory_oct_rzqin     => '0',
            reset_reset_n        => '0'
        );

    dut_2 : component pcf8582_simulator
        port map (
            reset      => reset,
            clk_100khz => clk_100khz,
            clk_200khz => clk_200khz,
            scl        => scl,
            sda        => sda
        );

    dut_3 : component clk_div
        port map (
            clk_in     => clk_50mhz_in,
            reset      => reset,
            clk_100khz => clk_100khz,
            clk_200khz => clk_200khz
        );

    clk_process : process is
    begin

        clk <= '0';
        wait for CLK_PERIOD / 2;
        clk <= '1';
        wait for CLK_PERIOD / 2;

        if (check = 1) then
            wait;
        end if;

    end process clk_process;

    timer_clk_process : process is
    begin

        timer_external_input <= '0';
        wait for EXT_CLK_PERIOD / 2;
        timer_external_input <= '1';
        wait for EXT_CLK_PERIOD / 2;

        if (check = 1) then
            wait;
        end if;

    end process timer_clk_process;

    clk_50mhz_process : process is
    begin

        if (enable = '0') then
            wait until rising_edge(enable_50mhz);
        end if;

        clk_50mhz_in <= '0';
        wait for CLK_50MHZ_PERIOD / 2;
        clk_50mhz_in <= '1';
        wait for CLK_50MHZ_PERIOD / 2;

        if (check = 1) then
            wait;
        end if;

    end process clk_50mhz_process;

    write_to_file : process is

        file     result_file : text open write_mode is input_file;
        variable lineout     : line;
        variable write_flag  : std_logic := '0';
        variable first_flag  : std_logic := '1';

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

        file     stimulus_file  : text open read_mode is output_file;
        variable comma          : character;
        variable linein         : line;
        variable count          : integer;
        variable binary_command : std_logic_vector(13 downto 0);

    begin

        reset <= '1';
        wait for 500 us;
        reset <= '0';

        while (not endfile(stimulus_file)) loop
            readline(stimulus_file, linein);

            if (linein.all = "RESET") then
                result_num <= result_num + 1;
                reset      <= '1';
                wait for 500 us;
                reset      <= '0';
                next;
            elsif (linein.all(1) = '#') then
                write_comment <= '1';
                wait until falling_edge(clk);
                write_comment <= '0';
                next;
            elsif (linein.all = "ENABLE_50MHZ") then
                enable_50mhz <= '1';
                enable       <= '1';
                next;
            elsif (linein.all = "DISABLE_50MHZ") then
                enable_50mhz <= '0';
                enable       <= '0';
                next;
            end if;

            read(linein, binary_command);
            wait until falling_edge(clk);
            mosi  <= '1';
            count := 13;

            while (count >= 0) loop
                serial_in <= binary_command(count);
                count     := count - 1;
                wait until falling_edge(clk);
            end loop;

            mosi <= '0';
            wait for 5 ms;
        end loop;

        wait for 10 ms;
        check <= 1;
        file_close(stimulus_file);
        wait;

    end process stimulus;

end architecture behavior;
