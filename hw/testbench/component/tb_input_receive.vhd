library ieee;
library vunit_lib;
    context vunit_lib.vunit_context;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity tb_input_receive is
    generic (
        runner_cfg : string := runner_cfg_default
    );
end entity tb_input_receive;

architecture tb of tb_input_receive is

    signal   clk                      : std_logic := '0';
    signal   reset                    : std_logic := '0';
    signal   serial_in                : std_logic := '0';
    signal   mosi                     : std_logic := '0';
    signal   trig_instruction_process : std_logic := '0';
    signal   binary_string            : std_logic_vector(13 downto 0) := (others => '0');
    signal   check_sig                : natural := 0;
    constant CLK_PERIOD               : time := 250 us;

    component input_receive is
        port (
            clk                      : in    std_logic;
            reset                    : in    std_logic;
            serial_in                : in    std_logic;
            mosi                     : in    std_logic;
            trig_instruction_process : out   std_logic;
            binary_string            : out   std_logic_vector(13 downto 0)
        );
    end component;

begin

    input_receive_instance : component input_receive
        port map (
            clk                      => clk,
            reset                    => reset,
            serial_in                => serial_in,
            mosi                     => mosi,
            trig_instruction_process => trig_instruction_process,
            binary_string            => binary_string
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
                check_equal(binary_string, std_logic_vector(unsigned'("00000000000000")),
                            "Comparing binary_string against reference failed.");
                check_equal(trig_instruction_process, '0',
                            "Comparing trig_instruction_process reference failed.");
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_mosi_detected") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_mosi_detected");
                info("--------------------------------------------------------------------------------");
                reset     <= '1';
                wait for CLK_PERIOD * 2;
                reset     <= '0';
                serial_in <= '1';
                mosi      <= '1';
                wait for CLK_PERIOD;
                check_equal(binary_string, std_logic_vector(unsigned'("00000000000001")),
                            "Comparing binary_string against reference failed.");
                check_equal(trig_instruction_process, '0',
                            "Comparing trig_instruction_process reference failed.");
                mosi      <= '0';
                wait for CLK_PERIOD * 4;
                check_equal(trig_instruction_process, '1',
                            "Comparing trig_instruction_process reference failed.");
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_mosi_not_detected") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_mosi_not_detected");
                info("--------------------------------------------------------------------------------");
                reset     <= '1';
                wait for CLK_PERIOD * 2;
                reset     <= '0';
                serial_in <= '1';
                mosi      <= '0';
                check_equal(binary_string, std_logic_vector(unsigned'("00000000000000")),
                            "Comparing binary_string against reference failed.");
                check_equal(trig_instruction_process, '0',
                            "Comparing trig_instruction_process reference failed.");
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            end if;

        end loop;

        test_runner_cleanup(runner);

    end process test_runner;

end architecture tb;
