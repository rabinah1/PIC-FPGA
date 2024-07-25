library ieee;
library vunit_lib;
    context vunit_lib.vunit_context;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity tb_w_register is
    generic (
        runner_cfg : string := runner_cfg_default
    );
end entity tb_w_register;

architecture tb of tb_w_register is

    signal   clk        : std_logic := '0';
    signal   enable     : std_logic := '0';
    signal   reset      : std_logic := '0';
    signal   data_in    : std_logic_vector(7 downto 0) := (others => '0');
    signal   data_out   : std_logic_vector(7 downto 0) := (others => '0');
    signal   check_sig  : natural := 0;
    constant CLK_PERIOD : time := 250 us;

    component w_register is
        port (
            clk      : in    std_logic;
            enable   : in    std_logic;
            reset    : in    std_logic;
            data_in  : in    std_logic_vector(7 downto 0);
            data_out : out   std_logic_vector(7 downto 0)
        );
    end component;

begin

    w_register_instance : component w_register
        port map (
            clk      => clk,
            enable   => enable,
            reset    => reset,
            data_in  => data_in,
            data_out => data_out
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

            if run("test_output_is_zero_if_reset_is_enabled_without_enable") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_output_is_zero_if_reset_is_enabled_without_enable");
                info("--------------------------------------------------------------------------------");
                reset     <= '1';
                enable    <= '0';
                data_in   <= std_logic_vector(to_unsigned(5, 8));
                wait for 1 ms;
                check(data_out = std_logic_vector(to_unsigned(0, 8)), "Expect data_out to be 0.");
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_output_is_zero_if_reset_is_enabled_with_enable") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_output_is_zero_if_reset_is_enabled_with_enable");
                info("--------------------------------------------------------------------------------");
                reset     <= '1';
                enable    <= '1';
                data_in   <= std_logic_vector(to_unsigned(5, 8));
                wait for 1 ms;
                check(data_out = std_logic_vector(to_unsigned(0, 8)), "Expect data_out to be 0.");
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_output_is_zero_when_enable_is_zero") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_output_is_zero_when_enable_is_zero");
                info("--------------------------------------------------------------------------------");
                reset     <= '1';
                wait for 1 us;
                reset     <= '0';
                enable    <= '0';
                data_in   <= std_logic_vector(to_unsigned(56, 8));
                wait for 1 ms;
                check(data_out = std_logic_vector(to_unsigned(0, 8)), "Expect data_out to be 0.");
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_input_is_passed_to_output_when_enable_is_one") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_input_is_passed_to_output_when_enable_is_one");
                info("--------------------------------------------------------------------------------");
                reset     <= '1';
                wait for 1 us;
                reset     <= '0';
                enable    <= '1';
                data_in   <= std_logic_vector(to_unsigned(56, 8));
                wait for 1 ms;
                check(data_out = std_logic_vector(to_unsigned(56, 8)), "Expect data_out to be 56.");
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            end if;

        end loop;

        test_runner_cleanup(runner);

    end process test_runner;

end architecture tb;
