library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;
use work.constants_package.all;

entity ram is
    port (
        clk                            : in    std_logic;
        reset                          : in    std_logic;
        data                           : in    std_logic_vector(7 downto 0);
        status_in                      : in    std_logic_vector(7 downto 0);
        address                        : in    std_logic_vector(6 downto 0);
        write_enable                   : in    std_logic;
        read_enable                    : in    std_logic;
        mem_dump_enable                : in    std_logic;
        status_write_enable            : in    std_logic;
        timer_write_enable             : in    std_logic;
        eeprom_read                    : in    std_logic;
        timer_external_counter_falling : in    std_logic_vector(7 downto 0);
        timer_external_counter_rising  : in    std_logic_vector(7 downto 0);
        eeprom_data_in                 : in    std_logic_vector(7 downto 0);
        edge_trigger                   : out   std_logic;
        mem_dump                       : out   std_logic_vector(1015 downto 0);
        data_out                       : out   std_logic_vector(7 downto 0);
        status_out                     : out   std_logic_vector(7 downto 0);
        eedata_out                     : out   std_logic_vector(7 downto 0);
        eeadr_out                      : out   std_logic_vector(7 downto 0);
        eecon_out                      : out   std_logic_vector(7 downto 0)
    );
end entity ram;

architecture rtl of ram is

    type memory is array(126 downto 0) of std_logic_vector(7 downto 0);

    signal ram_block   : memory;
    signal reset_eecon : std_logic;

    function ram_to_linear (memory_data : in memory)

        return std_logic_vector is

        variable linear_data : std_logic_vector(1015 downto 0);

    begin

        linear_data := (others => '0');

        for i in 0 to (memory_data'length) - 1 loop
            linear_data(i * 8 + 7 downto i * 8) := memory_data(i);
        end loop;

        return linear_data;

    end ram_to_linear;

begin

    func : process (all) is

        procedure write_ram (
            signal address : in std_logic_vector(6 downto 0);
            signal data    : in std_logic_vector(7 downto 0)) is

            variable fsr_value : std_logic_vector(6 downto 0);

        begin

            fsr_value := "0000000";

            if (address = STATUS_ADDRESS) then
                ram_block(to_integer(unsigned(address)))(7 downto 5) <= data(7 downto 5);
            elsif (address = INDF_ADDRESS) then
                fsr_value := ram_block(to_integer(unsigned(FSR_ADDRESS)))(6 downto 0);
                if (fsr_value = "0000000") then
                    ram_block(to_integer(unsigned(fsr_value))) <= "00000000";
                else
                    ram_block(to_integer(unsigned(fsr_value))) <= data;
                end if;
            else
                ram_block(to_integer(unsigned(address))) <= data;
                if (address = EECON_ADDRESS) then
                    reset_eecon <= '1';
                end if;
            end if;

        end write_ram;

        procedure read_ram (
            signal address  : in std_logic_vector(6 downto 0);
            signal data_out : out std_logic_vector(7 downto 0)) is

            variable fsr_value : std_logic_vector(6 downto 0);

        begin

            fsr_value := "0000000";

            if (address = INDF_ADDRESS) then
                fsr_value := ram_block(to_integer(unsigned(FSR_ADDRESS)))(6 downto 0);
                data_out  <= ram_block(to_integer(unsigned(fsr_value)));
            else
                data_out <= ram_block(to_integer(unsigned(address)));
            end if;

        end read_ram;

    begin

        if (reset = '1') then
            ram_block    <= (others => (others => '0'));
            data_out     <= (others => '0');
            status_out   <= (others => '0');
            edge_trigger <= '0';
            mem_dump     <= (others => '0');
            eedata_out   <= (others => '0');
            eeadr_out    <= (others => '0');
            eecon_out    <= (others => '0');
            reset_eecon  <= '0';
        elsif (rising_edge(clk)) then
            status_out   <= ram_block(to_integer(unsigned(STATUS_ADDRESS)));
            edge_trigger <= ram_block(to_integer(unsigned(OPTION_ADDRESS)))(4);
            eedata_out   <= ram_block(to_integer(unsigned(EEDATA_ADDRESS)));
            eeadr_out    <= ram_block(to_integer(unsigned(EEADR_ADDRESS)));
            eecon_out    <= ram_block(to_integer(unsigned(EECON_ADDRESS)));
            if (eeprom_read = '1') then
                ram_block(to_integer(unsigned(EEDATA_ADDRESS))) <= eeprom_data_in;
            end if;
            if (reset_eecon = '1') then
                ram_block(to_integer(unsigned(address))) <= (others => '0');
                reset_eecon                              <= '0';
            end if;
            if (mem_dump_enable = '1') then
                mem_dump <= ram_to_linear(ram_block);
            elsif (read_enable = '1') then
                read_ram(address, data_out);
            elsif (write_enable = '1') then
                write_ram(address, data);
            end if;
            if (status_write_enable = '1') then
                ram_block(to_integer(unsigned(STATUS_ADDRESS)))(2 downto 0) <= status_in(2 downto 0);
            end if;
            if (timer_write_enable = '1' and ram_block(to_integer(unsigned(OPTION_ADDRESS)))(5) = '0') then
                ram_block(to_integer(unsigned(TMR0_ADDRESS))) <=
                    std_logic_vector(unsigned(ram_block(to_integer(unsigned(TMR0_ADDRESS)))) + 1);
            end if;
            if (ram_block(to_integer(unsigned(OPTION_ADDRESS)))(5) = '1') then
                ram_block(to_integer(unsigned(TMR0_ADDRESS))) <=
                    std_logic_vector(unsigned(timer_external_counter_falling) +
                                     unsigned(timer_external_counter_rising));
            end if;
        end if;

    end process func;

end architecture rtl;