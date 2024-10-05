library ieee;
library vunit_lib;
    context vunit_lib.vunit_context;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity tb_serial_to_parallel_instruction is
    generic (
        runner_cfg : string := runner_cfg_default
    );
end entity tb_serial_to_parallel_instruction;

architecture tb of tb_serial_to_parallel_instruction is

    signal   clk                : std_logic := '0';
    signal   reset              : std_logic := '0';
    signal   enable             : std_logic := '0';
    signal   binary_string      : std_logic_vector(13 downto 0) := (others => '0');
    signal   sel_alu_input_mux  : std_logic := '0';
    signal   sel_alu_demux      : std_logic := '0';
    signal   trig_state_machine : std_logic := '0';
    signal   transfer_to_sw     : std_logic := '0';
    signal   instruction_type   : std_logic_vector(2 downto 0) := (others => '0');
    signal   literal_out        : std_logic_vector(7 downto 0) := (others => '0');
    signal   address_out        : std_logic_vector(6 downto 0) := (others => '0');
    signal   bit_idx_out        : std_logic_vector(2 downto 0) := (others => '0');
    signal   opcode_out         : std_logic_vector(5 downto 0) := (others => '0');
    signal   check_sig          : natural := 0;
    constant CLK_PERIOD         : time := 250 us;

    component serial_to_parallel_instruction is
        port (
            clk                : in    std_logic;
            reset              : in    std_logic;
            enable             : in    std_logic;
            binary_string      : in    std_logic_vector(13 downto 0);
            sel_alu_input_mux  : out   std_logic;
            sel_alu_demux      : out   std_logic;
            trig_state_machine : out   std_logic;
            transfer_to_sw     : out   std_logic;
            instruction_type   : out   std_logic_vector(2 downto 0);
            literal_out        : out   std_logic_vector(7 downto 0);
            address_out        : out   std_logic_vector(6 downto 0);
            bit_idx_out        : out   std_logic_vector(2 downto 0);
            opcode_out         : out   std_logic_vector(5 downto 0)
        );
    end component;

begin

    serial_to_parallel_instruction_instance : component serial_to_parallel_instruction
        port map (
            clk                => clk,
            reset              => reset,
            enable             => enable,
            binary_string      => binary_string,
            sel_alu_input_mux  => sel_alu_input_mux,
            sel_alu_demux      => sel_alu_demux,
            trig_state_machine => trig_state_machine,
            transfer_to_sw     => transfer_to_sw,
            instruction_type   => instruction_type,
            literal_out        => literal_out,
            address_out        => address_out,
            bit_idx_out        => bit_idx_out,
            opcode_out         => opcode_out
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
                check_equal(literal_out, std_logic_vector(to_unsigned(0, 8)),
                            "Comparing literal_out against reference failed.");
                check_equal(address_out, std_logic_vector(to_unsigned(0, 7)),
                            "Comparing address_out against reference failed.");
                check_equal(opcode_out, std_logic_vector(to_unsigned(0, 6)),
                            "Comparing opcode_out against reference failed.");
                check_equal(bit_idx_out, std_logic_vector(to_unsigned(0, 3)),
                            "Comparing bit_idx_out against reference failed.");
                check_equal(sel_alu_input_mux, '0',
                            "Comparing sel_alu_input_mux against reference failed.");
                check_equal(sel_alu_demux, '0',
                            "Comparing sel_alu_demux against reference failed.");
                check_equal(trig_state_machine, '0',
                            "Comparing trig_state_machine against reference failed.");
                check_equal(transfer_to_sw, '0',
                            "Comparing transfer_to_sw against reference failed.");
                check_equal(instruction_type, std_logic_vector(to_unsigned(0, 3)),
                            "Comparing instruction_tpe against reference failed.");
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_bit_oriented_instruction") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_bit_oriented_instruction");
                info("--------------------------------------------------------------------------------");
                reset         <= '1';
                enable        <= '1';
                wait for CLK_PERIOD * 2;
                reset         <= '0';
                binary_string <= "01010010100011";
                wait for CLK_PERIOD * 2;
                check_equal(literal_out, std_logic_vector(to_unsigned(0, 8)),
                            "Comparing literal_out against reference failed.");
                check_equal(address_out, std_logic_vector(unsigned'("0100011")),
                            "Comparing address_out against reference failed.");
                check_equal(opcode_out, std_logic_vector(unsigned'("010100")),
                            "Comparing opcode_out against reference failed.");
                check_equal(bit_idx_out, std_logic_vector(unsigned'("001")),
                            "Comparing bit_idx_out against reference failed.");
                check_equal(sel_alu_input_mux, '1',
                            "Comparing sel_alu_input_mux against reference failed.");
                check_equal(sel_alu_demux, '1',
                            "Comparing sel_alu_demux against reference failed.");
                check_equal(trig_state_machine, '1',
                            "Comparing trig_state_machine against reference failed.");
                check_equal(transfer_to_sw, '0',
                            "Comparing transfer_to_sw against reference failed.");
                check_equal(instruction_type, std_logic_vector(unsigned'("010")),
                            "Comparing instruction_tpe against reference failed.");
                check_sig     <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_dump_eeprom") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_dump_eeprom");
                info("--------------------------------------------------------------------------------");
                reset         <= '1';
                enable        <= '1';
                wait for CLK_PERIOD * 2;
                reset         <= '0';
                binary_string <= "10110011010110";
                wait for CLK_PERIOD * 2;
                check_equal(literal_out, std_logic_vector(to_unsigned(0, 8)),
                            "Comparing literal_out against reference failed.");
                check_equal(address_out, std_logic_vector(to_unsigned(0, 7)),
                            "Comparing address_out against reference failed.");
                check_equal(opcode_out, std_logic_vector(unsigned'("101100")),
                            "Comparing opcode_out against reference failed.");
                check_equal(bit_idx_out, std_logic_vector(to_unsigned(0, 3)),
                            "Comparing bit_idx_out against reference failed.");
                check_equal(sel_alu_input_mux, '0',
                            "Comparing sel_alu_input_mux against reference failed.");
                check_equal(sel_alu_demux, '0',
                            "Comparing sel_alu_demux against reference failed.");
                check_equal(trig_state_machine, '1',
                            "Comparing trig_state_machine against reference failed.");
                check_equal(transfer_to_sw, '0',
                            "Comparing transfer_to_sw against reference failed.");
                check_equal(instruction_type, std_logic_vector(unsigned'("110")),
                            "Comparing instruction_tpe against reference failed.");
                check_sig     <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_dump_ram") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_dump_ram");
                info("--------------------------------------------------------------------------------");
                reset         <= '1';
                enable        <= '1';
                wait for CLK_PERIOD * 2;
                reset         <= '0';
                binary_string <= "10100011010110";
                wait for CLK_PERIOD * 2;
                check_equal(literal_out, std_logic_vector(to_unsigned(0, 8)),
                            "Comparing literal_out against reference failed.");
                check_equal(address_out, std_logic_vector(to_unsigned(0, 7)),
                            "Comparing address_out against reference failed.");
                check_equal(opcode_out, std_logic_vector(unsigned'("101000")),
                            "Comparing opcode_out against reference failed.");
                check_equal(bit_idx_out, std_logic_vector(to_unsigned(0, 3)),
                            "Comparing bit_idx_out against reference failed.");
                check_equal(sel_alu_input_mux, '0',
                            "Comparing sel_alu_input_mux against reference failed.");
                check_equal(sel_alu_demux, '0',
                            "Comparing sel_alu_demux against reference failed.");
                check_equal(trig_state_machine, '1',
                            "Comparing trig_state_machine against reference failed.");
                check_equal(transfer_to_sw, '0',
                            "Comparing transfer_to_sw against reference failed.");
                check_equal(instruction_type, std_logic_vector(unsigned'("101")),
                            "Comparing instruction_tpe against reference failed.");
                check_sig     <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_read_address") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_read_address");
                info("--------------------------------------------------------------------------------");
                reset         <= '1';
                enable        <= '1';
                wait for CLK_PERIOD * 2;
                reset         <= '0';
                binary_string <= "11001111010110";
                wait for CLK_PERIOD * 2;
                check_equal(literal_out, std_logic_vector(to_unsigned(0, 8)),
                            "Comparing literal_out against reference failed.");
                check_equal(address_out, std_logic_vector(unsigned'("1010110")),
                            "Comparing address_out against reference failed.");
                check_equal(opcode_out, std_logic_vector(unsigned'("110011")),
                            "Comparing opcode_out against reference failed.");
                check_equal(bit_idx_out, std_logic_vector(to_unsigned(0, 3)),
                            "Comparing bit_idx_out against reference failed.");
                check_equal(sel_alu_input_mux, '1',
                            "Comparing sel_alu_input_mux against reference failed.");
                check_equal(sel_alu_demux, '0',
                            "Comparing sel_alu_demux against reference failed.");
                check_equal(trig_state_machine, '1',
                            "Comparing trig_state_machine against reference failed.");
                check_equal(transfer_to_sw, '1',
                            "Comparing transfer_to_sw against reference failed.");
                check_equal(instruction_type, std_logic_vector(unsigned'("100")),
                            "Comparing instruction_tpe against reference failed.");
                check_sig     <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_read_wreg") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_read_wreg");
                info("--------------------------------------------------------------------------------");
                reset         <= '1';
                enable        <= '1';
                wait for CLK_PERIOD * 2;
                reset         <= '0';
                binary_string <= "11000111010110";
                wait for CLK_PERIOD * 2;
                check_equal(literal_out, std_logic_vector(to_unsigned(0, 8)),
                            "Comparing literal_out against reference failed.");
                check_equal(address_out, std_logic_vector(to_unsigned(0, 7)),
                            "Comparing address_out against reference failed.");
                check_equal(opcode_out, std_logic_vector(unsigned'("110001")),
                            "Comparing opcode_out against reference failed.");
                check_equal(bit_idx_out, std_logic_vector(to_unsigned(0, 3)),
                            "Comparing bit_idx_out against reference failed.");
                check_equal(sel_alu_input_mux, '0',
                            "Comparing sel_alu_input_mux against reference failed.");
                check_equal(sel_alu_demux, '0',
                            "Comparing sel_alu_demux against reference failed.");
                check_equal(trig_state_machine, '1',
                            "Comparing trig_state_machine against reference failed.");
                check_equal(transfer_to_sw, '1',
                            "Comparing transfer_to_sw against reference failed.");
                check_equal(instruction_type, std_logic_vector(unsigned'("011")),
                            "Comparing instruction_tpe against reference failed.");
                check_sig     <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_byte_oriented_instruction") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_byte_oriented_instruction");
                info("--------------------------------------------------------------------------------");
                reset         <= '1';
                enable        <= '1';
                wait for CLK_PERIOD * 2;
                reset         <= '0';
                binary_string <= "00010011010110";
                wait for CLK_PERIOD * 2;
                check_equal(literal_out, std_logic_vector(to_unsigned(0, 8)),
                            "Comparing literal_out against reference failed.");
                check_equal(address_out, std_logic_vector(unsigned'("1010110")),
                            "Comparing address_out against reference failed.");
                check_equal(opcode_out, std_logic_vector(unsigned'("000100")),
                            "Comparing opcode_out against reference failed.");
                check_equal(bit_idx_out, std_logic_vector(to_unsigned(0, 3)),
                            "Comparing bit_idx_out against reference failed.");
                check_equal(sel_alu_input_mux, '1',
                            "Comparing sel_alu_input_mux against reference failed.");
                check_equal(sel_alu_demux, '1',
                            "Comparing sel_alu_demux against reference failed.");
                check_equal(trig_state_machine, '1',
                            "Comparing trig_state_machine against reference failed.");
                check_equal(transfer_to_sw, '0',
                            "Comparing transfer_to_sw against reference failed.");
                check_equal(instruction_type, std_logic_vector(unsigned'("010")),
                            "Comparing instruction_tpe against reference failed.");
                check_sig     <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_literal_instruction") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_literal_instruction");
                info("--------------------------------------------------------------------------------");
                reset         <= '1';
                enable        <= '1';
                wait for CLK_PERIOD * 2;
                reset         <= '0';
                binary_string <= "11100111010110";
                wait for CLK_PERIOD * 2;
                check_equal(literal_out, std_logic_vector(unsigned'("11010110")),
                            "Comparing literal_out against reference failed.");
                check_equal(address_out, std_logic_vector(to_unsigned(0, 7)),
                            "Comparing address_out against reference failed.");
                check_equal(opcode_out, std_logic_vector(unsigned'("111001")),
                            "Comparing opcode_out against reference failed.");
                check_equal(bit_idx_out, std_logic_vector(to_unsigned(0, 3)),
                            "Comparing bit_idx_out against reference failed.");
                check_equal(sel_alu_input_mux, '0',
                            "Comparing sel_alu_input_mux against reference failed.");
                check_equal(sel_alu_demux, '0',
                            "Comparing sel_alu_demux against reference failed.");
                check_equal(trig_state_machine, '1',
                            "Comparing trig_state_machine against reference failed.");
                check_equal(transfer_to_sw, '0',
                            "Comparing transfer_to_sw against reference failed.");
                check_equal(instruction_type, std_logic_vector(unsigned'("000")),
                            "Comparing instruction_tpe against reference failed.");
                check_sig     <= 1;
                info("===== TEST CASE FINISHED =====");
            end if;

        end loop;

        test_runner_cleanup(runner);

    end process test_runner;

end architecture tb;
