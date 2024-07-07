library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity parallel_to_serial_output is
    port (
        clk                       : in    std_logic;
        reset                     : in    std_logic;
        enable                    : in    std_logic;
        result_enable_ram_dump    : in    std_logic;
        result_enable_eeprom_dump : in    std_logic;
        data_to_sw                : in    std_logic_vector(7 downto 0);
        ram_dump_to_sw            : in    std_logic_vector(1015 downto 0);
        eeprom_dump_to_sw         : in    std_logic_vector(2047 downto 0);
        miso                      : out   std_logic;
        serial_out                : out   std_logic
    );
end entity parallel_to_serial_output;

architecture rtl of parallel_to_serial_output is

    signal data_to_send   : std_logic_vector(7 downto 0);
    signal ram_to_send    : std_logic_vector(1015 downto 0);
    signal eeprom_to_send : std_logic_vector(2047 downto 0);

begin

    parallel_to_serial_output : process (all) is

        variable idx                 : integer := 0;
        variable ram_dump_process    : std_logic;
        variable eeprom_dump_process : std_logic;
        variable data_send_process   : std_logic;

    begin

        if (reset = '1') then
            serial_out          <= '0';
            data_to_send        <= (others => '0');
            ram_to_send         <= (others => '0');
            eeprom_to_send      <= (others => '0');
            miso                <= '0';
            idx                 := 0;
            ram_dump_process    := '0';
            eeprom_dump_process := '0';
            data_send_process   := '0';
        elsif (rising_edge(clk)) then
            if (idx > 0) then
                if (data_send_process = '1') then
                    serial_out <= data_to_send(idx - 1);
                elsif (ram_dump_process = '1') then
                    serial_out <= ram_to_send(idx - 1);
                elsif (eeprom_dump_process = '1') then
                    serial_out <= eeprom_to_send(idx - 1);
                end if;
                miso <= '1';
                idx  := idx - 1;
            elsif (result_enable_eeprom_dump = '1') then
                data_to_send        <= (others => '0');
                ram_to_send         <= (others => '0');
                eeprom_to_send      <= eeprom_dump_to_sw;
                data_send_process   := '0';
                ram_dump_process    := '0';
                eeprom_dump_process := '1';
                serial_out          <= '0';
                miso                <= '1';
                idx                 := 2048;
            elsif (result_enable_ram_dump = '1') then
                data_to_send        <= (others => '0');
                ram_to_send         <= ram_dump_to_sw;
                eeprom_to_send      <= (others => '0');
                data_send_process   := '0';
                ram_dump_process    := '1';
                eeprom_dump_process := '0';
                serial_out          <= '0';
                miso                <= '1';
                idx                 := 1016;
            elsif (enable = '1') then
                data_to_send        <= data_to_sw;
                ram_to_send         <= (others => '0');
                eeprom_to_send      <= (others => '0');
                data_send_process   := '1';
                ram_dump_process    := '0';
                eeprom_dump_process := '0';
                serial_out          <= '0';
                miso                <= '1';
                idx                 := 8;
            else
                data_to_send        <= (others => '0');
                ram_to_send         <= (others => '0');
                eeprom_to_send      <= (others => '0');
                data_send_process   := '0';
                ram_dump_process    := '0';
                eeprom_dump_process := '0';
                serial_out          <= '0';
                miso                <= '0';
                idx                 := 0;
            end if;
        end if;

    end process parallel_to_serial_output;

end architecture rtl;
