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

        procedure test_andwf (
            input_1    : in integer;
            input_2    : in integer;
            alu_res    : in integer;
            status_res : in integer) is
        begin

            reset       <= '1';
            enable      <= '1';
            wait for CLK_PERIOD * 2;
            reset       <= '0';
            opcode      <= "000101";
            input_w_reg <= std_logic_vector(to_unsigned(input_1, 8));
            output_mux  <= std_logic_vector(to_unsigned(input_2, 8));
            wait for CLK_PERIOD * 2;
            check_equal(alu_output, std_logic_vector(to_unsigned(alu_res, 8)),
                        "Comparing alu_output against reference failed.");
            check_equal(status_out, std_logic_vector(to_unsigned(status_res, 8)),
                        "Comparing status_out against reference failed.");

        end procedure test_andwf;

        procedure test_bcf (
            input_1    : in integer;
            input_2    : in integer;
            alu_res    : in integer;
            status_res : in integer) is
        begin

            reset      <= '1';
            enable     <= '1';
            wait for CLK_PERIOD * 2;
            reset      <= '0';
            opcode     <= "010000";
            output_mux <= std_logic_vector(to_unsigned(input_1, 8));
            bit_idx    <= std_logic_vector(to_unsigned(input_2, 3));
            wait for CLK_PERIOD * 2;
            check_equal(alu_output, std_logic_vector(to_unsigned(alu_res, 8)),
                        "Comparing alu_output against reference failed.");
            check_equal(status_out, std_logic_vector(to_unsigned(status_res, 8)),
                        "Comparing status_out against reference failed.");

        end procedure test_bcf;

        procedure test_comf (
            input_1    : in integer;
            alu_res    : in integer;
            status_res : in integer) is
        begin

            reset      <= '1';
            enable     <= '1';
            wait for CLK_PERIOD * 2;
            reset      <= '0';
            opcode     <= "001001";
            output_mux <= std_logic_vector(to_unsigned(input_1, 8));
            wait for CLK_PERIOD * 2;
            check_equal(alu_output, std_logic_vector(to_unsigned(alu_res, 8)),
                        "Comparing alu_output against reference failed.");
            check_equal(status_out, std_logic_vector(to_unsigned(status_res, 8)),
                        "Comparing status_out against reference failed.");

        end procedure test_comf;

        procedure test_decf (
            input_1    : in integer;
            alu_res    : in integer;
            status_res : in integer) is
        begin

            reset      <= '1';
            enable     <= '1';
            wait for CLK_PERIOD * 2;
            reset      <= '0';
            opcode     <= "000011";
            output_mux <= std_logic_vector(to_unsigned(input_1, 8));
            wait for CLK_PERIOD * 2;
            check_equal(alu_output, std_logic_vector(to_unsigned(alu_res, 8)),
                        "Comparing alu_output against reference failed.");
            check_equal(status_out, std_logic_vector(to_unsigned(status_res, 8)),
                        "Comparing status_out against reference failed.");

        end procedure test_decf;

        procedure test_incf (
            input_1    : in integer;
            alu_res    : in integer;
            status_res : in integer) is
        begin

            reset      <= '1';
            enable     <= '1';
            wait for CLK_PERIOD * 2;
            reset      <= '0';
            opcode     <= "001010";
            output_mux <= std_logic_vector(to_unsigned(input_1, 8));
            wait for CLK_PERIOD * 2;
            check_equal(alu_output, std_logic_vector(to_unsigned(alu_res, 8)),
                        "Comparing alu_output against reference failed.");
            check_equal(status_out, std_logic_vector(to_unsigned(status_res, 8)),
                        "Comparing status_out against reference failed.");

        end procedure test_incf;

        procedure test_iorwf (
            input_1    : in integer;
            input_2    : in integer;
            alu_res    : in integer;
            status_res : in integer) is
        begin

            reset       <= '1';
            enable      <= '1';
            wait for CLK_PERIOD * 2;
            reset       <= '0';
            opcode      <= "000100";
            output_mux  <= std_logic_vector(to_unsigned(input_1, 8));
            input_w_reg <= std_logic_vector(to_unsigned(input_2, 8));
            wait for CLK_PERIOD * 2;
            check_equal(alu_output, std_logic_vector(to_unsigned(alu_res, 8)),
                        "Comparing alu_output against reference failed.");
            check_equal(status_out, std_logic_vector(to_unsigned(status_res, 8)),
                        "Comparing status_out against reference failed.");

        end procedure test_iorwf;

        procedure test_movf (
            input_1    : in integer;
            alu_res    : in integer;
            status_res : in integer) is
        begin

            reset      <= '1';
            enable     <= '1';
            wait for CLK_PERIOD * 2;
            reset      <= '0';
            opcode     <= "001000";
            output_mux <= std_logic_vector(to_unsigned(input_1, 8));
            wait for CLK_PERIOD * 2;
            check_equal(alu_output, std_logic_vector(to_unsigned(alu_res, 8)),
                        "Comparing alu_output against reference failed.");
            check_equal(status_out, std_logic_vector(to_unsigned(status_res, 8)),
                        "Comparing status_out against reference failed.");

        end procedure test_movf;

        procedure test_subwf (
            input_1 : in integer;
            input_2 : in integer;
            alu_res : in integer) is
        begin

            reset       <= '1';
            enable      <= '1';
            wait for CLK_PERIOD * 2;
            reset       <= '0';
            opcode      <= "000010";
            input_w_reg <= std_logic_vector(to_unsigned(input_1, 8));
            output_mux  <= std_logic_vector(to_unsigned(input_2, 8));
            wait for CLK_PERIOD * 2;
            check_equal(alu_output, std_logic_vector(to_unsigned(alu_res, 8)),
                        "Comparing alu_output against reference failed.");

        end procedure test_subwf;

        procedure test_xorwf (
            input_1    : in integer;
            input_2    : in integer;
            alu_res    : in integer;
            status_res : in integer) is
        begin

            reset       <= '1';
            enable      <= '1';
            wait for CLK_PERIOD * 2;
            reset       <= '0';
            opcode      <= "000110";
            output_mux  <= std_logic_vector(to_unsigned(input_1, 8));
            input_w_reg <= std_logic_vector(to_unsigned(input_2, 8));
            wait for CLK_PERIOD * 2;
            check_equal(alu_output, std_logic_vector(to_unsigned(alu_res, 8)),
                        "Comparing alu_output against reference failed.");
            check_equal(status_out, std_logic_vector(to_unsigned(status_res, 8)),
                        "Comparing status_out against reference failed.");

        end procedure test_xorwf;

        procedure test_addlw (
            input_1    : in integer;
            input_2    : in integer;
            alu_res    : in integer;
            status_res : in integer) is
        begin

            reset       <= '1';
            enable      <= '1';
            wait for CLK_PERIOD * 2;
            reset       <= '0';
            opcode      <= "111110";
            input_w_reg <= std_logic_vector(to_unsigned(input_1, 8));
            output_mux  <= std_logic_vector(to_unsigned(input_2, 8));
            wait for CLK_PERIOD * 2;
            check_equal(alu_output, std_logic_vector(to_unsigned(alu_res, 8)),
                        "Comparing alu_output against reference failed.");
            check_equal(status_out, std_logic_vector(to_unsigned(status_res, 8)),
                        "Comparing status_out against reference failed.");

        end procedure test_addlw;

        procedure test_andlw (
            input_1    : in integer;
            input_2    : in integer;
            alu_res    : in integer;
            status_res : in integer) is
        begin

            reset       <= '1';
            enable      <= '1';
            wait for CLK_PERIOD * 2;
            reset       <= '0';
            opcode      <= "111001";
            input_w_reg <= std_logic_vector(to_unsigned(input_1, 8));
            output_mux  <= std_logic_vector(to_unsigned(input_2, 8));
            wait for CLK_PERIOD * 2;
            check_equal(alu_output, std_logic_vector(to_unsigned(alu_res, 8)),
                        "Comparing alu_output against reference failed.");
            check_equal(status_out, std_logic_vector(to_unsigned(status_res, 8)),
                        "Comparing status_out against reference failed.");

        end procedure test_andlw;

        procedure test_iorlw (
            input_1    : in integer;
            input_2    : in integer;
            alu_res    : in integer;
            status_res : in integer) is
        begin

            reset       <= '1';
            enable      <= '1';
            wait for CLK_PERIOD * 2;
            reset       <= '0';
            opcode      <= "111000";
            output_mux  <= std_logic_vector(to_unsigned(input_1, 8));
            input_w_reg <= std_logic_vector(to_unsigned(input_2, 8));
            wait for CLK_PERIOD * 2;
            check_equal(alu_output, std_logic_vector(to_unsigned(alu_res, 8)),
                        "Comparing alu_output against reference failed.");
            check_equal(status_out, std_logic_vector(to_unsigned(status_res, 8)),
                        "Comparing status_out against reference failed.");

        end procedure test_iorlw;

        procedure test_movlw (
            input_1    : in integer;
            alu_res    : in integer;
            status_res : in integer) is
        begin

            reset      <= '1';
            enable     <= '1';
            wait for CLK_PERIOD * 2;
            reset      <= '0';
            opcode     <= "110000";
            output_mux <= std_logic_vector(to_unsigned(input_1, 8));
            wait for CLK_PERIOD * 2;
            check_equal(alu_output, std_logic_vector(to_unsigned(alu_res, 8)),
                        "Comparing alu_output against reference failed.");
            check_equal(status_out, std_logic_vector(to_unsigned(status_res, 8)),
                        "Comparing status_out against reference failed.");

        end procedure test_movlw;

        procedure test_sublw (
            input_1 : in integer;
            input_2 : in integer;
            alu_res : in integer) is
        begin

            reset       <= '1';
            enable      <= '1';
            wait for CLK_PERIOD * 2;
            reset       <= '0';
            opcode      <= "111101";
            input_w_reg <= std_logic_vector(to_unsigned(input_1, 8));
            output_mux  <= std_logic_vector(to_unsigned(input_2, 8));
            wait for CLK_PERIOD * 2;
            check_equal(alu_output, std_logic_vector(to_unsigned(alu_res, 8)),
                        "Comparing alu_output against reference failed.");

        end procedure test_sublw;

        procedure test_xorlw (
            input_1    : in integer;
            input_2    : in integer;
            alu_res    : in integer;
            status_res : in integer) is
        begin

            reset       <= '1';
            enable      <= '1';
            wait for CLK_PERIOD * 2;
            reset       <= '0';
            opcode      <= "111010";
            output_mux  <= std_logic_vector(to_unsigned(input_1, 8));
            input_w_reg <= std_logic_vector(to_unsigned(input_2, 8));
            wait for CLK_PERIOD * 2;
            check_equal(alu_output, std_logic_vector(to_unsigned(alu_res, 8)),
                        "Comparing alu_output against reference failed.");
            check_equal(status_out, std_logic_vector(to_unsigned(status_res, 8)),
                        "Comparing status_out against reference failed.");

        end procedure test_xorlw;

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
            elsif run("test_andwf_without_status_change") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_andwf_without_status_change");
                info("--------------------------------------------------------------------------------");
                test_andwf(15, 37, 5, 0);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_andwf_with_status_change") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_andwf_with_status_change");
                info("--------------------------------------------------------------------------------");
                test_andwf(0, 0, 0, 4);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_bcf_without_status_change") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_bcf_without_status_change");
                info("--------------------------------------------------------------------------------");
                test_bcf(100, 2, 96, 0);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_bcf_with_status_change") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_bcf_with_status_change");
                info("--------------------------------------------------------------------------------");
                test_bcf(16, 4, 0, 4);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_bsf") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_bsf");
                info("--------------------------------------------------------------------------------");
                reset      <= '1';
                enable     <= '1';
                wait for CLK_PERIOD * 2;
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
                wait for CLK_PERIOD * 2;
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
            elsif run("test_comf_without_status_change") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_comf_without_status_change");
                info("--------------------------------------------------------------------------------");
                test_comf(74, 181, 0);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_comf_with_status_change") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_comf_with_status_change");
                info("--------------------------------------------------------------------------------");
                test_comf(255, 0, 4);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_decf_without_status_change") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_decf_without_status_change");
                info("--------------------------------------------------------------------------------");
                test_decf(127, 126, 0);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_decf_with_status_change") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_decf_with_status_change");
                info("--------------------------------------------------------------------------------");
                test_decf(1, 0, 4);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_incf_without_status_change") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_incf_without_status_change");
                info("--------------------------------------------------------------------------------");
                test_incf(127, 128, 0);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_incf_with_status_change") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_incf_with_status_change");
                info("--------------------------------------------------------------------------------");
                test_incf(255, 0, 4);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_iorwf_without_status_change") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_iorwf_without_status_change");
                info("--------------------------------------------------------------------------------");
                test_iorwf(51, 68, 119, 0);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_iorwf_with_status_change") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_iorwf_with_status_change");
                info("--------------------------------------------------------------------------------");
                test_iorwf(0, 0, 0, 4);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_movf_without_status_change") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_movf_without_status_change");
                info("--------------------------------------------------------------------------------");
                test_movf(58, 58, 0);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_movf_with_status_change") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_movf_with_status_change");
                info("--------------------------------------------------------------------------------");
                test_movf(0, 0, 4);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_rlf") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_rlf");
                info("--------------------------------------------------------------------------------");
                reset      <= '1';
                enable     <= '1';
                wait for CLK_PERIOD * 2;
                reset      <= '0';
                opcode     <= "001101";
                output_mux <= std_logic_vector(to_unsigned(29, 8));
                status_in  <= std_logic_vector(to_unsigned(51, 8));
                wait for CLK_PERIOD * 2;
                check_equal(alu_output, std_logic_vector(to_unsigned(59, 8)),
                            "Comparing alu_output against reference failed.");
                check_equal(status_out, std_logic_vector(to_unsigned(50, 8)),
                            "Comparing status_out against reference failed.");
                check_sig  <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_rrf") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_rrf");
                info("--------------------------------------------------------------------------------");
                reset      <= '1';
                enable     <= '1';
                wait for CLK_PERIOD * 2;
                reset      <= '0';
                opcode     <= "001100";
                output_mux <= std_logic_vector(to_unsigned(29, 8));
                status_in  <= std_logic_vector(to_unsigned(51, 8));
                wait for CLK_PERIOD * 2;
                check_equal(alu_output, std_logic_vector(to_unsigned(142, 8)),
                            "Comparing alu_output against reference failed.");
                check_equal(status_out, std_logic_vector(to_unsigned(51, 8)),
                            "Comparing status_out against reference failed.");
                check_sig  <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_subwf_without_status_change") then
                -- TODO: check handling of status bits
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_subwf_without_status_change");
                info("--------------------------------------------------------------------------------");
                test_subwf(15, 45, 30);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_swapf") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_swapf");
                info("--------------------------------------------------------------------------------");
                reset      <= '1';
                enable     <= '1';
                wait for CLK_PERIOD * 2;
                reset      <= '0';
                opcode     <= "001110";
                output_mux <= std_logic_vector(to_unsigned(77, 8));
                wait for CLK_PERIOD * 2;
                check_equal(alu_output, std_logic_vector(to_unsigned(212, 8)),
                            "Comparing alu_output against reference failed.");
                check_sig  <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_xorwf_without_status_change") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_xorwf_without_status_change");
                info("--------------------------------------------------------------------------------");
                test_xorwf(51, 81, 98, 0);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_xorwf_with_status_change") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_xorwf_with_status_change");
                info("--------------------------------------------------------------------------------");
                test_xorwf(0, 0, 0, 4);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_addlw_without_status_change") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_addlw_without_status_change");
                info("--------------------------------------------------------------------------------");
                test_addlw(1, 3, 4, 0);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_addlw_status_bit_zero") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_addlw_status_bit_zero");
                info("--------------------------------------------------------------------------------");
                test_addlw(128, 129, 1, 1);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_addlw_status_bit_one") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_addlw_status_bit_one");
                info("--------------------------------------------------------------------------------");
                test_addlw(8, 8, 16, 2);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_addlw_status_bit_two") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_addlw_status_bit_two");
                info("--------------------------------------------------------------------------------");
                test_addlw(0, 0, 0, 4);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_andlw_without_status_change") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_andlw_without_status_change");
                info("--------------------------------------------------------------------------------");
                test_andlw(15, 37, 5, 0);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_andlw_with_status_change") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_andlw_with_status_change");
                info("--------------------------------------------------------------------------------");
                test_andlw(0, 0, 0, 4);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_iorlw_without_status_change") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_iorlw_without_status_change");
                info("--------------------------------------------------------------------------------");
                test_iorlw(51, 68, 119, 0);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_iorlw_with_status_change") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_iorlw_with_status_change");
                info("--------------------------------------------------------------------------------");
                test_iorlw(0, 0, 0, 4);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_movlw") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_movlw");
                info("--------------------------------------------------------------------------------");
                test_movlw(58, 58, 0);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_sublw_without_status_change") then
                -- TODO: check handling of status bits
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_sublw_without_status_change");
                info("--------------------------------------------------------------------------------");
                test_sublw(15, 45, 30);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_xorlw_without_status_change") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_xorlw_without_status_change");
                info("--------------------------------------------------------------------------------");
                test_xorlw(51, 81, 98, 0);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_xorlw_with_status_change") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_xorlw_with_status_change");
                info("--------------------------------------------------------------------------------");
                test_xorlw(0, 0, 0, 4);
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            end if;

        end loop;

        test_runner_cleanup(runner);

    end process test_runner;

end architecture tb;
