library ieee;
library vunit_lib;
    context vunit_lib.vunit_context;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity tb_hps_adder is
    generic (
        runner_cfg : string := runner_cfg_default
    );
end entity tb_hps_adder;

architecture tb of tb_hps_adder is

    signal   clk        : std_logic := '0';
    signal   reset      : std_logic := '0';
    signal   read       : std_logic := '0';
    signal   write      : std_logic := '0';
    signal   writedata  : std_logic_vector(15 downto 0) := (others => '0');
    signal   address    : std_logic_vector(1 downto 0) := (others => '0');
    signal   readdata   : std_logic_vector(15 downto 0) := (others => '0');
    signal   check_sig  : natural := 0;
    constant CLK_PERIOD : time := 250 us;

    component hps_adder is
        port (
            clk       : in    std_logic;
            reset     : in    std_logic;
            read      : in    std_logic;
            write     : in    std_logic;
            writedata : in    std_logic_vector(15 downto 0);
            address   : in    std_logic_vector(1 downto 0);
            readdata  : out   std_logic_vector(15 downto 0)
        );
    end component;

begin

    hps_adder_instance : component hps_adder
        port map (
            clk       => clk,
            reset     => reset,
            read      => read,
            write     => write,
            writedata => writedata,
            address   => address,
            readdata  => readdata
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
                check_equal(readdata, std_logic_vector(unsigned'("0000000000000000")),
                            "Comparing readdata against reference failed.");
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_write_read_address_zero") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_write_read_address_zero");
                info("--------------------------------------------------------------------------------");
                reset     <= '1';
                wait for CLK_PERIOD * 2;
                reset     <= '0';
                writedata <= std_logic_vector(to_unsigned(68, 16));
                address   <= "00";
                read      <= '0';
                write     <= '1';
                wait for CLK_PERIOD;
                read      <= '1';
                write     <= '0';
                wait for CLK_PERIOD;
                check_equal(readdata, std_logic_vector(unsigned'("1000000001000100")),
                            "Comparing readdata against reference failed.");
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_write_read_address_one") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_write_read_address_one");
                info("--------------------------------------------------------------------------------");
                reset     <= '1';
                wait for CLK_PERIOD * 2;
                reset     <= '0';
                writedata <= std_logic_vector(to_unsigned(68, 16));
                address   <= "01";
                read      <= '0';
                write     <= '1';
                wait for CLK_PERIOD;
                read      <= '1';
                write     <= '0';
                wait for CLK_PERIOD;
                check_equal(readdata, std_logic_vector(unsigned'("1000000001000100")),
                            "Comparing readdata against reference failed.");
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            elsif run("test_write_read_address_two") then
                info("--------------------------------------------------------------------------------");
                info("TEST CASE: test_write_read_address_two");
                info("--------------------------------------------------------------------------------");
                reset     <= '1';
                wait for CLK_PERIOD * 2;
                reset     <= '0';
                writedata <= std_logic_vector(to_unsigned(50, 16));
                address   <= "00";
                read      <= '0';
                write     <= '1';
                wait for CLK_PERIOD;
                write     <= '0';
                wait for CLK_PERIOD;
                writedata <= std_logic_vector(to_unsigned(123, 16));
                address   <= "01";
                write     <= '1';
                wait for CLK_PERIOD;
                read      <= '1';
                write     <= '0';
                address   <= "10";
                wait for CLK_PERIOD;
                check_equal(readdata, std_logic_vector(unsigned'("1000000010101101")),
                            "Comparing readdata against reference failed.");
                check_sig <= 1;
                info("===== TEST CASE FINISHED =====");
            end if;

        end loop;

        test_runner_cleanup(runner);

    end process test_runner;

end architecture tb;
