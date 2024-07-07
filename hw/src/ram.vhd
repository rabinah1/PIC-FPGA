library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;
use work.constants_package.all;

entity ram is
    port (
        clk                            : in    std_logic;
        reset                          : in    std_logic;
        write_enable                   : in    std_logic;
        read_enable                    : in    std_logic;
        mem_dump_enable                : in    std_logic;
        eeprom_dump_enable             : in    std_logic;
        status_write_enable            : in    std_logic;
        timer_write_enable             : in    std_logic;
        eeprom_read                    : in    std_logic;
        eeprom_write_completed         : in    std_logic;
        timer_external_counter_falling : in    std_logic_vector(7 downto 0);
        timer_external_counter_rising  : in    std_logic_vector(7 downto 0);
        eeprom_data_in                 : in    std_logic_vector(7 downto 0);
        data                           : in    std_logic_vector(7 downto 0);
        status_in                      : in    std_logic_vector(7 downto 0);
        address                        : in    std_logic_vector(6 downto 0);
        edge_trigger                   : out   std_logic;
        mem_dump                       : out   std_logic_vector(1015 downto 0);
        eeprom_dump                    : out   std_logic_vector(2047 downto 0);
        data_out                       : out   std_logic_vector(7 downto 0);
        status_out                     : out   std_logic_vector(7 downto 0);
        eedata_out                     : out   std_logic_vector(7 downto 0);
        eeadr_out                      : out   std_logic_vector(7 downto 0);
        eecon_out                      : out   std_logic_vector(7 downto 0)
    );
end entity ram;

architecture rtl of ram is

    type memory is array(RAM_SIZE - 1 downto 0) of std_logic_vector(7 downto 0);

    type eeprom is array(EEPROM_SIZE - 1 downto 0) of std_logic_vector(7 downto 0);

    signal ram_block   : memory;
    signal reset_eecon : std_logic;
    signal counter     : unsigned(2 downto 0);
    signal addr        : integer := 0;

    function get_addr (address : in std_logic_vector(6 downto 0))

        return integer is

    begin

        return to_integer(unsigned(address));

    end get_addr;

    function ram_to_linear (ram : in memory)

        return std_logic_vector is

        variable linear_data : std_logic_vector(1015 downto 0);

    begin

        linear_data := (others => '0');

        for i in 0 to (ram'length) - 1 loop
            linear_data(i * 8 + 7 downto i * 8) := ram(i);
        end loop;

        return linear_data;

    end ram_to_linear;

begin

    ram : process (all) is

        procedure write_ram is

            variable fsr_value : std_logic_vector(6 downto 0);

        begin

            fsr_value := "0000000";

            if (address = STATUS_ADDRESS) then
                ram_block(get_addr(address))(7 downto 5) <= data(7 downto 5);
            elsif (address = INDF_ADDRESS) then
                fsr_value := ram_block(get_addr(FSR_ADDRESS))(6 downto 0);
                if (fsr_value = "0000000") then
                    ram_block(get_addr(fsr_value)) <= "00000000";
                else
                    ram_block(get_addr(fsr_value)) <= data;
                end if;
            else
                ram_block(get_addr(address)) <= data;
                if (address = EECON_ADDRESS) then
                    reset_eecon <= '1';
                end if;
            end if;

        end write_ram;

        procedure read_ram is

            variable fsr_value : std_logic_vector(6 downto 0);

        begin

            fsr_value := "0000000";

            if (address = INDF_ADDRESS) then
                fsr_value := ram_block(get_addr(FSR_ADDRESS))(6 downto 0);
                data_out  <= ram_block(get_addr(fsr_value));
            else
                data_out <= ram_block(get_addr(address));
            end if;

        end read_ram;

        procedure write_eeprom_dump is
        begin

            if (counter = to_unsigned(0, counter'length)) then
                ram_block(get_addr(EECON_ADDRESS)) <= (others => '0');
                ram_block(get_addr(EEADR_ADDRESS)) <= std_logic_vector(to_unsigned(addr, 8));
                counter                            <= to_unsigned(1, counter'length);
            elsif (counter = to_unsigned(1, counter'length)) then
                ram_block(get_addr(EECON_ADDRESS)) <= "00000001";
                ram_block(get_addr(EEADR_ADDRESS)) <= std_logic_vector(to_unsigned(addr, 8));
                counter                            <= to_unsigned(2, counter'length);
            elsif (counter = to_unsigned(2, counter'length)) then
                ram_block(get_addr(EECON_ADDRESS)) <= "00000001";
                ram_block(get_addr(EEADR_ADDRESS)) <= std_logic_vector(to_unsigned(addr, 8));
                counter                            <= to_unsigned(3, counter'length);
            elsif (counter = to_unsigned(3, counter'length)) then
                ram_block(get_addr(EECON_ADDRESS)) <= (others => '0');
                ram_block(get_addr(EEADR_ADDRESS)) <= std_logic_vector(to_unsigned(addr, 8));
                counter                            <= to_unsigned(4, counter'length);
            elsif (counter = to_unsigned(4, counter'length)) then
                ram_block(get_addr(EECON_ADDRESS)) <= (others => '0');
                ram_block(get_addr(EEADR_ADDRESS)) <= std_logic_vector(to_unsigned(addr, 8));
                counter                            <= to_unsigned(5, counter'length);
            elsif (counter = to_unsigned(5, counter'length)) then
                ram_block(get_addr(EECON_ADDRESS))        <= (others => '0');
                ram_block(get_addr(EEADR_ADDRESS))        <= std_logic_vector(to_unsigned(addr, 8));
                eeprom_dump(addr * 8 + 7 downto addr * 8) <= eeprom_data_in;
                counter                                   <= to_unsigned(6, counter'length);
            else
                counter                            <= to_unsigned(0, counter'length);
                ram_block(get_addr(EECON_ADDRESS)) <= (others => '0');
                ram_block(get_addr(EEADR_ADDRESS)) <= (others => '0');
                if (addr >= 255) then
                    addr <= 0;
                else
                    addr <= addr + 1;
                end if;
            end if;

        end write_eeprom_dump;

    begin

        if (reset = '1') then
            counter      <= to_unsigned(0, counter'length);
            addr         <= 0;
            edge_trigger <= '0';
            reset_eecon  <= '0';
            data_out     <= (others => '0');
            status_out   <= (others => '0');
            mem_dump     <= (others => '0');
            eedata_out   <= (others => '0');
            eeadr_out    <= (others => '0');
            eecon_out    <= (others => '0');
            ram_block    <= (others => (others => '0'));
        elsif (rising_edge(clk)) then
            status_out   <= ram_block(get_addr(STATUS_ADDRESS));
            edge_trigger <= ram_block(get_addr(OPTION_ADDRESS))(4);
            eedata_out   <= ram_block(get_addr(EEDATA_ADDRESS));
            eeadr_out    <= ram_block(get_addr(EEADR_ADDRESS));
            eecon_out    <= ram_block(get_addr(EECON_ADDRESS));
            if (eeprom_read = '1') then
                ram_block(get_addr(EEDATA_ADDRESS)) <= eeprom_data_in;
            end if;
            if (reset_eecon = '1') then
                ram_block(get_addr(EECON_ADDRESS))(1 downto 0) <= (others => '0');
                reset_eecon                                    <= '0';
            end if;
            if (mem_dump_enable = '1') then
                mem_dump <= ram_to_linear(ram_block);
            elsif (eeprom_dump_enable = '1') then
                write_eeprom_dump;
            elsif (read_enable = '1') then
                read_ram;
            elsif (write_enable = '1') then
                write_ram;
            end if;
            if (status_write_enable = '1') then
                ram_block(get_addr(STATUS_ADDRESS))(2 downto 0) <= status_in(2 downto 0);
            end if;
            if (timer_write_enable = '1' and ram_block(get_addr(OPTION_ADDRESS))(5) = '0') then
                ram_block(get_addr(TMR0_ADDRESS)) <=
                    std_logic_vector(unsigned(ram_block(get_addr(TMR0_ADDRESS))) + 1);
            end if;
            if (ram_block(get_addr(OPTION_ADDRESS))(5) = '1') then
                ram_block(get_addr(TMR0_ADDRESS)) <=
                    std_logic_vector(unsigned(timer_external_counter_falling) +
                                     unsigned(timer_external_counter_rising));
            end if;
            if (eeprom_write_completed = '1') then
                ram_block(get_addr(EECON_ADDRESS))(4) <= '1';
            end if;
        end if;

    end process ram;

end architecture rtl;
