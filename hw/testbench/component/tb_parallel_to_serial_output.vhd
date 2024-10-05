library ieee;
library vunit_lib;
    context vunit_lib.vunit_context;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity tb_parallel_to_serial_output is
    generic (
        runner_cfg : string := runner_cfg_default
    );
end entity tb_parallel_to_serial_output;

architecture tb of tb_parallel_to_serial_output is

    signal   clk                       : std_logic := '0';
    signal   reset                     : std_logic := '0';
    signal   enable                    : std_logic := '0';
    signal   result_enable_ram_dump    : std_logic := '0';
    signal   result_enable_eeprom_dump : std_logic := '0';
    signal   data_to_sw                : std_logic_vector(7 downto 0) := (others => '0');
    signal   ram_dump_to_sw            : std_logic_vector(1015 downto 0) := (others => '0');
    signal   eeprom_dump_to_sw         : std_logic_vector(2047 downto 0) := (others => '0');
    signal   miso                      : std_logic := '0';
    signal   serial_out                : std_logic := '0';
    signal   check_sig                 : natural := 0;
    constant CLK_PERIOD                : time := 250 us;

    component parallel_to_serial_output is
        port (
            clk                       : in    std_logic;
            reset                     : in    std_logic;
            enable                    : in    std_logic;
            result_enable_ram_dump    : in    std_logic;
            result_enable_eeprom_dump : in    std_logic;
            data_to_sw                : in    std_logic_vector(7 downto 0);
            ram_dump_to_sw            : in    std_logic_vector(1015 downto 0);
            eeprom_dump_to_sw         : in    std_logic_vector(2047 downto 0);
            miso                      : out   std_logic;
            serial_out                : out   std_logic
        );
    end component;

begin

    parallel_to_serial_output_instance : component parallel_to_serial_output
        port map (
            clk                       => clk,
            reset                     => reset,
            enable                    => enable,
            result_enable_ram_dump    => result_enable_ram_dump,
            result_enable_eeprom_dump => result_enable_eeprom_dump,
            data_to_sw                => data_to_sw,
            ram_dump_to_sw            => ram_dump_to_sw,
            eeprom_dump_to_sw         => eeprom_dump_to_sw,
            miso                      => miso,
            serial_out                => serial_out
        );

    clk_process : process is
    begin

        clk <= '0';
        wait for CLK_PERIOD / 2;
        clk <= '1';
        wait for CLK_PERIOD / 2;

        if (check_sig = 1) then
            wait;
        end if;

    end process clk_process;

    test_runner : process is
    begin

        test_runner_setup(runner, runner_cfg);
        show(get_logger(default_checker), display_handler, pass);

        test_cases_loop : while test_suite loop

            if run("test_reset_is_enabled") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_reset_is_enabled");
                info("--------------------------------------------------------------------------------");
                reset     <= '1';
                wait for CLK_PERIOD * 2;
                check_equal(serial_out, '0',
                            "Comparing serial_out against reference failed.");
                check_equal(miso, '0',
                            "Comparing miso against reference failed.");
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_eeprom_dump") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_eeprom_dump");
                info("--------------------------------------------------------------------------------");
                reset                     <= '1';
                wait for CLK_PERIOD * 2;
                reset                     <= '0';
                result_enable_eeprom_dump <= '1';
                eeprom_dump_to_sw         <= (2047 => '1', 2046 => '0', 2045 => '1', others => '0');
                wait for CLK_PERIOD;
                check_equal(serial_out, '0',
                            "Comparing serial_out against reference failed.");
                check_equal(miso, '1',
                            "Comparing miso against reference failed.");
                wait for CLK_PERIOD;
                check_equal(serial_out, '1',
                            "Comparing serial_out against reference failed.");
                wait for CLK_PERIOD;
                check_equal(serial_out, '0',
                            "Comparing serial_out against reference failed.");
                wait for CLK_PERIOD;
                check_equal(serial_out, '1',
                            "Comparing serial_out against reference failed.");
                check_sig                 <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_ram_dump") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_ram_dump");
                info("--------------------------------------------------------------------------------");
                reset                  <= '1';
                wait for CLK_PERIOD * 2;
                reset                  <= '0';
                result_enable_ram_dump <= '1';
                ram_dump_to_sw         <= (1015 => '1', 1014 => '1', 1013 => '0', others => '0');
                wait for CLK_PERIOD;
                check_equal(serial_out, '0',
                            "Comparing serial_out against reference failed.");
                check_equal(miso, '1',
                            "Comparing miso against reference failed.");
                wait for CLK_PERIOD;
                check_equal(serial_out, '1',
                            "Comparing serial_out against reference failed.");
                wait for CLK_PERIOD;
                check_equal(serial_out, '1',
                            "Comparing serial_out against reference failed.");
                wait for CLK_PERIOD;
                check_equal(serial_out, '0',
                            "Comparing serial_out against reference failed.");
                check_sig              <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_normal_data") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_normal_data");
                info("--------------------------------------------------------------------------------");
                reset      <= '1';
                wait for CLK_PERIOD * 2;
                reset      <= '0';
                enable     <= '1';
                data_to_sw <= "11001100";
                wait for CLK_PERIOD;
                check_equal(serial_out, '0',
                            "Comparing serial_out against reference failed.");
                check_equal(miso, '1',
                            "Comparing miso against reference failed.");
                wait for CLK_PERIOD;
                check_equal(serial_out, '1',
                            "Comparing serial_out against reference failed.");
                wait for CLK_PERIOD;
                check_equal(serial_out, '1',
                            "Comparing serial_out against reference failed.");
                wait for CLK_PERIOD;
                check_equal(serial_out, '0',
                            "Comparing serial_out against reference failed.");
                wait for CLK_PERIOD;
                check_equal(serial_out, '0',
                            "Comparing serial_out against reference failed.");
                wait for CLK_PERIOD;
                check_equal(serial_out, '1',
                            "Comparing serial_out against reference failed.");
                wait for CLK_PERIOD;
                check_equal(serial_out, '1',
                            "Comparing serial_out against reference failed.");
                wait for CLK_PERIOD;
                check_equal(serial_out, '0',
                            "Comparing serial_out against reference failed.");
                wait for CLK_PERIOD;
                check_equal(serial_out, '0',
                            "Comparing serial_out against reference failed.");
                check_sig  <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_exception") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_exception");
                info("--------------------------------------------------------------------------------");
                reset     <= '1';
                wait for CLK_PERIOD * 2;
                reset     <= '0';
                wait for CLK_PERIOD * 2;
                check_equal(serial_out, '0',
                            "Comparing serial_out against reference failed.");
                check_equal(miso, '0',
                            "Comparing miso against reference failed.");
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            end if;

        end loop;

        test_runner_cleanup(runner);

    end process test_runner;

end architecture tb;
