library ieee;
library vunit_lib;
    context vunit_lib.vunit_context;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity tb_alu_output_demux is
    generic (
        runner_cfg : string := runner_cfg_default
    );
end entity tb_alu_output_demux;

architecture tb of tb_alu_output_demux is

    signal   clk            : std_logic := '0';
    signal   reset          : std_logic := '0';
    signal   sel            : std_logic := '0';
    signal   transfer_to_sw : std_logic := '0';
    signal   input_data     : std_logic_vector(7 downto 0) := (others => '0');
    signal   data_to_ram    : std_logic_vector(7 downto 0) := (others => '0');
    signal   data_to_sw     : std_logic_vector(7 downto 0) := (others => '0');
    signal   data_to_wreg   : std_logic_vector(7 downto 0) := (others => '0');
    signal   check_sig      : natural := 0;
    constant CLK_PERIOD     : time := 250 us;

    component alu_output_demux is
        port (
            clk            : in    std_logic;
            reset          : in    std_logic;
            sel            : in    std_logic;
            transfer_to_sw : in    std_logic;
            input_data     : in    std_logic_vector(7 downto 0);
            data_to_ram    : out   std_logic_vector(7 downto 0);
            data_to_sw     : out   std_logic_vector(7 downto 0);
            data_to_wreg   : out   std_logic_vector(7 downto 0)
        );
    end component;

begin

    alu_output_demux_instance : component alu_output_demux
        port map (
            clk            => clk,
            reset          => reset,
            sel            => sel,
            transfer_to_sw => transfer_to_sw,
            input_data     => input_data,
            data_to_ram    => data_to_ram,
            data_to_sw     => data_to_sw,
            data_to_wreg   => data_to_wreg
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

            if run("test_all_outputs_are_zero_when_reset_is_enabled") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_all_outputs_are_zero_when_reset_is_enabled");
                info("--------------------------------------------------------------------------------");
                reset      <= '1';
                input_data <= std_logic_vector(to_unsigned(1, 8));
                wait for 1 ms;
                check(data_to_wreg = std_logic_vector(to_unsigned(0, 8)), "Expect data_to_wreg to be 0.");
                check(data_to_ram = std_logic_vector(to_unsigned(0, 8)), "Expect data_to_ram to be 0.");
                check(data_to_sw = std_logic_vector(to_unsigned(0, 8)), "Expect data_to_sw to be 0.");
                check_sig  <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_input_is_sent_to_sw") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_input_is_sent_to_sw");
                info("--------------------------------------------------------------------------------");
                reset          <= '1';
                wait for 500 us;
                reset          <= '0';
                transfer_to_sw <= '1';
                sel            <= '1';
                input_data     <= std_logic_vector(to_unsigned(2, 8));
                wait for 1 ms;
                check(data_to_wreg = std_logic_vector(to_unsigned(0, 8)), "Expect data_to_wreg to be 0.");
                check(data_to_ram = std_logic_vector(to_unsigned(0, 8)), "Expect data_to_ram to be 0.");
                check(data_to_sw = std_logic_vector(to_unsigned(2, 8)), "Expect data_to_sw to be 2.");
                check_sig      <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_input_is_sent_to_w_register") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_input_is_sent_to_w_register");
                info("--------------------------------------------------------------------------------");
                reset          <= '1';
                wait for 500 us;
                reset          <= '0';
                transfer_to_sw <= '0';
                sel            <= '0';
                input_data     <= std_logic_vector(to_unsigned(3, 8));
                wait for 1 ms;
                check(data_to_wreg = std_logic_vector(to_unsigned(3, 8)), "Expect data_to_wreg to be 3.");
                check(data_to_ram = std_logic_vector(to_unsigned(0, 8)), "Expect data_to_ram to be 0.");
                check(data_to_sw = std_logic_vector(to_unsigned(0, 8)), "Expect data_to_sw to be 0.");
                check_sig      <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_input_is_sent_to_ram") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_input_is_sent_to_ram");
                info("--------------------------------------------------------------------------------");
                reset          <= '1';
                wait for 500 us;
                reset          <= '0';
                transfer_to_sw <= '0';
                sel            <= '1';
                input_data     <= std_logic_vector(to_unsigned(4, 8));
                wait for 1 ms;
                check(data_to_wreg = std_logic_vector(to_unsigned(0, 8)), "Expect data_to_wreg to be 0.");
                check(data_to_ram = std_logic_vector(to_unsigned(4, 8)), "Expect data_to_ram to be 4.");
                check(data_to_sw = std_logic_vector(to_unsigned(0, 8)), "Expect data_to_sw to be 0.");
                check_sig      <= 1;
                info("===== TEST CASE FINISHED =====");
            end if;

        end loop;

        test_runner_cleanup(runner);

    end process test_runner;

end architecture tb;
