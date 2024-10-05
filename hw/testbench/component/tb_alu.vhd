library ieee;
library vunit_lib;
    context vunit_lib.vunit_context;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity tb_alu is
    generic (
        runner_cfg : string := runner_cfg_default
    );
end entity tb_alu;

architecture tb of tb_alu is

    signal   clk         : std_logic := '0';
    signal   reset       : std_logic := '0';
    signal   enable      : std_logic := '0';
    signal   input_w_reg : std_logic_vector(7 downto 0) := (others => '0');
    signal   output_mux  : std_logic_vector(7 downto 0) := (others => '0');
    signal   opcode      : std_logic_vector(5 downto 0) := (others => '0');
    signal   status_in   : std_logic_vector(7 downto 0) := (others => '0');
    signal   bit_idx     : std_logic_vector(2 downto 0) := (others => '0');
    signal   status_out  : std_logic_vector(7 downto 0) := (others => '0');
    signal   alu_output  : std_logic_vector(7 downto 0) := (others => '0');
    signal   check_sig   : natural := 0;
    constant CLK_PERIOD  : time := 250 us;

    component alu is
        port (
            clk         : in    std_logic;
            reset       : in    std_logic;
            enable      : in    std_logic;
            input_w_reg : in    std_logic_vector(7 downto 0);
            output_mux  : in    std_logic_vector(7 downto 0);
            opcode      : in    std_logic_vector(5 downto 0);
            status_in   : in    std_logic_vector(7 downto 0);
            bit_idx     : in    std_logic_vector(2 downto 0);
            status_out  : out   std_logic_vector(7 downto 0);
            alu_output  : out   std_logic_vector(7 downto 0)
        );
    end component;

begin

    alu_instance : component alu
        port map (
            clk         => clk,
            reset       => reset,
            enable      => enable,
            input_w_reg => input_w_reg,
            output_mux  => output_mux,
            opcode      => opcode,
            status_in   => status_in,
            bit_idx     => bit_idx,
            status_out  => status_out,
            alu_output  => alu_output
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

        procedure test_addwf (
            input_1    : in integer;
            input_2    : in integer;
            alu_res    : in integer;
            status_res : in integer) is
        begin

            reset       <= '1';
            enable      <= '1';
            wait for CLK_PERIOD * 2;
            reset       <= '0';
            opcode      <= "000111";
            input_w_reg <= std_logic_vector(to_unsigned(input_1, 8));
            output_mux  <= std_logic_vector(to_unsigned(input_2, 8));
            wait for CLK_PERIOD * 2;
            check_equal(alu_output, std_logic_vector(to_unsigned(alu_res, 8)),
                        "Comparing alu_output against reference failed.");
            check_equal(status_out, std_logic_vector(to_unsigned(status_res, 8)),
                        "Comparing status_out against reference failed.");

        end procedure test_addwf;

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
                check_equal(alu_output, std_logic_vector(to_unsigned(0, 8)),
                            "Comparing alu_output against reference failed.");
                check_equal(status_out, std_logic_vector(to_unsigned(0, 8)),
                            "Comparing status_out against reference failed.");
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_addwf_without_status_change") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_addwf_without_status_change");
                info("--------------------------------------------------------------------------------");
                test_addwf(1, 3, 4, 0);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_addwf_status_bit_zero") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_addwf_status_bit_zero");
                info("--------------------------------------------------------------------------------");
                test_addwf(128, 129, 1, 1);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_addwf_status_bit_one") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_addwf_status_bit_one");
                info("--------------------------------------------------------------------------------");
                test_addwf(8, 8, 16, 2);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_addwf_status_bit_two") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_addwf_status_bit_two");
                info("--------------------------------------------------------------------------------");
                test_addwf(0, 0, 0, 4);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_andwf") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_andwf");
                info("--------------------------------------------------------------------------------");
                reset       <= '1';
                enable      <= '1';
                wait until rising_edge(clk);
                reset       <= '0';
                opcode      <= "000101";
                input_w_reg <= std_logic_vector(to_unsigned(15, 8));
                output_mux  <= std_logic_vector(to_unsigned(37, 8));
                wait for CLK_PERIOD * 2;
                check_equal(alu_output, std_logic_vector(to_unsigned(5, 8)),
                            "Comparing alu_output against reference failed.");
                check_equal(status_out, std_logic_vector(to_unsigned(0, 8)),
                            "Comparing status_out against reference failed.");
                wait until rising_edge(clk);
                reset       <= '1';
                wait until rising_edge(clk);
                reset       <= '0';
                input_w_reg <= (others => '0');
                output_mux  <= (others => '0');
                wait for CLK_PERIOD * 2;
                check_equal(alu_output, std_logic_vector(to_unsigned(0, 8)),
                            "Comparing alu_output against reference failed.");
                check_equal(status_out, std_logic_vector(to_unsigned(4, 8)),
                            "Comparing status_out against reference failed.");
                check_sig   <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_bcf") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_bcf");
                info("--------------------------------------------------------------------------------");
                reset      <= '1';
                enable     <= '1';
                wait until rising_edge(clk);
                reset      <= '0';
                opcode     <= "010000";
                output_mux <= std_logic_vector(to_unsigned(100, 8));
                bit_idx    <= std_logic_vector(to_unsigned(2, 3));
                wait for CLK_PERIOD * 2;
                check_equal(alu_output, std_logic_vector(to_unsigned(96, 8)),
                            "Comparing alu_output against reference failed.");
                check_equal(status_out, std_logic_vector(to_unsigned(0, 8)),
                            "Comparing status_out against reference failed.");
                wait until rising_edge(clk);
                reset      <= '1';
                wait until rising_edge(clk);
                reset      <= '0';
                output_mux <= std_logic_vector(to_unsigned(16, 8));
                bit_idx    <= std_logic_vector(to_unsigned(4, 3));
                wait for CLK_PERIOD * 2;
                check_equal(alu_output, std_logic_vector(to_unsigned(0, 8)),
                            "Comparing alu_output against reference failed.");
                check_equal(status_out, std_logic_vector(to_unsigned(4, 8)),
                            "Comparing status_out against reference failed.");
                check_sig  <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_bsf") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_bsf");
                info("--------------------------------------------------------------------------------");
                reset      <= '1';
                enable     <= '1';
                wait until rising_edge(clk);
                reset      <= '0';
                opcode     <= "010100";
                output_mux <= std_logic_vector(to_unsigned(100, 8));
                bit_idx    <= std_logic_vector(to_unsigned(0, 3));
                wait for CLK_PERIOD * 2;
                check_equal(alu_output, std_logic_vector(to_unsigned(101, 8)),
                            "Comparing alu_output against reference failed.");
                check_equal(status_out, std_logic_vector(to_unsigned(0, 8)),
                            "Comparing status_out against reference failed.");
                check_sig  <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_clr") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_clr");
                info("--------------------------------------------------------------------------------");
                reset       <= '1';
                enable      <= '1';
                wait until rising_edge(clk);
                reset       <= '0';
                opcode      <= "000001";
                input_w_reg <= std_logic_vector(to_unsigned(56, 8));
                output_mux  <= std_logic_vector(to_unsigned(128, 8));
                wait for CLK_PERIOD * 2;
                check_equal(alu_output, std_logic_vector(to_unsigned(0, 8)),
                            "Comparing alu_output against reference failed.");
                check_equal(status_out, std_logic_vector(to_unsigned(4, 8)),
                            "Comparing status_out against reference failed.");
                check_sig   <= 1;
                info("===== TEST CASE FINISHED =====");
            end if;

        end loop;

        test_runner_cleanup(runner);

    end process test_runner;

end architecture tb;
