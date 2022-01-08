library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;
use work.states_package.all;

entity input_receive is
    port (clk : in std_logic;
          reset : in std_logic;
          serial_in : in std_logic;
          mosi : in std_logic;
          trig_instruction_process : out std_logic;
          binary_string : out std_logic_vector(13 downto 0));
end input_receive;

architecture rtl of input_receive is
    signal counter : integer := 0;
    signal mosi_detected : std_logic;
begin
    func : process(all) is
    begin
        if (reset = '1') then
            binary_string <= (others => '0');
            trig_instruction_process <= '0';
            mosi_detected <= '0';
            counter <= 0;
        elsif (rising_edge(clk)) then
            if (mosi = '1') then
                binary_string <= binary_string(12 downto 0) & serial_in;
                mosi_detected <= '1';
                counter <= 5;
            else
                if (mosi_detected = '1' and counter > 0) then
                    if (counter < 4) then
                        trig_instruction_process <= '1';
                    end if;
                    counter <= counter - 1;
                else
                    counter <= 0;
                    mosi_detected <= '0';
                    binary_string <= (others => '0');
                    trig_instruction_process <= '0';
                end if;
            end if;
        end if;
    end process func;
end architecture rtl;