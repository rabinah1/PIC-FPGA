library ieee;
library vunit_lib;
    context vunit_lib.vunit_context;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity tb_alu_input_mux is
    generic (
        runner_cfg : string := runner_cfg_default
    );
end entity tb_alu_input_mux;

architecture tb of tb_alu_input_mux is

    signal   clk           : std_logic := '0';
    signal   reset         : std_logic := '0';
    signal   enable        : std_logic := '0';
    signal   sel           : std_logic := '0';
    signal   input_ram     : std_logic_vector(7 downto 0) := (others => '0');
    signal   input_literal : std_logic_vector(7 downto 0) := (others => '0');
    signal   data_out      : std_logic_vector(7 downto 0) := (others => '0');
    signal   check_sig     : natural := 0;
    constant CLK_PERIOD    : time := 250 us;

    component alu_input_mux is
        port (
            clk           : in    std_logic;
            reset         : in    std_logic;
            enable        : in    std_logic;
            sel           : in    std_logic;
            input_ram     : in    std_logic_vector(7 downto 0);
            input_literal : in    std_logic_vector(7 downto 0);
            data_out      : out   std_logic_vector(7 downto 0)
        );
    end component;

begin

    alu_input_mux_instance : component alu_input_mux
        port map (
            clk           => clk,
            reset         => reset,
            enable        => enable,
            sel           => sel,
            input_ram     => input_ram,
            input_literal => input_literal,
            data_out      => data_out
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

            if run("test_output_is_zero_if_reset_is_enabled") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_output_is_zero_if_reset_is_enabled");
                info("--------------------------------------------------------------------------------");
                reset         <= '1';
                enable        <= '0';
                sel           <= '0';
                input_literal <= std_logic_vector(to_unsigned(5, 8));
                input_ram     <= std_logic_vector(to_unsigned(10, 8));
                wait for CLK_PERIOD * 2;
                check_equal(data_out, std_logic_vector(to_unsigned(0, 8)),
                            "Comparing data_out against reference failed.");
                check_sig     <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_output_is_zero_if_enable_is_zero") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_output_is_zero_if_enable_is_zero");
                info("--------------------------------------------------------------------------------");
                reset         <= '1';
                wait until rising_edge(clk);
                reset         <= '0';
                enable        <= '0';
                sel           <= '1';
                input_literal <= std_logic_vector(to_unsigned(5, 8));
                input_ram     <= std_logic_vector(to_unsigned(20, 8));
                wait for CLK_PERIOD * 2;
                check_equal(data_out, std_logic_vector(to_unsigned(0, 8)),
                            "Comparing data_out against reference failed.");
                check_sig     <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_literal_is_passed_to_output") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_literal_is_passed_to_output");
                info("--------------------------------------------------------------------------------");
                reset         <= '1';
                wait until rising_edge(clk);
                reset         <= '0';
                enable        <= '1';
                sel           <= '0';
                input_literal <= std_logic_vector(to_unsigned(5, 8));
                input_ram     <= std_logic_vector(to_unsigned(20, 8));
                wait for CLK_PERIOD * 2;
                check_equal(data_out, std_logic_vector(to_unsigned(5, 8)),
                            "Comparing data_out against reference failed.");
                check_sig     <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_ram_is_passed_to_output") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_ram_is_passed_to_output");
                info("--------------------------------------------------------------------------------");
                reset         <= '1';
                wait until rising_edge(clk);
                reset         <= '0';
                enable        <= '1';
                sel           <= '1';
                input_literal <= std_logic_vector(to_unsigned(5, 8));
                input_ram     <= std_logic_vector(to_unsigned(20, 8));
                wait for CLK_PERIOD * 2;
                check_equal(data_out, std_logic_vector(to_unsigned(20, 8)),
                            "Comparing data_out against reference failed.");
                check_sig     <= 1;
                info("===== TEST CASE FINISHED =====");
            end if;

        end loop;

        test_runner_cleanup(runner);

    end process test_runner;

end architecture tb;
